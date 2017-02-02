/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <vm/Memory.hpp>
#include <vm/Object.hpp>

#include <gtest/gtest.h>

using namespace atom;

struct TestObject
{
    static const int type = 7;

    ObjectHeader header;
    Ref val;
};

TEST(ObjectTest, Size)
{
#ifdef ATOM_VM_64BITS
    ASSERT_EQ(24UL, sizeof(PrimDataObject));
#endif

#ifdef ATOM_VM_32BITS
    ASSERT_EQ(12UL, sizeof(PrimDataObject));
#endif
}

TEST(ObjectTest, CreatePrim)
{
    Memory mem(1024);
    char buf[128];

    Ref prim = mem.createUnmanagedPrim(4, &buf, 128);
    PrimDataObject* rp = (PrimDataObject*) ptr_val(prim);

    ASSERT_EQ(rp->data_, buf);
    ASSERT_EQ(3, rp->header_.size);
    ASSERT_EQ(0, rp->header_.mark);
    ASSERT_EQ(4, rp->header_.objtype);
    ASSERT_TRUE(rp->managed_ == 0);

    ASSERT_EQ(128, rp->size());

    prim = mem.createManagedPrim(4, &buf, 128);
    rp = (PrimDataObject*) ptr_val(prim);

    ASSERT_EQ(rp->data_, buf);
    ASSERT_EQ(3, rp->header_.size);
    ASSERT_EQ(0, rp->header_.mark);
    ASSERT_EQ(4, rp->header_.objtype);
    ASSERT_TRUE(rp->managed_ == 1);
    ASSERT_EQ(128, rp->size());
}

TEST(ObjectTest, CreateObject)
{
    Memory mem(1024);

    TestObject* obj = mem.createObject<TestObject>();

    ASSERT_EQ(0, obj->header.mark);
    ASSERT_EQ(7, obj->header.objtype);
    ASSERT_EQ(2, obj->header.size);
    ASSERT_EQ(0, int_val(obj->val));
}

TEST(ObjectTest, CreateFloat)
{
    Memory mem(1024);

    ASSERT_EQ(mem.toSpace_->free_, mem.toSpace_->start_);
    Ref r = mem.createFloat(678.344);
    ASSERT_EQ(mem.toSpace_->free_, mem.toSpace_->start_ + 2);

    ASSERT_TRUE(is_float(r));
    ASSERT_FALSE(is_int(r));
    ASSERT_DOUBLE_EQ(678.344, float_val(r));
}
