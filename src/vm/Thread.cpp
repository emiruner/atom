/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Thread.hpp"
#include "Opcode.hpp"

#include <stdexcept>
#include <signal.h>

using namespace atom;

namespace
{

// send $0 to $1 > $0
const byte startBallRollingBytecodesBytes[] = {Opcode::SEND_VAL_TO_VAL_WRES, 0, 1, 0};

} // namespace <anonymous>

namespace atom
{
    Thread::Thread(Memory* memory)
    : memory_(memory), halt_(false), cc_(PtrHandle<CallContext>(0, memory_)), sendCount_(0), bytecodeCount_(0)
    {
        startBallRollingBytecodes_ = RefHandle(memory_->createUnmanagedByteArray((byte*) startBallRollingBytecodesBytes, sizeof(startBallRollingBytecodesBytes)), memory_);
        nullArray_ = RefHandle(ptr2ref(memory_->createObjectArray(0)), memory_);
        execErrorBa_ = RefHandle(memory_->createByteArrayFromString("execError"), memory_);
        processExceptionBa_ = RefHandle(memory_->createByteArrayFromString("processException"), memory_);
    }

    void Thread::prepareInitialSend(RefHandle firstTarget, RefHandle firstMessage)
    {
        cc_ = PtrHandle<CallContext>(memory_->createObject<CallContext>(), memory_);
        cc_->initNew(nullArray_, nullArray_, nullArray_);
        cc_->initBytecodesTemps(startBallRollingBytecodes_.ref(), memory_->createObjectArray(firstMessage, firstTarget));
    }

    bool Thread::checkConditional()
    {
        byte opcode = cc_->nextbytecode();
        Ref cond = cc_->nextTempFromBytecode();

        if(Opcode::CONDITIONAL_ONE == opcode)
        {
            return (is_int(cond) && int_val(cond) == 1);
        }
        else
        {
            return (!is_int(cond) || int_val(cond) != 1);
        }
    }

    void Thread::decodeSend(RefHandle& msg, RefHandle& target, int& resultTmp)
    {
        byte opcode = cc_->nextbytecode();

        msg = RefHandle(cc_->nextTempFromBytecode(), memory_);
        target = RefHandle(cc_->nextTempFromBytecode(), memory_);
               
        resultTmp = opcode == Opcode::SEND_VAL_TO_VAL_WRES ? cc_->nextbytecode() : -1;
    }

    RefHandle Thread::createMetaMessage(RefHandle msg, RefHandle target)
    {
        PtrHandle<ObjectArray> metaMessage(memory_->createObjectArray(3), memory_);

        metaMessage->atPut(MetaMessageElements::ZERO, zeroRef());
        
        if(is_int(target.ref()))
        {
            metaMessage->atPut(MetaMessageElements::TARGET, target.ref());
        }
        else
        {
            metaMessage->atPut(MetaMessageElements::TARGET, set_array_access(target.ref()));
        }
        
        metaMessage->atPut(MetaMessageElements::REALMSG, msg.ref());
        
        return RefHandle(ptr2ref(metaMessage.ptr(), true), memory_);
    }

    void Thread::checkInstruction(byte first)
    {
        cc_->ensureEnoughBytecodes(2);

        if(!Opcode::isValid(first))
        {
            throw invalid_opcode_error();
        }

        byte second = peekbytecode(1);
        
        cc_->ensureEnoughBytecodes(Opcode::instructionSize(first, second));
    }

    void Thread::checkTempIndexes(byte b0)
    {
        byte b1 = peekbytecode(1);
        
        // If instruction is not a variable length instruction then the second bytecode
        // points to a temp index, so check it.
        if(!Opcode::isVariableLength(b0))
        {
            cc_->validateTemp(b1);
        }

        int isize = Opcode::instructionSize(b0, b1);

        for(int i = 2; i < isize; ++i)
        {
            cc_->validateTemp(peekbytecode(i));
        }
    }

    void Thread::doReturn(RefHandle result, bool withResult)
    {
        if(cc_->parent_ == nullArray_.ref())
        {
            halt();
        }
        else
        {
            if(withResult)
            {
                popContext(result);
            }
            else
            {
                popContext();
            }
            
            if(cc_->exceptionObject_ != nullArray_.ref())
            {
                RefHandle exceptionObject(cc_->exceptionObject_, memory_);
                
                doReturn(RefHandle(zeroRef(), memory_), false);
                
                // If exception handler returned a zero value this indicates that
                // exception handler cannot handle this exception so continue propagating.
                if(withResult && result.ref() == zeroRef())
                {
                    raiseError(exceptionObject);                    
                }
            }
        }    
    }

    void Thread::handleReturn()
    {
        byte opcode = cc_->nextbytecode();
        
        if(Opcode::RETURN_RESULT == opcode)
        {
            doReturn(RefHandle(cc_->nextTempFromBytecode(), memory_), true);
        }
        else
        {
            doReturn(RefHandle(zeroRef(), memory_), false);
        }
    }

    void Thread::handleCreateObjectArray()
    {
        cc_->skipBytecodes(1);

        RefHandle oa(createObjectArrayFromBytecodes(cc_->nextbytecode()));
        cc_->temps()->atPut(cc_->nextbytecode(), oa.ref());
    }

    void Thread::handleCreateObject()
    {
        cc_->skipBytecodes(1);

        RefHandle oa(createObjectFromBytecodes(cc_->nextbytecode()));
        cc_->temps()->atPut(cc_->nextbytecode(), oa.ref());
    }

    template <typename T>
    inline void Thread::handleArrayAtOrAtPut(byte opcode, T arr)
    {
        Ref indexRef = cc_->nextTempFromBytecode();

        if(!is_int(indexRef))
        {
            throw non_integer_array_index_error();
        }

        word index = int_val(indexRef);

        if(opcode == Opcode::ARRAY_AT)
        {
            cc_->temps()->atPut(cc_->nextbytecode(), arr->at(index));
        }
        else
        {
            arr->atPut(index, cc_->nextTempFromBytecode());
        }
    }

    RefHandle Thread::extractArrayLikeFromTemps()
    {
        Ref arrayRef = cc_->nextTempFromBytecode();
        
        if(!array_access(arrayRef))
        {            
            throw array_access_disabled_error();
        }

        if(!is_byte_array(arrayRef) && !ObjectType::containsSlots(obj_type(arrayRef)))
        {
            throw non_indexable_object_error();
        }

        return RefHandle(arrayRef, memory_);
    }

    void Thread::handleArrayAtOrAtPut()
    {
        byte opcode = cc_->nextbytecode();        
        RefHandle arrayRef = extractArrayLikeFromTemps();

        if(is_byte_array(arrayRef.ref()))
        {
            handleArrayAtOrAtPut(opcode, cast<ByteArray>(arrayRef.ref()));
        }
        else
        {
            handleArrayAtOrAtPut(opcode, cast<ObjectArray>(arrayRef.ref()));
        }
    }

    void Thread::handleArrayLength()
    {
        cc_->skipBytecodes(1);
        Ref arrayRef = cc_->nextTempFromBytecode();

        if(!array_access(arrayRef))
        {
            throw array_access_disabled_error();
        }
        
        if(is_byte_array(arrayRef))
        {
            cc_->temps()->atPut(cc_->nextbytecode(), word2ref(cast<ByteArray>(arrayRef)->size()));
        }
        else
        {
            cc_->temps()->atPut(cc_->nextbytecode(), word2ref(cast<ObjectArray>(arrayRef)->size()));
        }
    }

    void Thread::raiseExecError(int code)
    {
        raiseError(RefHandle(ptr2ref(memory_->createObjectArray(execErrorBa_, RefHandle(word2ref(code), memory_)), true), memory_));
    }

    void Thread::raiseError(RefHandle excObj)
    {
        while(true)
        {
            if(cc_->exceptionHandler_ == nullArray_.ref())
            {
                if(cc_->parent_ == nullArray_.ref())
                {
                    DEBUG("No exception handler found. Halting.\n");
                    halt();
                    break;
                }
                else
                {
                    // TODO: May record unwound stack frames to have a stack trace!
                    popContext();
                }
            }
            else
            {   
                if(cc_->exceptionObject_ == nullArray_.ref())
                {
                    cc_->exceptionObject_ = excObj.ref();
                    
                    try
                    {
                        sendMessage(RefHandle(ptr2ref(memory_->createObjectArray(processExceptionBa_, excObj), true), memory_), RefHandle(cc_->exceptionHandler_, memory_), -1);
                    }
                    catch(vm_error const& ex)
                    {
                        // If an exception occurs while trying to invoke exception handler,
                        // stop using this exception handler and propagate new exception to caller.
                        raiseExecError(ex.code());
                    }

                    break;
                }
                else
                {
                    // If cc's exception object is not null then an exception has occured
                    // while handling an exception using this cc's exception handler
                    // so we cannot use this cc's exception handler.
                    popContext();
                }
            }                       
        }
    }

    void Thread::sendToSimpleFunction(RefHandle msg, RefHandle target, int resultTmp)
    {
        PtrHandle<ObjectArray> fn(cast<ObjectArray>(target.ref()), memory_);
        const int size = fn->size();

        if(size < 2 || !is_byte_array(fn->at(0)) || !is_int(fn->at(1)))
        {
            throw invalid_simple_function_structure_error();
        }
    
        const word tempCount = int_val(fn->at(1));
        
        PtrHandle<ObjectArray> temps(memory_->createObjectArray(size + tempCount), memory_);

        temps->atPut(0, msg.ref());
        temps->atPut(1, zeroRef()); // Always a temp present.

        for(int i = 2; i < size; ++i)
        {
            temps->atPut(i, fn->at(i));
        }

        pushNewContext(resultTmp);
        cc_->initBytecodesTemps(fn->at(0), temps.ptr());
    }

    void Thread::sendMessage(RefHandle msg, RefHandle target, int resultTmp)
    {
        if(is_native_fn(target.ref()))
        {
            cast<NativeFunction>(target.ref())->call(this, resultTmp, msg.ref());
        }
        else if(is_simple_fn(target.ref()))
        {
            sendToSimpleFunction(msg, target, resultTmp);
        }
        else
        {
            RefHandle metaObject(memory_->findMetaObject(target.ref()), memory_);
            
            if(metaObject.ref() == target.ref())
            {
                throw recursive_metaobject_error();
            }
            
            sendMessage(createMetaMessage(msg, target), metaObject, resultTmp);
        }
    }

    void Thread::handleSend()
    {
        RefHandle msg;
        RefHandle target;
        int resultTmp;

        decodeSend(msg, target, resultTmp);
        sendMessage(msg, target, resultTmp);
    }

    void Thread::handleInstallExHandler()
    {
        cc_->skipBytecodes(1);
        cc_->exceptionHandler_ = cc_->nextTempFromBytecode();
    }

    void Thread::handleRaiseException()
    {
        cc_->skipBytecodes(1);
        raiseError(RefHandle(cc_->nextTempFromBytecode(), memory_));
    }

    void Thread::handleJump()
    {
        cc_->skipBytecodes(1);
        Ref newIpRef = cc_->nextTempFromBytecode();
        
        if(!is_int(newIpRef))
        {
            throw non_integer_jump_offset();
        }
        
        int newIp = int_val(newIpRef);
        
        if(newIp < 0 || newIp >= cc_->bytecodes()->size())
        {
            throw jump_offset_not_in_bounds();
        }
        
        cc_->ip(newIp);
    }

    void Thread::handleSet()
    {
        cc_->skipBytecodes(1);
        
        int target = cc_->nextbytecode();
        int src = cc_->nextbytecode();
        
        cc_->temps()->atPut(target, cc_->temps()->at(src));
    }
    
    void Thread::step()
    {
        ++bytecodeCount_;
        
        if(halt_)
        {
            throw vm_was_halted_error();
        }

        if(!cc_->hasMoreBytecodes(1))
        {
            doReturn(RefHandle(zeroRef(), memory_), false);
            return;
        }
        
        byte opcode = peekbytecode();
        
        if(Opcode::HALT == opcode)
        {
            cc_->skipBytecodes(1);
            halt_ = true;
            return;
        }

        checkInstruction(opcode);

        if(Opcode::isConditional(opcode))
        {
            bool condTrue = checkConditional();
            opcode = peekbytecode();
            checkInstruction(opcode);
            
            if(Opcode::isConditional(opcode))
            {
                throw double_conditional_error();
            }

            if(!condTrue)
            {
                cc_->skipBytecodes(Opcode::instructionSize(opcode, peekbytecode(1)));
                return;
            }
        }

        opcode = peekbytecode();
        
        checkTempIndexes(opcode);

        if(Opcode::isReturn(opcode))
        {
            handleReturn();
        }
        else if(Opcode::ARRAY_AT == opcode || Opcode::ARRAY_AT_PUT == opcode)
        {
            handleArrayAtOrAtPut();
        }
        else if(Opcode::ARRAY_LENGTH == opcode)
        {
            handleArrayLength();
        }
        else if(Opcode::CREATE_OBJECT == opcode)
        {
            handleCreateObject();
        }
        else if(Opcode::CREATE_OBJECT_ARRAY == opcode)
        {
            handleCreateObjectArray();
        }
        else if(Opcode::INSTALL_EXCEPTION_HANDLER == opcode)
        {
            handleInstallExHandler();
        }
        else if(Opcode::RAISE_EXCEPTION == opcode)
        {
            handleRaiseException();
        }
        else if(Opcode::JUMP == opcode)
        {
            handleJump();
        }
        else if(Opcode::SET_LOCAL == opcode)
        {
            handleSet();
        }
        else if(Opcode::SEND_VAL_TO_VAL == opcode || Opcode::SEND_VAL_TO_VAL_WRES  == opcode)
        {
            ++sendCount_;
            handleSend();
        }
        else
        {
            throw std::runtime_error("must not reach here");
        }
    }
    
    void Thread::execute()
    {
        if(halted())
        {
            DEBUG("VM is already halted. Cannot reanimate.\n");
        }
        else
        {
            while(!halted())
            {
                try
                {
                    step();
                }
                catch(memory_exhausted_error const& ex)
                {
                    // Cannot recover from memory exhausted error.
                    DEBUG("VM memory is exhausted. Halting.");
                    halt();
                }
                catch(vm_error const& ex)
                {
                    DEBUG("An error occurred while executing: " << ex.what() << "\n");
                    raiseExecError(ex.code());
                }
            }
        }
    }

} // namespace atom


