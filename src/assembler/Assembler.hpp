/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_ASSEMBLER_ASSEMBLER_HPP_INCLUDED
#define ATOM_ASSEMBLER_ASSEMBLER_HPP_INCLUDED

#include "Parser.hpp"

#include <stdexcept>
#include <sstream>
#include <vector>
#include <map>

#include <vm/Memory.hpp>

namespace atom
{

namespace assembler
{

typedef std::map<std::string, Ref> StringRefMap;

/**
 * Both parses and generates code for atom assembly fragments.
 */
class Assembler
{
    struct FixupRecord
    {
        std::string refName;
        uword arrayIndex;
        ObjectArray* ptr;
        ByteArray* ba;
        bool array_access;
        
        template <int TYPE>
        inline FixupRecord(std::string const& prefName, uword parrayIndex, RefArrayObjectBase<TYPE>* pptr)
        : refName(prefName), arrayIndex(parrayIndex), ptr((ObjectArray*) pptr), ba(0)
        {
        }

        inline FixupRecord(std::string const& prefName, uword parrayIndex, ByteArray* pba)
        : refName(prefName), arrayIndex(parrayIndex), ptr(0), ba(pba)
        {
        }

        inline void fixUp(StringRefMap& objects)
        {            
            bool arracc = false;
            
            if(refName[0] == '*')
            {
                refName = refName.substr(1);
                arracc = true;
            }
            
            if(objects.find(refName) == objects.end())
            {
                throw std::runtime_error(refName + " named object not found");
            }

            Ref resolved = objects[refName];

            if(ptr != 0)
            {
                if(arracc)
                {
                    resolved = set_array_access(resolved);
                }
                
                ptr->atPut(arrayIndex, resolved);
            }
            else
            {
                if(is_int(resolved))
                {
                    int ival = int_val(resolved);

                    if(ival > 255)
                    {
                        throw std::runtime_error("Byte array can contain integers with value in range [0,255]");
                    }

                    ba->atPut(arrayIndex, resolved);
                }
                else
                {
                    throw std::runtime_error("Byte array can contain only integers.");
                }
            }
        }
    };

    typedef std::vector<FixupRecord> FixupRecordVec;

public:
    /**
     * Construct a lexer that uses the given stream as source.
     */
    inline Assembler(Memory& mem, TopLevelDefVec& defs)
    : mem_(mem), defs_(defs), result_(0)
    {
    }

    /**
     * Creates objects constructed from parse tree.
     */
    void assemble();

    Ref assembleNode(Node* node);

    template <int TYPE>
    void assembleNodeInto(Node* node, RefArrayObjectBase<TYPE>* o, int index);

    void assembleNodeInto(Node* node, ByteArray* ba, int index);
    Ref assembleObjectObjectDef(ObjectObjectDefNode* node);
    Ref assembleObjectArrayDef(ObjectArrayDefNode* node);
    Ref assembleBytearrayDef(BytearrayDefNode* node);
    Ref assembleStringLiteral(StringLiteralNode* sln);
    Ref assembleSimpleFunction(SimpleFnDefNode* sfd);
    int checkForTempRefOrAssembleNode(SimpleFnDefNode* sfd, Node* n);
    int checkForTemp(SimpleFnDefNode* sfd, Node* n);
    int countInlineDefsInSimpleFunction(SimpleFnDefNode* sfd);
    IntVec generateBytecodes(SimpleFnDefNode* sfd);
    int processVarNodesInSimpleFunction(SimpleFnDefNode* sfd);
    
    /**
     * Returns result of assembling as an object array.
     */
    ObjectArray* result();

private:
    Memory& mem_;
    TopLevelDefVec& defs_;
    ObjectArray* result_;
    StringRefMap assembledObjects_;
    FixupRecordVec fixups_;
};

} // namespace assembler

} // namespace atom

#endif
