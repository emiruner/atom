/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <vm/Object.hpp>
#include <vm/Memory.hpp>

#include <gtest/gtest.h>
#include <stdexcept>

using namespace atom;

TEST(ObjectTest, ObjectArray)
{
    Memory mem(1024);
    Ref* rawoa = new Ref[3];

    rawoa[1] = word2ref(61);
    rawoa[2] = word2ref(7);

    Object* oa = mem.createObject<Object>(17);
    ASSERT_EQ(16, oa->size());

    oa = (Object*) rawoa;
    oa->header_.size = 3;

    ASSERT_EQ(61, int_val(oa->at(0)));
    ASSERT_EQ(7, int_val(oa->at(1)));

    // We decrease the size artificially so at method throws exception.
    oa->header_.size = 2;

    try
    {        
        oa->at(1);
        ASSERT_TRUE(false);
    }
    catch(std::runtime_error& ex)
    {
    }

    delete[] rawoa;
}

TEST(ObjectTest, ByteArray)
{
    Memory mem(1024);
    byte data[] = {45, 12, 76};

    Ref baRef = mem.createUnmanagedPrim(ObjectType::BYTE_ARRAY, data, 3);
    ByteArray* ba = cast<ByteArray>(baRef);

    ASSERT_EQ(45, int_val(ba->at(0)));
    ASSERT_EQ(12, int_val(ba->at(1)));
    ASSERT_EQ(76, int_val(ba->at(2)));
    ASSERT_EQ(3, ba->size());

    ba->atPut(1, word2ref(222));

    ASSERT_EQ(45, int_val(ba->at(0)));
    ASSERT_EQ(222, int_val(ba->at(1)));
    ASSERT_EQ(76, int_val(ba->at(2)));

    ASSERT_EQ(45, data[0]);
    ASSERT_EQ(222, data[1]);
    ASSERT_EQ(76, data[2]);
}
