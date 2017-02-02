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

#include "TestHelper.hpp"

using namespace atom;
using namespace atom::assembler;

TEST(LexerTest, Simple)
{
    std::ifstream in("../test/simple.z");

    ASSERT_TRUE(in.is_open());

    Lexer lexer(in, std::string("-"));
    Token token(lexer.getNextToken());

    ASSERT_TRUE(token.isIdentifier());
    ASSERT_EQ("msg", token.asString());

    int count = 0;
    while(!token.isEof())
    {
        ++count;
        token = lexer.getNextToken();
    }

    ASSERT_EQ(66, count);
}

TEST(ParserTest, ParseBasic)
{
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

    EXPECT_EQ(8, parser.toplevelNodes().size());

    // 0
    ASSERT_EQ("msg", parser.toplevelNodes()[0]->name());
    ASSERT_EQ("hello, world!", parser.toplevelNodes()[0]->extract<StringLiteralNode>().literal());

    // 1
    ASSERT_EQ("fn", parser.toplevelNodes()[1]->name());

    SimplefnRawNode& sfnr = parser.toplevelNodes()[1]->extract<SimplefnRawNode>();
    ASSERT_SYMREFNAME_EQ("b1", sfnr.bytecodes());
    ASSERT_INTLIT_EQ(5, sfnr.tempcount());
    ASSERT_EQ(2, sfnr.knownObjects().size());
    ASSERT_SYMREFNAME_EQ("foo", sfnr.knownObjects()[0]);
    ASSERT_SYMREFNAME_EQ("count", sfnr.knownObjects()[1]);

    // 2
    ASSERT_EQ("coords", parser.toplevelNodes()[2]->name());
    ObjectObjectDefNode& oadn = parser.toplevelNodes()[2]->extract<ObjectObjectDefNode>();
    ASSERT_EQ(3, oadn.remaining().size());

    ASSERT_SYMREFNAME_EQ("fn", oadn.metaObject());
    ASSERT_FLOATLIT_EQ(56.333, oadn.remaining()[0]);
    ASSERT_SYMREFNAME_EQ("msg", oadn.remaining()[1]);
    ASSERT_STRLIT_EQ("what?", oadn.remaining()[2]);

    // 3
    ASSERT_EQ("foo", parser.toplevelNodes()[3]->name());
    ObjectObjectDefNode& oodn = parser.toplevelNodes()[3]->extract<ObjectObjectDefNode>();
    ASSERT_SYMREFNAME_EQ("fn", oodn.metaObject());
    ASSERT_SYMREFNAME_EQ("coords", oodn.remaining()[0]);

    // 4
    ASSERT_EQ("count", parser.toplevelNodes()[4]->name());
    ASSERT_INTLIT_EQ(45, parser.toplevelNodes()[4]->def());

    // 5
    ASSERT_EQ("pi", parser.toplevelNodes()[5]->name());
    ASSERT_FLOATLIT_EQ(3.141592, parser.toplevelNodes()[5]->def());

    // 6
    ASSERT_EQ("b1", parser.toplevelNodes()[6]->name());
    BytearrayDefNode& badn = parser.toplevelNodes()[6]->extract<BytearrayDefNode>();

    ASSERT_EQ(7, badn.elements().size());
    ASSERT_INTLIT_EQ(12, badn.elements()[0]);
    ASSERT_SYMREFNAME_EQ("count", badn.elements()[1]);
    ASSERT_INTLIT_EQ(34, badn.elements()[2]);
    ASSERT_INTLIT_EQ(78, badn.elements()[3]);
    ASSERT_INTLIT_EQ(0, badn.elements()[4]);
    ASSERT_INTLIT_EQ(0, badn.elements()[5]);
    ASSERT_INTLIT_EQ(90, badn.elements()[6]);

    // 7
    ASSERT_EQ("", parser.toplevelNodes()[7]->name());
    ObjectObjectDefNode& ano = parser.toplevelNodes()[7]->extract<ObjectObjectDefNode>();
    ASSERT_SYMREFNAME_EQ("b1", ano.metaObject());
    ASSERT_INTLIT_EQ(88, ano.remaining()[0]);
}

/*
TEST(ParserTest, ParseSimpleFnDef)
{
    std::ifstream in("../test/fndef.z");

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

    EXPECT_EQ(2, parser.toplevelNodes().size());

    ASSERT_EQ("fn2", parser.toplevelNodes()[0]->name());
    SimpleFnDefNode& sfn = parser.toplevelNodes()[0]->extract<SimpleFnDefNode>();

    ASSERT_EQ(6, sfn.vars().size());

    ASSERT_EQ(0, sfn.vars()[0]->index());
    ASSERT_EQ("cont", sfn.vars()[0]->name());
    ASSERT_TRUE(sfn.vars()[0]->value() == 0);

    ASSERT_EQ(1, sfn.vars()[1]->index());
    ASSERT_EQ("msg", sfn.vars()[1]->name());
    ASSERT_TRUE(sfn.vars()[1]->value() == 0);

    ASSERT_EQ(2, sfn.vars()[2]->index());
    ASSERT_EQ("length", sfn.vars()[2]->name());
    ASSERT_INTLIT_EQ(123, sfn.vars()[2]->value());

    ASSERT_EQ(5, sfn.vars()[3]->index());
    ASSERT_EQ("test", sfn.vars()[3]->name());
    ASSERT_FLOATLIT_EQ(12.34, sfn.vars()[3]->value());

    ASSERT_EQ(-1, sfn.vars()[4]->index());
    ASSERT_EQ("hello", sfn.vars()[4]->name());
    ASSERT_TRUE(sfn.vars()[4]->value() == 0);

    ASSERT_EQ(-1, sfn.vars()[5]->index());
    ASSERT_EQ("foobar", sfn.vars()[5]->name());
    ASSERT_TRUE(sfn.vars()[5]->value() == 0);

    ASSERT_EQ(-1, sfn.vars()[6]->index());
    ASSERT_EQ("something", sfn.vars()[6]->name());

    ObjectObjectDefNode& fnsome = dynamic_cast<ObjectObjectDefNode&>(*(sfn.vars()[6]->value()));
    ASSERT_INTLIT_EQ(666, fnsome.metaObject());
    ASSERT_EQ(1, fnsome.remaining().size());
    ASSERT_INTLIT_EQ(888, fnsome.remaining()[0]);

    ASSERT_EQ(17, sfn.instructions().size());

    AlenNode& alenNode = dynamic_cast<AlenNode&>(*(sfn.instructions()[0]));
    ASSERT_SYMREFNAME_EQ("msg", alenNode.arr());
    ASSERT_SYMREFNAME_EQ("foobar", alenNode.result());

    AatNode& aatNode = dynamic_cast<AatNode&>(*(sfn.instructions()[1]));
    ASSERT_SYMREFNAME_EQ("msg", aatNode.arr());
    ASSERT_INTLIT_EQ(5, aatNode.index());
    ASSERT_SYMREFNAME_EQ("zeroth", aatNode.result());

    CroaNode& croaNode = dynamic_cast<CroaNode&>(*(sfn.instructions()[2]));
    ASSERT_EQ(4, croaNode.elements().size());
    ASSERT_SYMREFNAME_EQ("fn", croaNode.elements()[0]);
    ASSERT_INTLIT_EQ(37, croaNode.elements()[1]);
    ASSERT_SYMREFNAME_EQ("cont", croaNode.elements()[2]);
    ObjectObjectDefNode& croadn = dynamic_cast<ObjectObjectDefNode&>(*(croaNode.elements()[3]));
    ASSERT_EQ(0, croadn.remaining().size());
    ASSERT_INTLIT_EQ(678, croadn.metaObject());
    ASSERT_SYMREFNAME_EQ("test", croaNode.result());

    SendNode& sendNode = dynamic_cast<SendNode&>(*(sfn.instructions()[3]));
    ASSERT_SYMREFNAME_EQ("msg", sendNode.message());
    ASSERT_INTLIT_EQ(872, sendNode.target());
    ASSERT_FALSE(sendNode.wcc());

    SendNode& s1 = dynamic_cast<SendNode&>(*(sfn.instructions()[4]));
    ASSERT_STRLIT_EQ("hello", s1.message());
    ASSERT_SYMREFNAME_EQ("length", s1.target());
    ASSERT_TRUE(s1.result() == 0);
    ASSERT_TRUE(s1.wcc());

    SendNode& s2 = dynamic_cast<SendNode&>(*(sfn.instructions()[5]));
    ASSERT_INTLIT_EQ(3, s2.message());
    ASSERT_INTLIT_EQ(5, s2.target());
    ASSERT_SYMREFNAME_EQ("msg", s2.result());
    ASSERT_TRUE(s2.wcc());

    AputNode& ap = dynamic_cast<AputNode&>(*(sfn.instructions()[6]));
    ASSERT_SYMREFNAME_EQ("msg", ap.arr());
    ASSERT_INTLIT_EQ(2, ap.index());
    ASSERT_STRLIT_EQ("hello", ap.value());

    ConditionalNode& c1 = dynamic_cast<ConditionalNode&>(*(sfn.instructions()[7]));
    ASSERT_SYMREFNAME_EQ("cont", c1.cond());
    ASSERT_TRUE(c1.positive());
    SendNode& c1send = dynamic_cast<SendNode&>(*(sfn.instructions()[8]));
    ASSERT_INTLIT_EQ(6, c1send.message());
    ASSERT_INTLIT_EQ(7, c1send.target());
    ASSERT_FALSE(c1send.wcc());

    ConditionalNode& c2 = dynamic_cast<ConditionalNode&>(*(sfn.instructions()[9]));
    ASSERT_SYMREFNAME_EQ("cont", c2.cond());
    ASSERT_FALSE(c2.positive());
    SendNode& c2send = dynamic_cast<SendNode&>(*(sfn.instructions()[10]));
    ASSERT_INTLIT_EQ(6, c2send.message());
    ASSERT_INTLIT_EQ(7, c2send.target());
    ASSERT_FALSE(c2send.wcc());

    ContNode& cont1 = dynamic_cast<ContNode&>(*(sfn.instructions()[11]));
    ASSERT_SYMREFNAME_EQ("cont", cont1.continuation());
    ASSERT_TRUE(cont1.with() == 0);

    ContNode& cont2 = dynamic_cast<ContNode&>(*(sfn.instructions()[12]));
    ASSERT_SYMREFNAME_EQ("cont", cont2.continuation());
    ASSERT_INTLIT_EQ(789, cont2.with());

    StoreccNode& scc1 = dynamic_cast<StoreccNode&>(*(sfn.instructions()[13]));
    ASSERT_TRUE(scc1.with() == 0);
    ASSERT_SYMREFNAME_EQ("hello", scc1.result());

    StoreccNode& scc2 = dynamic_cast<StoreccNode&>(*(sfn.instructions()[14]));
    ASSERT_SYMREFNAME_EQ("data", scc2.with());
    ASSERT_SYMREFNAME_EQ("foobar", scc2.result());

    ASSERT_TRUE(dynamic_cast<HaltNode*>(sfn.instructions()[15]) != 0);

    ASSERT_EQ("e", parser.toplevelNodes()[1]->name());
    ASSERT_FLOATLIT_EQ(2.71, parser.toplevelNodes()[1]->def());
}
    */
