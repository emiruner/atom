/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include <vm/Thread.hpp>
#include <vm/Opcode.hpp>
#include <os/ObjectStore.hpp>

#include "LineReader.hpp"
#include "Prims.hpp"

using namespace atom;
using namespace atom::debugger;

typedef std::vector<void*> VoidPtrVec;

std::map<void*, std::string> natives;
std::map<std::string, Ref> nativesRefs;

inline bool isDumped(VoidPtrVec const& dumped, void* ref)
{
    return std::find(dumped.begin(), dumped.end(), ref) != dumped.end();
}

#define CASESTR(C) case ObjectType::C: return #C

char const* objectTypeStr(int type)
{
    switch(type)
    {
        CASESTR(BYTE_ARRAY);
        CASESTR(NATIVE_FUNCTION);
        CASESTR(OBJECT);
        CASESTR(OBJECT_ARRAY);
        CASESTR(SIMPLE_FUNCTION);
        CASESTR(FLOAT);
    }

    return "UNKNOWN_OBJECT_TYPE";
}

#undef CASESTR

void memoryStatusSummary(Memory& mem)
{
    std::cout << "Memory status summary:\n"
              << "    Object memory size               : " << mem.toSpace_->size() << "\n"
              << "    Object memory allocated slots    : " << mem.toSpace_->freeSize() << "\n";
}

void printPrimObjectHeader(PrimDataObject* rp)
{
    std::cout << objectTypeStr(rp->header_.objtype)
              << ", length = " << ((PrimDataObject*) rp)->size()
              << (rp->header_.objtype == ObjectType::NATIVE_FUNCTION ? ", " : "")
              << (rp->header_.objtype == ObjectType::NATIVE_FUNCTION ? natives[rp->data_] : "");
}

void printObjectHeader(ObjectHeader* oh)
{
    if(oh->objtype ==  ObjectType::NATIVE_FUNCTION || oh->objtype == ObjectType::BYTE_ARRAY)
    {
        printPrimObjectHeader((PrimDataObject*) oh);
    }
    else
    {
        std::cout << objectTypeStr(oh->objtype) << ", slot count = " << oh->size;
    }
}

void dumpObjectMemory(Memory& om)
{
    Ref* current = om.toSpace_->start_;

    while(current != om.toSpace_->free_)
    {
        ObjectHeader* oh = (ObjectHeader*) current;

        std::cout << (void*) oh << ": ";
        printObjectHeader(oh);
        std::cout << "\n";

        current += oh->size;
    }
}

void dumpFloat(FloatObject*, VoidPtrVec& dumped);
void dumpObjectArray(ObjectArray*, VoidPtrVec& dumped);
void dumpSimpleFunction(SimpleFunction* sf, VoidPtrVec& dumped);
void dumpSimObject(Object* obj, VoidPtrVec& dumped);

void dumpObject(Ref ref, VoidPtrVec& dumped)
{
    if(is_int(ref))
    {
        std::cout << int_val(ref);
    }
    else
    {
        if(array_access(ref))
        {
            std::cout << "*";
        }

        if(isDumped(dumped, ptr_val(ref)))
        {
            std::cout << ptr_val(ref) << ' ';
            return;
        }
        else
        {
            dumped.push_back(ptr_val(ref));
        }

        ObjectHeader* oh = (ObjectHeader*) ptr_val(ref);

        if(oh->objtype == ObjectType::OBJECT)
        {
            dumpSimObject((Object*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::OBJECT_ARRAY)
        {
            dumpObjectArray((ObjectArray*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::SIMPLE_FUNCTION)
        {
            dumpSimpleFunction((SimpleFunction*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::FLOAT)
        {
            dumpFloat((FloatObject*) oh, dumped);
        }
        else
        {
            printObjectHeader(oh);
        }
    }
}

void dumpFloat(FloatObject* o, VoidPtrVec& dumped)
{
    std::cout << "float: " << o->value_;
}

void dumpSlots(Object* oa, VoidPtrVec& dumped)
{
    for(int i = 0; i < oa->size(); ++i)
    {
        std::cout << i << " -> ";
        dumpObject(oa->at(i), dumped);
        std::cout << ", ";
    }
}

void dumpSimObject(Object* obj, VoidPtrVec& dumped)
{
    std::cout << "object: [";
    dumpSlots(obj, dumped);
    std::cout << "]";
}

void dumpObjectArray(ObjectArray* obj, VoidPtrVec& dumped)
{
    std::cout << "object-array: [";
    dumpSlots((Object*) obj, dumped);
    std::cout << "]";
}

void dumpSimpleFunction(SimpleFunction* sf, VoidPtrVec& dumped)
{
    std::cout << "simple function: [";

    ObjectArray* oa = (ObjectArray*) sf;
    
    std::cout << "bytecodes -> ";
    dumpObject(oa->at(0), dumped);;
    
    std::cout << ", tempCount -> ";
    dumpObject(oa->at(1), dumped);;
    std::cout << ", ";
    
    for(int i = 2; i < oa->size(); ++i)
    {
        std::cout << i << " -> ";
        dumpObject(oa->at(i), dumped);
        std::cout << ", ";
    }

    std::cout << "]";
}

void dumpByteArray(Ref baRef)
{
    if(!is_byte_array(baRef))
    {
        std::cout << "given object is not a byte array\n";
    }
    else
    {
        ByteArray* baObject = cast<ByteArray>(baRef);
        byte* ba = baObject->data();

        for(unsigned int i = 0; i < baObject->size(); ++i)
        {
            std::cout << (int) ba[i] << ' ';
        }

        std::cout << '\n';
    }
}

void dumpBytecodes(Ref baRef)
{
    if(!is_byte_array(baRef))
    {
        std::cout << "given object is not a byte array\n";
    }
    else
    {
        ByteArray* baObject = cast<ByteArray>(baRef);
        byte* ba = baObject->data();

        for(unsigned int i = 0; i < baObject->size(); ++i)
        {
            if(ba[i] == Opcode::CONDITIONAL_ONE)
            {
                std::cout << "if1 ";
                ++i;
                std::cout << "$" << (int) ba[i] << " ";
            }
            else if(ba[i] == Opcode::CONDITIONAL_NOT_ONE)
            {
                std::cout << "ifnot1 ";
                ++i;
                std::cout << "$" << (int) ba[i] << " ";
            }
            else if(ba[i] == Opcode::SEND_VAL_TO_VAL)
            {
                std::cout << "send ";
                ++i;
                std::cout << "$" << (int) ba[i] << " to ";
                ++i;
                std::cout << "$" << (int) ba[i] << "\n";
            }
            else if(ba[i] == Opcode::SEND_VAL_TO_VAL_WRES)
            {
                std::cout << "send ";
                ++i;
                std::cout << "$" << (int) ba[i] << " to ";
                ++i;
                std::cout << "$" << (int) ba[i] << " > ";
                ++i;
                std::cout << "$" << (int) ba[i] << '\n';
            }
            else if(ba[i] == Opcode::CREATE_OBJECT_ARRAY)
            {
                std::cout << "croa ";
                ++i;

                int length = ba[i];
                ++i;

                std::cout << "[";
                for(int x = 0; x < length - 1; ++x)
                {
                    std::cout << "$" << (int) ba[i] << ", ";
                    ++i;
                }

                std::cout << "$" << (int) ba[i] << "] > ";
                ++i;

                std::cout << "$" << (int) ba[i] << "\n";
            }
            else if(ba[i] == 4)
            {
                std::cout << "return\n";
            }
            else if(ba[i] == Opcode::RETURN_RESULT)
            {
                std::cout << "return ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";

                std::cout << "\n";
            }
            else if(ba[i] == Opcode::ARRAY_AT)
            {
                std::cout << "aat ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";
                ++i;

                std::cout << "> $" << (int) ba[i];

                std::cout << "\n";
            }
            else if(ba[i] == Opcode::ARRAY_AT_PUT)
            {
                std::cout << "aatput ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";
                ++i;

                std::cout << "$" << (int) ba[i];

                std::cout << "\n";
            }
            else if(ba[i] == Opcode::ARRAY_LENGTH)
            {
                std::cout << "alen ";
                ++i;

                std::cout << "$" << (int) ba[i] << " ";
                ++i;

                std::cout << "> $" << (int) ba[i];

                std::cout << "\n";
            }
            else if(ba[i] == Opcode::HALT)
            {
                std::cout << "halt\n";
            }
            else if(ba[i] == Opcode::INSTALL_EXCEPTION_HANDLER)
            {
                std::cout << "iexh ";
                ++i;
                
                std::cout << "$" << (int) ba[i] << "\n";                
            }
            else
            {
                std::cout << (int) ba[i] << '\n';
                i += (Opcode::instructionSize(ba[i], ba[i + 1]) - 1);
            }
        }
    }
}

Ref convertToPtr(std::string str)
{
    if(str.find("0x") == 0)
    {
        std::istringstream myiss(str);

        word ptr;

        myiss >> std::hex >> ptr;
        return ptr2ref((void*) ptr);
    }
    else
    {
        std::istringstream myiss(str);
        word val;
        myiss >> val;
        return word2ref(val);
    }
}

Ref readPtr(std::istream& iss)
{
    std::string str;
    iss >> str;

    return convertToPtr(str);
}

void readRefs(std::istream& iss, std::vector<Ref>& refs)
{
    while(!iss.eof())
    {
        std::string str;
        iss >> str;

        if(str == "")
        {
            continue;
        }

        refs.push_back(convertToPtr(str));
    }
}

#define NEW_FN(fn) { natives[(void*) &fn] = #fn; nativesRefs[#fn] = mem.createNativeFunction(&fn); }

int main()
{
    string_vector commands;

    commands.push_back("memory-status-summary");
    commands.push_back("dump-memory");
    commands.push_back("show-object");
    commands.push_back("dump-bytecodes");
    commands.push_back("dump-byte-array");
    commands.push_back("load-object-store");
    commands.push_back("save-object-store");
    commands.push_back("set-elem");
    commands.push_back("prepare-send");
    commands.push_back("create-cstring");
    commands.push_back("create-float");
    commands.push_back("create-byte-array");
    commands.push_back("create-object-array");
    commands.push_back("create-object");
    commands.push_back("create-function");
    commands.push_back("create-startup-array");
    commands.push_back("quit");
    commands.push_back("step");
    commands.push_back("run");
    commands.push_back("gc");

    LineReader input("(zdb) ", commands);
    std::string line;

    Memory mem(10240);
    Thread thread(&mem);
    mem.setThread(&thread);
    
    NEW_FN(fn_println);
    NEW_FN(fn_printCString);
    NEW_FN(fn_addOne);
    NEW_FN(fn_exitVm);
    NEW_FN(fn_multInt);
    NEW_FN(fn_divInt);
    NEW_FN(fn_addInt);
    NEW_FN(fn_subInt);
    NEW_FN(fn_modInt);
    NEW_FN(fn_eqInt);
    NEW_FN(fn_readInt);
    NEW_FN(fn_is_object);
    NEW_FN(fn_is_int_object);
    NEW_FN(fn_intLessThan);
    NEW_FN(fn_intGreaterThan);
    NEW_FN(fn_is_byte_array);
    NEW_FN(fn_arrayCopy);

    while(true)
    try
    {
        std::string line = input.readLine();
        std::istringstream iss(line.c_str());

        std::string cmd;
        iss >> cmd;

        if(cmd == "memory-status-summary")
        {
            memoryStatusSummary(mem);
        }
        else if(cmd == "dump-memory")
        {
            dumpObjectMemory(mem);
        }
        else if(cmd == "show-object")
        {
            VoidPtrVec dumped;
            dumpObject(readPtr(iss), dumped);
            std::cout << "\n";
        }
        else if(cmd == "dump-bytecodes")
        {
            dumpBytecodes(readPtr(iss));
        }
        else if(cmd == "dump-byte-array")
        {
            dumpByteArray(readPtr(iss));
        }
        else if(cmd == "save-object-store")
        {
            std::string osFile;
            iss >> osFile;

            std::vector<Ref> refs;
            readRefs(iss, refs);

            ObjectStoreWriter osr(osFile.c_str());

            for(unsigned int i = 0; i < refs.size(); ++i)
            {
                osr.addObject(refs.at(i));
            }

            osr.flush();
        }
        else if(cmd == "load-object-store")
        {
            std::string osFile;
            iss >> osFile;

            if(osFile.empty())
            {
                std::cerr << "You must specify object store filename.\n";
            }
            else
            {
                std::cerr << "Object store file is: " << osFile << '\n';
                ObjectStoreReader(osFile.c_str(), &mem).readAll();
            }
        }
        else if(cmd == "prepare-send")
        {
            RefHandle target(readPtr(iss), &mem);
            RefHandle message(readPtr(iss), &mem);

            thread.prepareInitialSend(target, message);
            thread.halt_ = false;
        }
        else if(cmd == "gc")
        {
            mem.flipSpaces();
        }
        else if(cmd == "step")
        {
            try 
            {
                thread.step();
            }
            catch(std::runtime_error& ex)
            {
                std::cerr << "An error occured during VM step: " << ex.what() << '\n';
            }
        }
        else if(cmd == "run")
        {
            thread.execute();
        }
        else if(cmd == "create-byte-array")
        {
            std::vector<int> bytes;

            while(!iss.eof())
            {
                int b;
                iss >> b;
                bytes.push_back(b);
            }

            Ref bbRef = mem.createManagedByteArray(bytes.size());
            byte* bb = cast<ByteArray>(bbRef)->data();

            for(unsigned int i = 0; i < bytes.size(); ++i)
            {
                bb[i] = bytes.at(i);
            }

            std::cout << "created byte array: " << ptr_val(bbRef) << "\n";
        }
        else if(cmd == "create-object-array")
        {
            std::vector<Ref> refs;
            readRefs(iss, refs);

            ObjectArray* oa = mem.createObjectArray(refs.size());

            for(unsigned int i = 0; i < refs.size(); ++i)
            {
                oa->atPut(i, refs.at(i));
            }
        }
        else if(cmd == "create-object")
        {
            std::vector<Ref> refs;
            readRefs(iss, refs);
            Object* obj = mem.createObject<Object>(refs.size() + 1);
            Object* oa = (Object*) obj;

            for(unsigned int i = 0; i < refs.size(); ++i)
            {
                oa->atPut(i, refs.at(i));
            }

            std::cout << "created object: " << obj << "\n";
        }
        else if(cmd == "create-startup-array")
        {
            Ref printFn = readPtr(iss);

            if(!is_native_fn(printFn))
            {
                std::cerr << "parameter must be a native function.";
            }
            else
            {
                std::cout << "created startup array: " << ptr_val(mem.createStartupMessage(printFn)) << '\n';
            }
        }
        else if(cmd == "create-function")
        {
            std::vector<Ref> refs;
            readRefs(iss, refs);

            if(!is_byte_array(refs.at(0)))
            {
                std::cerr << "first object must be bytecode array\n";
            }
            else if(!is_int(refs.at(1)))
            {
                std::cerr << "second object must be an int\n";
            }
            else
            {
                SimpleFunction* obj = mem.createObject<SimpleFunction>(refs.size() + 1);
                Object* oa = (Object*) obj;

                for(unsigned int i = 0; i < refs.size(); ++i)
                {
                    oa->atPut(i, refs.at(i));
                }

                std::cout << "created object: " << obj << "\n";
            }
        }
        else if(cmd == "create-cstring")
        {
            std::string str;
            std::getline(iss, str);
            str = str.substr(1, str.length() - 1);

            Ref baRef = mem.createManagedByteArray(str.length() + 1);
            byte* ba = cast<ByteArray>(baRef)->data();

            for(unsigned int i = 0; i < str.length(); ++i)
            {
                ba[i] = str.at(i);
            }

            ba[str.length()] = '\0';
            std::cout << "created byte array: " << ptr_val(baRef) << "\n";
        }
        else if(cmd == "create-float")
        {
            double d;
            iss >> d;

            Ref result = mem.createFloat(d);
            std::cout << "created float: " << ptr_val(result) << "\n";
        }
        else if(cmd == "set-elem")
        {
            Ref oaref = readPtr(iss);
            word idx;
            iss >> idx;
            Ref elem = readPtr(iss);

            Object* oa = cast<Object>(oaref);

            oa->atPut(idx, elem);
        }
        else if(cmd == "quit" || line.empty())
        {
            break;
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }
    catch(std::runtime_error& ex)
    {
        std::cerr << "an error occured: " << ex.what() << '\n';
    }
}
