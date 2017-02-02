/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Object.hpp"

#include <vector>
#include <iostream>
#include <algorithm>

using namespace atom;

namespace
{

typedef std::vector<Ref> RefVec;

inline bool isDumped(RefVec const& dumped, Ref ref)
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
        CASESTR(SIMPLE_FUNCTION);
        CASESTR(FLOAT);
    }

    return "UNKNOWN_OBJECT_TYPE";
}

#undef CASESTR

void dumpObject(Ref ref, RefVec& dumped);

void dumpSlots(Object* oa, RefVec& dumped)
{
    for(int i = 0; i < oa->size(); ++i)
    {
        std::cout << i << " -> ";
        dumpObject(oa->at(i), dumped);
        std::cout << ", ";
    }
}

void dumpRegularObject(Object* obj, RefVec& dumped)
{
    std::cout << "object: [";
    dumpSlots((Object*) obj, dumped);
    std::cout << "]";
}

void dumpFloat(FloatObject* o, RefVec& dumped)
{
    std::cout << "float: " << o->value_;
}

void dumpSimpleFunction(SimpleFunction* sf, RefVec& dumped)
{
    std::cout << "simple function: [";
    dumpSlots((Object*) sf, dumped);
    std::cout << "]";
}

void dumpObject(Ref ref, RefVec& dumped)
{
    if(is_int(ref))
    {
        std::cout << int_val(ref);
    }
    else
    {
        if(isDumped(dumped, ref))
        {
            std::cout << ptr_val(ref) << ' ';
            return;
        }
        else
        {
            dumped.push_back(ref);
        }

        ObjectHeader* oh = cast<ObjectHeader>(ref);

        if(oh->objtype == ObjectType::OBJECT)
        {
            dumpRegularObject((Object*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::SIMPLE_FUNCTION)
        {
            dumpSimpleFunction((SimpleFunction*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::FLOAT)
        {
            dumpFloat((FloatObject*) oh, dumped);
        }
        else if(oh->objtype == ObjectType::NATIVE_FUNCTION || oh->objtype == ObjectType::BYTE_ARRAY)
        {
            std::cout << objectTypeStr(oh->objtype) << ", length = " << ((PrimDataObject*) oh)->size();
        }
        else
        {
            std::cout << "WARNING: Unknown object type.";
        }
    }
}

} // namespace <anonymous>

namespace atom
{

void printObject(Ref ref)
{
    RefVec dumped;
    dumpObject(ref, dumped);
}

} // namespace atom
