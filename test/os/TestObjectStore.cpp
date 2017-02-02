/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <string.h>

#include <vm/Object.hpp>
#include <vm/Thread.hpp>
#include <os/ObjectStore.hpp>

#include <gtest/gtest.h>

using namespace atom;

class ObjectStoreTest : public ::testing::Test
{
protected:
    Memory* mem;
    Thread* thread;

    virtual void SetUp()
    {
        mem = new Memory(1024);
        thread = new Thread(mem);
    }

    virtual void TearDown()
    {
        delete mem;
        delete thread;
    }
    
    RefHandle intRef(word w)
    {
        return RefHandle(word2ref(w), mem);
    }
    
    RefHandle ptrRef(void* ptr)
    {
        return RefHandle(ptr2ref(ptr), mem);
    }
};

TEST_F(ObjectStoreTest, SimpleStore1)
{
    ObjectArray* child = mem->createObjectArray(0);
    ObjectArray* parent = mem->createObjectArray(1);
    parent->atPut(0, ptr2ref(child));

    ObjectStoreWriter osw("test0.os");
    osw.addObject(ptr2ref(parent));
    osw.flush();
}

TEST_F(ObjectStoreTest, SimpleStore2)
{
    ObjectArray* oa1 = mem->createObjectArray(intRef(1), intRef(2));
    ObjectArray* oa2 = mem->createObjectArray(intRef(3), ptrRef(oa1));

    ObjectStoreWriter osw("test1.os");
    osw.addObject(ptr2ref(oa2));
    osw.flush();
}

TEST_F(ObjectStoreTest, Cycle)
{
    ObjectArray* oa1 = mem->createObjectArray(2);
    ObjectArray* oa2 = mem->createObjectArray(2);
    ObjectArray* oa3 = mem->createObjectArray(2);

    oa1->atPut(0, word2ref(1L));
    oa1->atPut(1, ptr2ref(oa2));

    oa2->atPut(0, word2ref(2L));
    oa2->atPut(1, ptr2ref(oa3));

    oa3->atPut(0, word2ref(3L));
    oa3->atPut(1, ptr2ref(oa1));

    ObjectStoreWriter osw("test2.os");
    osw.addObject(ptr2ref(oa2));
    osw.flush();
}

TEST_F(ObjectStoreTest, ByteArrayAndComplexWrite)
{
    // TODO: Fix later.
    /*
    char* buf1 = (char*) "\xd\xe\xa\xd\xb\xe\xe\xf";
    Ref ba1 = mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, buf1, 8);

    char* buf2 = (char*) "\xc\xa\xf\xe\xd";
    Ref ba2 = mem->createUnmanagedPrim(ObjectType::BYTE_ARRAY, buf2, 5);

    ObjectArray* temps = mem->createObjectArray(word2ref(254L), word2ref(1022L));

    Continuation* c = mem->createObject<Continuation>();
    c->ip_ = word2ref(55L);
    c->bytecodes_ = ba2;
    c->temps_ = ptr2ref(temps);
    c->resultTmp_ = word2ref(2L);

    ObjectArray* oa = mem->createObjectArray(ba1, ptr2ref(c), ba2);

    ObjectStoreWriter osw("test3.os");
    osw.addObject(oa);
    osw.flush()
    */
}

TEST_F(ObjectStoreTest, ObjectStoreLoad)
{
    // TODO: Fix later.
    /*
    ObjectStoreReader osr("test3.os", mem);

    Object* all = osr.readAll();

    ASSERT_NE((void*) 0, all);
    ASSERT_EQ(5L, all->size());

    ASSERT_TRUE(is_object_array(all->at(0)));
    ASSERT_TRUE(is_byte_array(all->at(1)));
    ASSERT_TRUE(is_continuation(all->at(2)));
    ASSERT_TRUE(is_byte_array(all->at(3)));
    ASSERT_TRUE(is_object_array(all->at(4)));

    ObjectArray* oa = cast<ObjectArray>(all->at(0));
    ASSERT_EQ(3L, oa->size());

    ASSERT_TRUE(is_byte_array(oa->at(0)));
    ByteArray* ba1 = cast<ByteArray>(oa->at(0));

    ASSERT_EQ(8L, ba1->size());
    ASSERT_EQ(0, strncmp((char const*) ba1->data(), "\xd\xe\xa\xd\xb\xe\xe\xf", ba1->size()));

    ASSERT_TRUE(is_continuation(oa->at(1)));
    Continuation* c = cast<Continuation>(oa->at(1));

    ASSERT_EQ(55L, int_val(c->ip_));
    ASSERT_EQ(ptr_val(c->bytecodes_), cast<ByteArray>(oa->at(2)));

    ASSERT_TRUE(is_object_array(c->temps_));
    ASSERT_EQ(2L, cast<ObjectArray>(c->temps_)->size());
    ASSERT_EQ(254L, int_val(cast<ObjectArray>(c->temps_)->at(0)));
    ASSERT_EQ(1022L, int_val(cast<ObjectArray>(c->temps_)->at(1)));
    ASSERT_EQ(2L, int_val(c->resultTmp_));

    ASSERT_TRUE(is_byte_array(oa->at(2)));

    ByteArray* ba2 = cast<ByteArray>(oa->at(2));
    ASSERT_EQ(5L, ba2->size());
    ASSERT_EQ(0, strncmp((char const*) ba2->data(), "\xc\xa\xf\xe\xd", ba2->size()));
    */
}
