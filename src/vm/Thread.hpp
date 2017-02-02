/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_THREAD_HPP_INCLUDED
#define ATOM_THREAD_HPP_INCLUDED

#include "Object.hpp"
#include "Memory.hpp"
#include "Opcode.hpp"
#include <stdexcept>

namespace atom
{
    namespace MetaMessageElements
    {
        enum
        {
            ZERO         = 0,
            TARGET       = 1,
            REALMSG      = 2
        };
    }

    struct Thread
    {
        Memory* memory_;
        bool halt_;

        long sendCount_;
        long bytecodeCount_;
        
        // Static thread state.
        RefHandle startBallRollingBytecodes_;
        RefHandle execErrorBa_;
        RefHandle processExceptionBa_;
        
        // An object to represent missing objects.
        RefHandle nullArray_;
        
        PtrHandle<CallContext> cc_;

        Thread(Memory* memory);

        void prepareInitialSend(RefHandle firstTarget, RefHandle firstMessage);
        
        /**
         * Decodes send opcodes.
         * Extracts message, target and resultTmp values from instruction stream.
         */
        void decodeSend(RefHandle& msg, RefHandle& target, int& resultTmp);
        
        bool checkConditional();
        void checkInstruction(byte peekbytecode0);
        void checkTempIndexes(byte b0);
        void handleReturn();
        
        void doReturn(RefHandle result, bool withResult);
        
        void handleArrayAtOrAtPut();

        template <typename T>
        void handleArrayAtOrAtPut(byte opcode, T arr);
        
        RefHandle extractArrayLikeFromTemps();
        void handleJump();
        void handleSet();
        void handleArrayLength();
        void handleCreateObject();
        void handleCreateObjectArray();
        void handleInstallExHandler();
        void handleRaiseException();
        void sendToSimpleFunction(RefHandle msg, RefHandle target, int resultTmp);
        void handleSend();
        void sendMessage(RefHandle msg, RefHandle target, int resultTmp);
        void step();
        RefHandle createMetaMessage(RefHandle msg, RefHandle target);        
        void raiseExecError(int code);
        void raiseError(RefHandle excObj);
        
        void execute();

        inline void pushNewContext(int resultTmp)
        {
            cc_->resultTmp_ = word2ref(resultTmp);            
            cc_ = PtrHandle<CallContext>(memory_->createObject<CallContext>()->initNew(RefHandle(ptr2ref(cc_.ptr()), memory_), nullArray_, nullArray_), memory_);
        }

        inline void popContext()
        {
            cc_ = PtrHandle<CallContext>(cc_->parent(), memory_);
        }

        inline void popContext(RefHandle returnValue)
        {
            popContext();
            
            if(cc_->resultTmp() != -1)
            {
                cc_->setTemp(cc_->resultTmp(), returnValue.ref());
            }
        }
        
        inline RefHandle createObjectFromBytecodes(int size)
        {
            PtrHandle<Object> array(memory_->createObject<Object>(size + 1), memory_);

            for(int i = 0; i < size; ++i)
            {
                array->atPut(i, cc_->nextTempFromBytecode());
            }

            return RefHandle(ptr2ref(array.ptr(), true), memory_);
        }

        /**
         * Creates an object array using the byte codes as temp indexes.
         * Assumes the first read bytecode denotes the length.
         */
        inline RefHandle createObjectArrayFromBytecodes(int size)
        {
            PtrHandle<ObjectArray> array(memory_->createObjectArray(size), memory_);

            for(int i = 0; i < size; ++i)
            {
                array->atPut(i, cc_->nextTempFromBytecode());
            }

            return RefHandle(ptr2ref(array.ptr(), true), memory_);
        }

        inline byte peekbytecode(int n = 0)
        {
            return cc_->peekbytecode(n);
        }
        
        inline void setResult(int resultTmp, Ref result)
        {
            if(resultTmp != -1)
            {
                cc_->temps()->atPut(resultTmp, result);
            }
        }
        
        inline void halt()
        {
            halt_ = true;
        }
        
        inline bool halted() const
        {
            return halt_;
        }            
    };
} // namespace atom

#endif /* ATOM_THREAD_HPP_INCLUDED */
