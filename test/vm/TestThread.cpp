/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <cstring>
#include <vm/Thread.hpp>

#include <gtest/gtest.h>
#include <stdexcept>
#include <ostream>

using namespace atom;

inline std::ostream& operator<<(std::ostream& os, Ref r)
{
    return os;
}

void add1(Thread* thread, int resultTmp, Ref message)
{
    thread->setResult(resultTmp, word2ref(int_val(message) + 1));
}

bool simpleMetaobjectCalled;

void simpleMetaobject(Thread* thread, int resultTmp, Ref message)
{
    simpleMetaobjectCalled = true;
}

bool fun1Called;

void fun1(Thread* thread, int resultTmp, Ref message)
{
    fun1Called = true;
    thread->setResult(resultTmp, zeroRef());
}

void simpleObjectArrayMetaobject(Thread* thread, int resultTmp, Ref metaMessage)
{
    ASSERT_TRUE(is_object_array(metaMessage));

    ObjectArray* mm = cast<ObjectArray>(metaMessage);
    
    Ref targetRef = mm->at(MetaMessageElements::TARGET);
    Ref realMsg = mm->at(MetaMessageElements::REALMSG);
    
    ASSERT_TRUE(is_object(targetRef));
    Object* target = cast<Object>(targetRef);

    if(is_int(realMsg))
    {
        thread->setResult(resultTmp, target->at(int_val(realMsg) + 1));
    }
    else
    {
        ASSERT_TRUE(is_object_array(realMsg));
        ObjectArray* msg = cast<ObjectArray>(realMsg);

        target->atPut(int_val(msg->at(0)) + 1, msg->at(1));
    }
}

void exceptionObjectTestExceptionHandler(Thread* thread, int resultTmp, Ref msg)
{
    ASSERT_TRUE(is_object_array(msg));
    ASSERT_TRUE(array_access(msg));
    
    ObjectArray* oa = cast<ObjectArray>(msg);
    ASSERT_EQ(thread->processExceptionBa_.ref(), oa->at(0));
    ASSERT_TRUE(array_access(oa->at(0)));
    ASSERT_TRUE(array_access(oa->at(1)));
    
    ObjectArray* inner = cast<ObjectArray>(oa->at(1));
    ASSERT_TRUE(array_access(inner->at(0)));
}

class ThreadTest : public ::testing::Test
{
protected:
    Memory* mem;
    Thread* thread;
    byte* bytes3101;
    byte* bytes1234;
    byte* bytes0123;

    virtual void SetUp()
    {
        bytes3101 = new byte[4];
        memcpy(bytes3101, "\003\001\000\001", 4);

        bytes1234 = new byte[4];
        memcpy(bytes1234, "\001\002\003\004", 4);

        bytes0123 = new byte[4];
        memcpy(bytes0123, "\000\001\002\003", 4);

        mem = new Memory(1024);
        thread = new Thread(mem);
        thread->prepareInitialSend(RefHandle(zeroRef(), mem), RefHandle(zeroRef(), mem));
    }

    virtual void TearDown()
    {
        delete[] bytes3101;
        delete[] bytes1234;
        delete[] bytes0123;
        delete mem;
        delete thread;
    }
};

TEST_F(ThreadTest, RaisedObjectTest)
{
    thread->cc_->exceptionHandler_ = mem->createNativeFunction(&exceptionObjectTestExceptionHandler);
    thread->raiseExecError(3);
}

TEST_F(ThreadTest, InitialSend)
{
    thread->prepareInitialSend(RefHandle(mem->createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) &add1, 0), mem), RefHandle(word2ref(61), mem));

    thread->step();
    ASSERT_EQ(62L, int_val(thread->cc_->temps()->at(0)));
}

TEST_F(ThreadTest, Execute)
{
    thread->prepareInitialSend(RefHandle(mem->createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) &add1, 0), mem), RefHandle(word2ref(61), mem));
    thread->execute();

    ASSERT_EQ(62L, int_val(thread->cc_->temps()->at(0)));
}

TEST_F(ThreadTest, StepMetaobjectNotFound)
{
    thread->prepareInitialSend(RefHandle(word2ref(3L), mem), RefHandle(word2ref(4L), mem));

    // We got this error at the second step when we try to send nil, which has nil as metaobject.
    try
    {
        thread->step();
        ASSERT_TRUE(false);
    }
    catch(std::runtime_error& ex)
    {
    }
}

TEST_F(ThreadTest, NextbytecodeAndHasMoreBytecodes)
{
    thread->cc_->initBytecodesTemps(mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, bytes0123, 4), 0);

    ASSERT_TRUE(thread->cc_->hasMoreBytecodes(4));
    ASSERT_EQ(0, (int) thread->cc_->nextbytecode());
    ASSERT_TRUE(thread->cc_->hasMoreBytecodes(3));
    ASSERT_EQ(1, (int) thread->cc_->nextbytecode());
    ASSERT_TRUE(thread->cc_->hasMoreBytecodes(2));
    ASSERT_EQ(2, (int) thread->cc_->nextbytecode());
    ASSERT_TRUE(thread->cc_->hasMoreBytecodes(1));
    ASSERT_EQ(3, (int) thread->cc_->nextbytecode());
    ASSERT_TRUE(!thread->cc_->hasMoreBytecodes(1));
}

TEST_F(ThreadTest, Bytecodes)
{
    thread->cc_->initBytecodesTemps(mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, bytes0123, 4), 0);
    ASSERT_EQ(bytes0123, thread->cc_->bytecodes()->data_);
}

TEST_F(ThreadTest, FindMetaObject)
{
    mem->integerMo_ = RefHandle(word2ref(2L), mem);
    mem->byteArrayMo_ = RefHandle(word2ref(4L), mem);

    Object* o = mem->createObject<Object>();
    o->metaObject_ = word2ref(7L);

    Ref ba = mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, 0, 0);

    ASSERT_EQ((word) 2, (word) int_val(mem->findMetaObject(word2ref(453L))));
    ASSERT_EQ((word) 4, (word) int_val(mem->findMetaObject(ba)));
    ASSERT_EQ((word) 7, (word) int_val(mem->findMetaObject(ptr2ref(o))));
}

TEST_F(ThreadTest, NextTempFromBytecode)
{
    Ref bytecodes = mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, bytes0123, 4);

    thread->cc_->initBytecodesTemps(bytecodes, mem->createObjectArray(RefHandle(word2ref(55), mem), RefHandle(word2ref(77), mem), RefHandle(word2ref(88), mem)), 1);

    Ref result = thread->cc_->nextTempFromBytecode();

    ASSERT_EQ(77L, int_val(result));
    ASSERT_EQ(2L, thread->cc_->ip());
}

TEST_F(ThreadTest, SendingToObjectWithANativeFunctionMetaObject)
{
    Object* o = mem->createObject<Object>();
    o->metaObject_ = mem->createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) simpleMetaobject, 0);

    simpleMetaobjectCalled = false;
    thread->prepareInitialSend(RefHandle(ptr2ref(o), mem), RefHandle(word2ref(65L), mem));
    thread->execute();
    ASSERT_EQ(true, simpleMetaobjectCalled);
}

TEST_F(ThreadTest, SimpleObjectArrayMetaobjectAtPut)
{
    Object* target = mem->createObject<Object>(6);
    target->atPut(0, mem->createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) simpleObjectArrayMetaobject, 0));
    target->atPut(1, word2ref(23L));
    target->atPut(2, word2ref(47L));
    target->atPut(3, word2ref(12L));
    target->atPut(4, word2ref(89L));

    ObjectArray* msg = mem->createObjectArray(RefHandle(word2ref(2), mem), RefHandle(word2ref(56), mem));

    ASSERT_EQ(12L, int_val(target->at(3)));
    thread->prepareInitialSend(RefHandle(ptr2ref(target), mem), RefHandle(ptr2ref(msg), mem));
    thread->execute();
    ASSERT_EQ(56L, int_val(target->at(3)));
}

TEST_F(ThreadTest, SimpleObjectArrayMetaobjectAt)
{
    Object* target = mem->createObject<Object>(6);
    target->atPut(0, mem->createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) simpleObjectArrayMetaobject, 0));
    target->atPut(1, word2ref(23L));
    target->atPut(2, word2ref(47L));
    target->atPut(3, word2ref(12L));
    target->atPut(4, word2ref(89L));

    ASSERT_EQ(47L, int_val(target->at(2)));
    thread->prepareInitialSend(RefHandle(ptr2ref(target), mem), RefHandle(word2ref(1L), mem));

    ASSERT_EQ(1L, int_val(thread->cc_->temps()->at(0)));
    thread->execute();
    ASSERT_EQ(47L, int_val(thread->cc_->temps()->at(0)));
}

TEST_F(ThreadTest, InitRegs)
{
    Ref bytecodes = mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, bytes3101, 4);
    ObjectArray* temps = mem->createObjectArray(RefHandle(word2ref(34), mem), RefHandle(word2ref(7), mem), RefHandle(word2ref(5), mem));

    thread->cc_->initBytecodesTemps(bytecodes, temps, 77);

    ASSERT_EQ(bytecodes, thread->cc_->bytecodes_);
    ASSERT_EQ(temps, thread->cc_->temps());
    ASSERT_EQ(77, thread->cc_->ip());
    ASSERT_EQ(34L, int_val(temps->at(0)));
}

TEST_F(ThreadTest, CreateObjectArrayFromBytecodes)
{
    thread->cc_->initBytecodesTemps(mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, bytes3101, 4), mem->createObjectArray(RefHandle(word2ref(7), mem), RefHandle(word2ref(5), mem)));

    RefHandle oaref = thread->createObjectArrayFromBytecodes(thread->cc_->nextbytecode());

    ObjectArray* oa = cast<ObjectArray>(oaref.ref());

    ASSERT_EQ((word) 3, oa->size());
    ASSERT_EQ(5L, int_val(oa->at(0)));
    ASSERT_EQ(7L, int_val(oa->at(1)));
    ASSERT_EQ(5L, int_val(oa->at(2)));
}

TEST_F(ThreadTest, CreateObjectArray)
{
    ObjectArray* oa = mem->createObjectArray(40);

    ASSERT_EQ((int) ObjectType::OBJECT_ARRAY, (int) oa->header_.objtype);
    ASSERT_EQ(40L, oa->size());
}

TEST_F(ThreadTest, CreateByteArray)
{
    Ref baref = mem->createUnmanagedByteArray(bytes1234, 4);
    ByteArray* ba = cast<ByteArray>(baref);

    ASSERT_EQ((int) ObjectType::BYTE_ARRAY, (int) ba->header_.objtype);
    ASSERT_EQ((word) 4, ba->size());
    ASSERT_EQ(3, (int) ba->data()[2]);
}


TEST_F(ThreadTest, ContinuationCall)
{
    // TODO: Write test for Continuation::call
}
