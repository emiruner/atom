/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <vm/Ref.hpp>
#include <vm/Object.hpp>

#include <gtest/gtest.h>

using namespace atom;

TEST(RefTest, Size)
{
#ifdef ATOM_VM_64BITS
    ASSERT_EQ(8UL, sizeof(Ref));
#endif

#ifdef ATOM_VM_32BITS
    ASSERT_EQ(4UL, sizeof(Ref));
#endif
}

TEST(RefTest, Conversion)
{
    Object obj;
    obj.header_.objtype = ObjectType::OBJECT;

    Ref ref = ptr2ref(&obj);
    ASSERT_EQ(&obj, cast<Object>(ref));
}

Ref ptrAsRef(uword ptr)
{
    Ref r;
    void** v = (void**) &r;
    *v = (void*) ptr;
    return r;
}

TEST(RefTest, FieldValues)
{
#ifdef ATOM_VM_64BITS
    Ref ref;

    ref = ptrAsRef(1);
    ASSERT_TRUE(is_int(ref));
    ASSERT_EQ(0, int_val(ref));

    ref = ptrAsRef(3);
    ASSERT_TRUE(is_int(ref));
    ASSERT_EQ(1, int_val(ref));

    ref = ptrAsRef(0xFFFFFFFFFFFFFFFC);
    ASSERT_FALSE(is_int(ref));
    ASSERT_EQ((void*) 0xFFFFFFFFFFFFFFFC, ptr_val(ref));
    ASSERT_FALSE(array_access(ref));

    ref = ptrAsRef(0x9544343A2FE2433C);
    ASSERT_FALSE(is_int(ref));
    ASSERT_EQ((void*) 0x9544343A2FE2433C, ptr_val(ref));
    ASSERT_FALSE(array_access(ref));

    ref = ptrAsRef(0x9544343A2FE2433E);
    ASSERT_FALSE(is_int(ref));
    ASSERT_EQ((void*) 0x9544343A2FE2433C, ptr_val(ref));
    ASSERT_TRUE(array_access(ref));
    ref = clear_array_access(ref);
    ASSERT_FALSE(array_access(ref));
    ASSERT_EQ((void*) 0x9544343A2FE2433C, ptr_val(ref));
    ref = set_array_access(ref);
    ASSERT_TRUE(array_access(ref));
    ASSERT_EQ((void*) 0x9544343A2FE2433C, ptr_val(ref));


    ref = ptrAsRef(0x3FFFFFFFFFFFFFFF);
    ASSERT_TRUE(is_int(ref));
    ASSERT_EQ(0x1FFFFFFFFFFFFFFF, int_val(ref));
#endif

#ifdef ATOM_VM_32BITS
#error Write the test!!!!
#endif
}
