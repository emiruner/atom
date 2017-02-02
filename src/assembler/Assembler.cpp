/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Assembler.hpp"
#include <vm/Opcode.hpp>

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <typeinfo>

using namespace atom;
using namespace atom::assembler;

namespace
{

template <typename T>
inline bool isa(Node* n)
{
    return dynamic_cast<T*>(n) != 0;
}

inline int max(int x, int y)
{
    if(x > y)
    {
        return x;
    }
    else
    {
        return y;
    }
}

} // namespace <unnamed>

void Assembler::assemble()
{
    result_ = mem_.createObjectArray(defs_.size());

    int i = 0;
    for(TopLevelDefVec::const_iterator it = defs_.begin(); it != defs_.end(); ++it, ++i)
    {
        TopLevelDef* tld = *it;
        Ref ref = assembleNode(tld->def());
        result_->atPut(i, ref);
        assembledObjects_[tld->name()] = ref;
    }

    for(FixupRecordVec::iterator it = fixups_.begin(); it != fixups_.end(); ++it)
    {
        it->fixUp(assembledObjects_);
    }
}

template <int TYPE>
void Assembler::assembleNodeInto(Node* node, RefArrayObjectBase<TYPE>* o, int index)
{
    if(isa<SymbolicRef>(node))
    {
        SymbolicRef* sr = (SymbolicRef*) node;
        fixups_.push_back(FixupRecord(sr->name(), index, o));
    }
    else
    {
        o->atPut(index, assembleNode(node));
    }
}

void Assembler::assembleNodeInto(Node* node, ByteArray* ba, int index)
{
    if(isa<SymbolicRef>(node))
    {
        SymbolicRef* sr = (SymbolicRef*) node;
        fixups_.push_back(FixupRecord(sr->name(), index, ba));
    }
    else
    {
        ba->atPut(index, assembleNode(node));
    }
}

Ref Assembler::assembleStringLiteral(StringLiteralNode* sln)
{
    return mem_.createByteArrayFromString(sln->literal().c_str());
}

Ref Assembler::assembleNode(Node* node)
{
    if(isa<ObjectObjectDefNode>(node))
    {
        return assembleObjectObjectDef((ObjectObjectDefNode*) node);
    }
    if(isa<ObjectArrayDefNode>(node))
    {
        return assembleObjectArrayDef((ObjectArrayDefNode*) node);
    }
    else if(isa<BytearrayDefNode>(node))
    {
        return assembleBytearrayDef((BytearrayDefNode*) node);
    }
    else if(isa<StringLiteralNode>(node))
    {
        return assembleStringLiteral((StringLiteralNode*) node);
    }
    else if(isa<IntegerLiteralNode>(node))
    {
        return word2ref(((IntegerLiteralNode*) node)->literal());
    }
    else if(isa<FloatLiteralNode>(node))
    {
        FloatLiteralNode* fln = (FloatLiteralNode*) node;
        FloatObject* f = mem_.createObject<FloatObject>();
        f->value_ = fln->literal();
        return ptr2ref(f);
    }
    else if(isa<SimplefnRawNode>(node))
    {
        SimplefnRawNode* sfnr = (SimplefnRawNode*) node;
        SimpleFunction* sf = mem_.createObject<SimpleFunction>(1 + 2 + sfnr->knownObjects().size());

        assembleNodeInto(sfnr->bytecodes(), sf, 0);
        assembleNodeInto(sfnr->tempcount(), sf, 1);

        for(unsigned int i = 0; i < sfnr->knownObjects().size(); ++i)
        {
            assembleNodeInto(sfnr->knownObjects()[i], sf, i + 2);
        }

        return ptr2ref(sf);
    }
    else if(isa<SimpleFnDefNode>(node))
    {
        return assembleSimpleFunction((SimpleFnDefNode*) node);
    }

    throw std::runtime_error("Unexpected node type: " + std::string(typeid(*node).name()));
}

Ref Assembler::assembleObjectObjectDef(ObjectObjectDefNode* node)
{
    Object* o = mem_.createObject<Object>(node->remaining().size() + 2);

    assembleNodeInto(node->metaObject(), o, 0);

    for(unsigned int i = 0; i < node->remaining().size(); ++i)
    {
        assembleNodeInto(node->remaining()[i], o, i + 1);
    }

    return ptr2ref(o);
}

Ref Assembler::assembleObjectArrayDef(ObjectArrayDefNode* node)
{
    ObjectArray* o = mem_.createObjectArray(node->elements().size());

    for(unsigned int i = 0; i < node->elements().size(); ++i)
    {
        assembleNodeInto(node->elements()[i], o, i);
    }

    return ptr2ref(o, true);
}

Ref Assembler::assembleBytearrayDef(BytearrayDefNode* node)
{
    ByteArray* ba = cast<ByteArray>(mem_.createManagedByteArray(node->elements().size()));

    for(unsigned int i = 0; i < node->elements().size(); ++i)
    {
        assembleNodeInto(node->elements()[i], ba, i);
    }

    return ptr2ref(ba, true);
}

int Assembler::countInlineDefsInSimpleFunction(SimpleFnDefNode* sfd)
{
    int inlineDefsInsideInstructions = 0;

    for(InstructionNodeVec::iterator it = sfd->instructions().begin(); it != sfd->instructions().end(); ++it)
    {
        InstructionNode* in = *it;

        if(isa<SendNode>(in))
        {
            SendNode* n = (SendNode*) in;

            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->message());
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->target());
        }
        else if(isa<AlenNode>(in))
        {
            AlenNode* n = (AlenNode*) in;

            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->arr());
        }
        else if(isa<AatNode>(in))
        {
            AatNode* n = (AatNode*) in;

            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->arr());
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->index());
        }
        else if(isa<AputNode>(in))
        {
            AputNode* n = (AputNode*) in;

            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->arr());
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->index());
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->value());
        }
        else if(isa<CroNode>(in))
        {
            CroNode* n = (CroNode*) in;

            for(NodeVec::iterator it = n->elements().begin(); it != n->elements().end(); ++it)
            {
                inlineDefsInsideInstructions += !sfd->isTempVarRef(*it);
            }
        }
        else if(isa<CroaNode>(in))
        {
            CroaNode* n = (CroaNode*) in;

            for(NodeVec::iterator it = n->elements().begin(); it != n->elements().end(); ++it)
            {
                inlineDefsInsideInstructions += !sfd->isTempVarRef(*it);
            }
        }
        else if(isa<ConditionalNode>(in))
        {
            // Do nothing.
        }
        else if(isa<ReturnNode>(in))
        {
            ReturnNode* n = (ReturnNode*) in;

            if(n->result() != 0)
            {
                inlineDefsInsideInstructions += !sfd->isTempVarRef(n->result());
            }
        }
        else if(isa<InstallExHandlerNode>(in))
        {
            InstallExHandlerNode* n = (InstallExHandlerNode*) in;
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->handler());            
        }
        else if(isa<RaiseExceptionNode>(in))
        {
            RaiseExceptionNode* n = (RaiseExceptionNode*) in;
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->exception());            
        }
        else if(isa<SetNode>(in))
        {
            SetNode* n = (SetNode*) in;
            inlineDefsInsideInstructions += !sfd->isTempVarRef(n->value());
        }        
        else if(isa<JumpNode>(in))
        {
            inlineDefsInsideInstructions += 1;
        }
        else if(isa<HaltNode>(in) || isa<LabelNode>(in))
        {
            // Do nothing.
        }        
        else
        {
            throw std::runtime_error("Unexpected node type while assembling simple function: " + std::string(typeid(*in).name()));
        }
    }
    
    return inlineDefsInsideInstructions;
}

IntVec Assembler::generateBytecodes(SimpleFnDefNode* sfd)
{
    IntVec bc;
    StringIntMap labelPositions;
    IntStringMap jumpPositions;
    
    for(InstructionNodeVec::iterator it = sfd->instructions().begin(); it != sfd->instructions().end(); ++it)
    {
        InstructionNode* in = *it;

        if(isa<SendNode>(in))
        {
            SendNode* n = (SendNode*) in;

            int resultIdx;

            if(n->result() != 0)
            {
                resultIdx = checkForTemp(sfd, n->result());
            }
            else
            {
                resultIdx = -1;
            }

            if(resultIdx == -1)
            {
                bc.push_back(Opcode::SEND_VAL_TO_VAL);
            }
            else
            {
                bc.push_back(Opcode::SEND_VAL_TO_VAL_WRES);
            }

            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->message()));
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->target()));

            if(resultIdx != -1)
            {
                bc.push_back(resultIdx);
            }
        }
        else if(isa<AlenNode>(in))
        {
            AlenNode* n = (AlenNode*) in;

            bc.push_back(Opcode::ARRAY_LENGTH);
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->arr()));
            bc.push_back(checkForTemp(sfd, n->result()));
        }
        else if(isa<AatNode>(in))
        {
            AatNode* n = (AatNode*) in;

            bc.push_back(Opcode::ARRAY_AT);
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->arr()));
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->index()));
            bc.push_back(checkForTemp(sfd, n->result()));
        }
        else if(isa<AputNode>(in))
        {
            AputNode* n = (AputNode*) in;

            bc.push_back(Opcode::ARRAY_AT_PUT);
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->arr()));
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->index()));
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->value()));
        }
        else if(isa<ConditionalNode>(in))
        {
            ConditionalNode* n = (ConditionalNode*) in;

            if(n->positive())
            {
                bc.push_back(Opcode::CONDITIONAL_ONE);
            }
            else
            {
                bc.push_back(Opcode::CONDITIONAL_NOT_ONE);
            }

            bc.push_back(checkForTemp(sfd, n->cond()));
        }
        else if(isa<CroNode>(in))
        {
            CroNode* n = (CroNode*) in;

            bc.push_back(Opcode::CREATE_OBJECT);
            bc.push_back(n->elements().size());

            for(NodeVec::iterator it = n->elements().begin(); it != n->elements().end(); ++it)
            {
                bc.push_back(checkForTempRefOrAssembleNode(sfd, *it));
            }

            bc.push_back(checkForTemp(sfd, n->result()));
        }
        else if(isa<CroaNode>(in))
        {
            CroaNode* n = (CroaNode*) in;

            bc.push_back(Opcode::CREATE_OBJECT_ARRAY);
            bc.push_back(n->elements().size());

            for(NodeVec::iterator it = n->elements().begin(); it != n->elements().end(); ++it)
            {
                bc.push_back(checkForTempRefOrAssembleNode(sfd, *it));
            }

            bc.push_back(checkForTemp(sfd, n->result()));
        }
        else if(isa<ReturnNode>(in))
        {
            ReturnNode* n = (ReturnNode*) in;

            if(n->result() == 0)
            {
                bc.push_back(Opcode::RETURN);
            }
            else
            {
                bc.push_back(Opcode::RETURN_RESULT);
            }

            if(n->result() != 0)
            {
                bc.push_back(checkForTempRefOrAssembleNode(sfd, n->result()));
            }
        }
        else if(isa<HaltNode>(in))
        {
            bc.push_back(Opcode::HALT);
        }
        else if(isa<InstallExHandlerNode>(in))
        {
            InstallExHandlerNode* n = (InstallExHandlerNode*) in;
            
            bc.push_back(Opcode::INSTALL_EXCEPTION_HANDLER);
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->handler()));
        }        
        else if(isa<RaiseExceptionNode>(in))
        {
            RaiseExceptionNode* n = (RaiseExceptionNode*) in;
            
            bc.push_back(Opcode::RAISE_EXCEPTION);
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->exception()));
        }
        else if(isa<SetNode>(in))
        {
            SetNode* n = (SetNode*) in;
            
            bc.push_back(Opcode::SET_LOCAL);
            bc.push_back(checkForTemp(sfd, n->target()));
            bc.push_back(checkForTempRefOrAssembleNode(sfd, n->value()));
        }
        else if(isa<LabelNode>(in))
        {
            LabelNode* n = (LabelNode*) in;
            labelPositions[n->name()->name()] = bc.size();
            
        }
        else if(isa<JumpNode>(in))
        {
            JumpNode* n = (JumpNode*) in;
            
            bc.push_back(Opcode::JUMP);
            jumpPositions[bc.size()] = n->label()->name();
            bc.push_back(0);            
        }
        else
        {
            throw std::runtime_error("Unexpected node type while assembling simple function: " + std::string(typeid(*in).name()));
        }
    }
    
    for(IntStringMap::const_iterator it = jumpPositions.begin(); it != jumpPositions.end(); ++it)
    {
        if(labelPositions.find(it->second) == labelPositions.end())
        {
            throw std::runtime_error("Label not found in jump: " + it->second);
        }
                
        int index = sfd->allocateFreeSlotFromTemps();
        sfd->temps()->atPut(index, word2ref(labelPositions.find(it->second)->second));
        bc[it->first] = index;
    }
    
    return bc;
}

int Assembler::processVarNodesInSimpleFunction(SimpleFnDefNode* sfd)
{
    int maxSpecifiedIndex = -1;

    for(VarNodeVec::iterator it = sfd->vars().begin(); it != sfd->vars().end(); ++it)
    {
        VarNode* vn = *it;

        if(vn->value() == 0)
        {
            if(vn->index() == -1)
            {
                sfd->allocateFreeTemp(vn->name());
            }
            else
            {
                if(vn->index() > maxSpecifiedIndex)
                {
                    maxSpecifiedIndex = vn->index();
                }

                sfd->setNameForIndex(vn->index(), vn->name());
            }
        }
        else
        {
            if(vn->index() == -1)
            {
                int index = sfd->allocateKnownObject(vn->name());
                assembleNodeInto(vn->value(), sfd->temps(), index);
            }
            else
            {
                if(vn->index() > maxSpecifiedIndex)
                {
                    maxSpecifiedIndex = vn->index();
                }

                sfd->setNameForIndex(vn->index(), vn->name());

                if(vn->index() < 2)
                {
                    throw std::runtime_error("cannot set value for temps 0 and 1");
                }

                assembleNodeInto(vn->value(), sfd->temps(), vn->index());
            }
        }
    }

    return maxSpecifiedIndex;
}

Ref Assembler::assembleSimpleFunction(SimpleFnDefNode* sfd)
{
    sfd->temps(mem_.createObjectArray(255));

    int maxSpecifiedIndex = processVarNodesInSimpleFunction(sfd);
   
    maxSpecifiedIndex = max(maxSpecifiedIndex, maxSpecifiedIndex + countInlineDefsInSimpleFunction(sfd) - sfd->emptySlotsBefore(maxSpecifiedIndex)) + 1;

    sfd->fixFreeTemps(maxSpecifiedIndex);

    IntVec bc = generateBytecodes(sfd);

    SimpleFunction* sf = mem_.createObject<SimpleFunction>(maxSpecifiedIndex + 1);
    sf->tempCount_ = word2ref(sfd->lastFreeTempIndex() + 1);

    for(int i = 2; i < maxSpecifiedIndex; ++i)
    {
        sf->atPut(i, sfd->temps()->at(i));
    }

    for(FixupRecordVec::iterator it = fixups_.begin(); it != fixups_.end(); ++it)
    {
        FixupRecord& fixup = *it;

        if(fixup.ptr == sfd->temps())
        {
            fixup.ptr = (ObjectArray*) sf;
        }
    }

    ByteArray* ba = (ByteArray*) ptr_val(mem_.createManagedByteArray(bc.size()));

    for(unsigned int i = 0; i < bc.size(); ++i)
    {
        ba->atPut(i, word2ref(bc[i]));
    }

    sf->bytecodes_ = ptr2ref(ba);

    return ptr2ref(sf);
}

int Assembler::checkForTempRefOrAssembleNode(SimpleFnDefNode* sfd, Node* n)
{
    SymbolicRef* sr = dynamic_cast<SymbolicRef*>(n);
    int index;

    if(sr != 0 && sfd->indexOfNamedTemp(sr->name()) != -1)
    {
        index = sfd->indexOfNamedTemp(sr->name());
    }
    else
    {
        index = sfd->allocateFreeSlotFromTemps();
        assembleNodeInto(n, sfd->temps(), index);
    }

    return index;
}

int Assembler::checkForTemp(SimpleFnDefNode* sfd, Node* n)
{
    return sfd->indexOfNamedTemp(dynamic_cast<SymbolicRef*>(n)->name());
}

ObjectArray* Assembler::result()
{
    return result_;
}
