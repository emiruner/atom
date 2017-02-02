/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_ASSEMBLER_TESTHELPER_HPP_INCLUDED
#define ATOM_ASSEMBLER_TESTHELPER_HPP_INCLUDED

#include <gtest/gtest.h>

#define ASSERT_SYMREFNAME_EQ(nm, node) \
    ASSERT_TRUE(dynamic_cast<SymbolicRef*>(node) != 0); \
    ASSERT_EQ(nm, dynamic_cast<SymbolicRef*>(node)->name());

#define ASSERT_INTLIT_EQ(x, node) \
        ASSERT_TRUE(dynamic_cast<IntegerLiteralNode*>(node) != 0); \
        ASSERT_EQ(x, dynamic_cast<IntegerLiteralNode*>(node)->literal());

#define ASSERT_FLOATLIT_EQ(x, node) \
        ASSERT_TRUE(dynamic_cast<FloatLiteralNode*>(node) != 0); \
        ASSERT_FLOAT_EQ(x, dynamic_cast<FloatLiteralNode*>(node)->literal());

#define ASSERT_STRLIT_EQ(x, node) \
        ASSERT_TRUE(dynamic_cast<StringLiteralNode*>(node) != 0); \
        ASSERT_EQ(x, dynamic_cast<StringLiteralNode*>(node)->literal());


#endif

