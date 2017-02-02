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

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <assembler/Lexer.hpp>
#include <assembler/Parser.hpp>
#include <assembler/Assembler.hpp>

#include <vm/Opcode.hpp>

#include "TestHelper.hpp"

using namespace atom;
using namespace atom::assembler;

#define ASSERT_IS_CSTRING(str, ref) ASSERT_TRUE(is_byte_array(ref)); ASSERT_EQ(str, std::string((char const*) (cast<ByteArray>(ref)->data())));

TEST(AssemblerTest, Simple)
{
    /*
    std::ifstream in("../test/simple.z");

    ASSERT_TRUE(in.is_open());

    Lexer lexer(in, std::string("-"));
    Parser parser(lexer);

    try
    {
        parser.parse();
    }
    catch(std::runtime_error const& e)
    {
        std::cerr << "An exception caught: " << e.what() << "\nBut continuing with test for partial checking.\n";
    }

    Memory mem(10240);
    Assembler* as = new Assembler(mem, parser.toplevelNodes());
    as->assemble();

    ObjectArray* oa = as->result();

    ASSERT_TRUE(oa != 0);
    ASSERT_EQ(8, oa->size());

    ASSERT_IS_CSTRING("hello, world!", oa->at(0));

    ASSERT_TRUE(is_simple_fn(oa->at(1)));
    SimpleFunction* fn = cast<SimpleFunction>(oa->at(1));

    ASSERT_TRUE(is_byte_array(fn->bytecodes_));
    ByteArray* b1 = cast<ByteArray>(fn->bytecodes_);

    ASSERT_EQ(7, b1->size());
    ASSERT_EQ(12, int_val(b1->at(0)));
    ASSERT_EQ(45, int_val(b1->at(1)));
    ASSERT_EQ(34, int_val(b1->at(2)));
    ASSERT_EQ(78, int_val(b1->at(3)));
    ASSERT_EQ(0, int_val(b1->at(4)));
    ASSERT_EQ(0, int_val(b1->at(5)));
    ASSERT_EQ(90, int_val(b1->at(6)));

    ASSERT_EQ(5, int_val(fn->tempCount_));

    ASSERT_TRUE(is_object(fn->at(2)));
    Object* foo = cast<Object>(fn->at(2));

    ASSERT_TRUE(fn == ptr_val(foo->at(0)));

    ASSERT_TRUE(is_object(foo->at(1)));
    Object* coords = cast<Object>(foo->at(1));
    ASSERT_TRUE(fn == ptr_val(coords->at(0)));
    ASSERT_FLOAT_EQ(56.333, cast<FloatObject>(coords->at(1))->value_);
    ASSERT_EQ(ptr_val(oa->at(0)), ptr_val(coords->at(2)));
    ASSERT_IS_CSTRING("what?", coords->at(3));

    ASSERT_EQ(45, int_val(fn->at(3)));

    ASSERT_TRUE(coords == ptr_val(oa->at(2)));
    ASSERT_TRUE(foo == ptr_val(oa->at(3)));
    ASSERT_EQ(45, int_val(oa->at(4)));
    ASSERT_FLOAT_EQ(3.141592, cast<FloatObject>(oa->at(5))->value_);
    ASSERT_TRUE(b1 == ptr_val(oa->at(6)));

    ASSERT_TRUE(is_object(oa->at(7)));
    Object* o7 = cast<Object>(oa->at(7));

    ASSERT_TRUE(b1 == ptr_val(o7->at(0)));
    ASSERT_EQ(88, int_val(o7->at(1)));
    */
}

TEST(AssemblerTest, VarDef)
{
    /*
    std::ifstream in("../test/vardef.z");

    ASSERT_TRUE(in.is_open());

    Lexer lexer(in, std::string("-"));
    Parser parser(lexer);

    try
    {
        parser.parse();
    }
    catch(std::runtime_error const& e)
    {
        std::cerr << "An exception caught: " << e.what() << "\nBut continuing with test for partial checking.\n";
    }

    Memory mem(10240);
    Assembler* as = new Assembler(mem, parser.toplevelNodes());
    as->assemble();

    ObjectArray* oa = as->result();

    ASSERT_TRUE(oa != 0);
    ASSERT_EQ(2, oa->size());

    ASSERT_TRUE(is_simple_fn(oa->at(0)));

    SimpleFunction* sf = cast<SimpleFunction>(oa->at(0));

    //ASSERT_EQ(10, sf->size());

    ASSERT_EQ(2, int_val(sf->tempCount_));

    ASSERT_EQ(123, int_val(sf->at(2)));
    ASSERT_TRUE(is_object(sf->at(3)));
    Object* something = cast<Object>(sf->at(3));
    ASSERT_EQ(2, something->size());
    ASSERT_DOUBLE_EQ(3.14, float_val(something->at(0)));
    ASSERT_IS_CSTRING("baz", something->at(1));
    ASSERT_IS_CSTRING("foo", sf->at(4));
    ASSERT_DOUBLE_EQ(3.14, float_val(sf->at(5)));
    ASSERT_EQ(5, int_val(sf->at(6)));
    ASSERT_TRUE(sf == ptr_val(sf->at(7)));
    ASSERT_FLOAT_EQ(12.34, float_val(sf->at(8)));
    ASSERT_EQ(37, int_val(sf->at(9)));
    ASSERT_TRUE(is_object(sf->at(10)));
    Object* o2 = cast<Object>(sf->at(10));
    ASSERT_EQ(1, o2->size());
    ASSERT_EQ(678, int_val(o2->at(0)));

    ASSERT_TRUE(is_byte_array(sf->bytecodes_));
    ByteArray* ba = cast<ByteArray>(sf->bytecodes_);

    ASSERT_EQ(45, ba->size());
    ASSERT_EQ(Opcode::SEND_VAL_TO_VAL, int_val(ba->at(0)));
    ASSERT_EQ(5, int_val(ba->at(1)));
    ASSERT_EQ(4, int_val(ba->at(2)));
    ASSERT_EQ(Opcode::HALT, int_val(ba->at(3)));
    ASSERT_EQ(Opcode::ARRAY_LENGTH, int_val(ba->at(4)));
    ASSERT_EQ(1, int_val(ba->at(5)));
    ASSERT_EQ(12, int_val(ba->at(6)));
    ASSERT_EQ(Opcode::ARRAY_AT, int_val(ba->at(7)));
    ASSERT_EQ(1, int_val(ba->at(8)));
    ASSERT_EQ(6, int_val(ba->at(9)));
    ASSERT_EQ(3, int_val(ba->at(10)));
    ASSERT_EQ(Opcode::CREATE_OBJECT_ARRAY, int_val(ba->at(11)));
    ASSERT_EQ(4, int_val(ba->at(12)));
    ASSERT_EQ(7, int_val(ba->at(13)));
    ASSERT_EQ(9, int_val(ba->at(14)));
    ASSERT_EQ(0, int_val(ba->at(15)));
    ASSERT_EQ(10, int_val(ba->at(16)));
    ASSERT_EQ(11, int_val(ba->at(17)));
    ASSERT_EQ(Opcode::SEND_VAL_TO_VAL_WCC, int_val(ba->at(18)));
    ASSERT_EQ(11, int_val(ba->at(19)));
    ASSERT_EQ(2, int_val(ba->at(20)));
    ASSERT_EQ(Opcode::ARRAY_AT_PUT, int_val(ba->at(21)));
    ASSERT_EQ(1, int_val(ba->at(22)));
    ASSERT_EQ(2, int_val(ba->at(23)));
    ASSERT_EQ(5, int_val(ba->at(24)));
    ASSERT_EQ(Opcode::CONDITIONAL_ONE, int_val(ba->at(25)));
    ASSERT_EQ(1, int_val(ba->at(26)));
    ASSERT_EQ(Opcode::SEND_VAL_TO_VAL, int_val(ba->at(27)));
    ASSERT_EQ(5, int_val(ba->at(28)));
    ASSERT_EQ(5, int_val(ba->at(29)));
    ASSERT_EQ(Opcode::CONDITIONAL_NOT_ONE, int_val(ba->at(30)));
    ASSERT_EQ(1, int_val(ba->at(31)));
    ASSERT_EQ(Opcode::SEND_VAL_TO_VAL, int_val(ba->at(32)));
    ASSERT_EQ(5, int_val(ba->at(33)));
    ASSERT_EQ(5, int_val(ba->at(34)));
    ASSERT_EQ(Opcode::CONTINUE_WITH, int_val(ba->at(35)));
    ASSERT_EQ(0, int_val(ba->at(36)));
    ASSERT_EQ(Opcode::CONTINUE_WITH_RESULT, int_val(ba->at(37)));
    ASSERT_EQ(0, int_val(ba->at(38)));
    ASSERT_EQ(12, int_val(ba->at(39)));

    ASSERT_FLOAT_EQ(3.14, float_val(oa->at(1)));
    */
}
