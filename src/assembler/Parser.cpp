/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Parser.hpp"

#include <stdexcept>
#include <string>
#include <sstream>

using namespace atom;
using namespace atom::assembler;

namespace
{

inline std::string printParseError(std::string msg, Token const& token)
{
    std::ostringstream oss;

    oss << msg;
    oss << token;

    return oss.str();
}

VarNode* parseVarBody(Parser& parser, Lexer& lexer)
{
    int index = -1;

    if (lexer.discardIfMatches(TokenType::Colon))
    {
        index = lexer.expectToken(TokenType::IntegerLiteral).asInteger();

        if (index < 0 || index > 255)
        {
            throw std::runtime_error("index must be in range [0, 255]");
        }
    }

    std::string varName = lexer.expectToken(TokenType::Identifier).asString();

    Node* varValue = 0;
    if (lexer.discardIfMatches(TokenType::Equal))
    {
        varValue = parser.parseSymbolicRefOrObjectDef();
    }

    return new VarNode(index, varName, varValue);
}

AlenNode* parseAlen(Parser& parser, Lexer& lexer)
{
    AlenNode* an = new AlenNode();

    an->arr(parser.parseSymbolicRef());
    lexer.discardToken(TokenType::Gt);
    an->result(parser.parseSymbolicRef());

    return an;
}

JumpNode* parseJump(Parser& parser, Lexer& lexer)
{
    JumpNode* n = new JumpNode();
    n->label(parser.parseSymbolicRef());
    return n;
}

SetNode* parseSet(Parser& parser, Lexer& lexer)
{
    SetNode* n = new SetNode();
    
    n->target(parser.parseSymbolicRef());
    n->value(parser.parseSymbolicRefOrObjectDef());
    
    return n;
}

CroaNode* parseCroa(Parser& parser, Lexer& lexer)
{
    CroaNode* n = new CroaNode();

    lexer.discardToken(TokenType::LSquare);
    lexer.ungetToken(Token(TokenType::Comma, -1));

    parser.parseCommaSeparatedNodes(n->elements(), TokenType::RSquare);

    lexer.discardToken(TokenType::Gt);

    n->result(parser.parseSymbolicRef());

    return n;
}

CroNode* parseCro(Parser& parser, Lexer& lexer)
{
    CroNode* n = new CroNode();

    lexer.discardToken(TokenType::LSquare);
    lexer.ungetToken(Token(TokenType::Comma, -1));

    parser.parseCommaSeparatedNodes(n->elements(), TokenType::RSquare);

    lexer.discardToken(TokenType::Gt);

    n->result(parser.parseSymbolicRef());

    return n;
}

AatNode* parseAat(Parser& parser, Lexer& lexer)
{
    AatNode* aat = new AatNode();

    aat->arr(parser.parseSymbolicRef());

    Node* idxNode = parser.parseSymbolicRefOrObjectDef();
    if (!isa<IntegerLiteralNode>(idxNode) && !isa<SymbolicRef>(idxNode))
    {
        throw std::runtime_error("index must be an integer literal or symbolic ref");
    }

    aat->index(idxNode);

    lexer.discardToken(TokenType::Gt);
    aat->result(parser.parseSymbolicRef());

    return aat;
}

AputNode* parseAput(Parser& parser, Lexer& lexer)
{
    AputNode* ap = new AputNode();

    ap->arr(parser.parseSymbolicRef());

    Node* idxNode = parser.parseSymbolicRefOrObjectDef();
    if (!isa<IntegerLiteralNode>(idxNode) && !isa<SymbolicRef>(idxNode))
    {
        throw std::runtime_error("index must be an integer literal or symbolic ref");
    }

    ap->index(idxNode);
    ap->value(parser.parseSymbolicRefOrObjectDef());

    return ap;
}

SendNode* parseSend(Parser& parser, Lexer& lexer)
{
    SendNode* sn = new SendNode();

    sn->message(parser.parseSymbolicRefOrObjectDef());
    lexer.discardToken(TokenType::To);
    sn->target(parser.parseSymbolicRefOrObjectDef());

    if (lexer.discardIfMatches(TokenType::Gt))
    {
        sn->result(parser.parseSymbolicRef());
    }

    return sn;
}

InstallExHandlerNode* parseInstallExHandler(Parser& parser, Lexer& lexer)
{
    InstallExHandlerNode* n = new InstallExHandlerNode();
    n->handler(parser.parseSymbolicRefOrObjectDef());
    return n;
}

RaiseExceptionNode* parseRaiseException(Parser& parser, Lexer& lexer)
{
    RaiseExceptionNode* n = new RaiseExceptionNode();
    n->exception(parser.parseSymbolicRefOrObjectDef());
    return n;
}

ConditionalNode* parseConditional(Parser& parser, Lexer& lexer, bool positive)
{
    ConditionalNode* cn = new ConditionalNode();

    cn->positive(positive);
    cn->cond(parser.parseSymbolicRef());

    return cn;
}

ReturnNode* parseRet(Parser& parser, Lexer& lexer)
{
    ReturnNode* rn = new ReturnNode();
    return rn;
}

ReturnNode* parseRetRes(Parser& parser, Lexer& lexer)
{
    ReturnNode* rn = new ReturnNode();
    rn->result(parser.parseSymbolicRefOrObjectDef());
    return rn;
}

} // namespace <unnamed>

Parser::Parser(Lexer& in)
        : in_(in)
{
}

void Parser::parse()
{
    TopLevelDef* tld;

    while ((tld = parseToplevel()) != 0)
    {
        toplevelNodes_.push_back(tld);
    }
}

TopLevelDef* Parser::parseToplevel()
{
    Token token(in_.getNextToken());
    std::string id;

    if (token.isIdentifier())
    {
        id = token.asString();
    }
    else if (token.isEof())
    {
        return 0;
    }
    else
    {
        in_.ungetToken(token);
    }

    in_.discardToken(TokenType::Colon);

    ObjectDefNode* node = parseAnyObjectDef();
    return new TopLevelDef(id, node);
}

ObjectDefNode* Parser::parseAnyObjectDef()
{
    Token token(in_.getNextToken());

    if (token.isStringLiteral())
    {
        return new StringLiteralNode(token.asString());
    }
    else if (token.isSimplefnRaw())
    {
        return parseSimplefnRawBody();
    }
    else if (token.isSimplefn())
    {
        return parseSimpleFnBody();
    }
    else if (token.isObjarr())
    {
        return parseObjectArray();
    }
    else if (token.isBytearr())
    {
        return parseByteArray();
    }
    else if (token.isObject())
    {
        return parseObjectBody();
    }
    else if (token.isIntegerLiteral())
    {
        return new IntegerLiteralNode(token.asInteger());
    }
    else if (token.isFloatLiteral())
    {
        return new FloatLiteralNode(token.asFloat());
    }

    throw std::runtime_error(printParseError("parseAnyObjectDef(), unexpected token: ", token));
}

ObjectArrayDefNode* Parser::parseObjectArray()
{
    in_.discardToken(TokenType::LCurly);
    in_.ungetToken(Token(TokenType::Comma, -1));

    ObjectArrayDefNode* oadn = new ObjectArrayDefNode();
    parseCommaSeparatedNodes(oadn->elements());
    return oadn;
}

Node* Parser::parseSymbolicRefOrObjectDef()
{
    Token token(in_.getNextToken());

    if (token.isIdentifier())
    {
        return new SymbolicRef(token.asString());
    }
    else
    {
        in_.ungetToken(token);
        return parseAnyObjectDef();
    }
}

SymbolicRef* Parser::parseSymbolicRef()
{
    return new SymbolicRef(in_.expectToken(TokenType::Identifier).asString());
}

SimplefnRawNode* Parser::parseSimplefnRawBody()
{
    in_.discardToken(TokenType::LCurly);

    Node* bytecodes = parseSymbolicRefOrObjectDef();

    if (dynamic_cast<SymbolicRef*>(bytecodes) == 0 && dynamic_cast<BytearrayDefNode*>(bytecodes) == 0)
    {
        throw std::runtime_error("either sybolic ref or a byte array is expected");
    }

    in_.discardToken(TokenType::Comma);

    Node* tempcount = parseSymbolicRefOrObjectDef();

    if (dynamic_cast<SymbolicRef*>(tempcount) == 0 && dynamic_cast<IntegerLiteralNode*>(tempcount) == 0)
    {
        throw std::runtime_error("either sybolic ref or integer is expected");
    }

    SimplefnRawNode* sfnr = new SimplefnRawNode();

    parseCommaSeparatedNodes(sfnr->knownObjects());
    sfnr->bytecodes(bytecodes);
    sfnr->tempcount(tempcount);

    return sfnr;
}

void Parser::parseCommaSeparatedNodes(NodeVec& elements, TokenType::type closingTokenType)
{
    Token token(in_.getNextToken());

    while (token.getType() != closingTokenType && !token.isEof())
    {
        if (!token.isComma())
        {
            throw std::runtime_error("expecting a comma");
        }

        elements.push_back(parseSymbolicRefOrObjectDef());
        token = in_.getNextToken();
    }

    if (token.getType() != closingTokenType)
    {
        throw std::runtime_error("was expecting an closing curly brace");
    }
}

SimpleFnDefNode* Parser::parseSimpleFnBody()
{
    in_.discardToken(TokenType::LCurly);

    Token token(in_.getNextToken());
    SimpleFnDefNode* sfn = new SimpleFnDefNode();

    while (!token.isRCurly() && !token.isEof())
    {
        if (token.isVar())
        {
            sfn->vars().push_back(parseVarBody(*this, in_));
        }
        else if (token.isAlen())
        {
            sfn->instructions().push_back(parseAlen(*this, in_));
        }
        else if (token.isAat())
        {
            sfn->instructions().push_back(parseAat(*this, in_));
        }
        else if (token.isAput())
        {
            sfn->instructions().push_back(parseAput(*this, in_));
        }
        else if (token.isCro())
        {
            sfn->instructions().push_back(parseCro(*this, in_));
        }
        else if (token.isCroa())
        {
            sfn->instructions().push_back(parseCroa(*this, in_));
        }
        else if (token.isSend())
        {
            sfn->instructions().push_back(parseSend(*this, in_));
        }
        else if (token.isIf1() || token.isIfnot1())
        {
            sfn->instructions().push_back(parseConditional(*this, in_, token.isIf1()));
        }
        else if (token.isRet())
        {
            sfn->instructions().push_back(parseRet(*this, in_));
        }
        else if (token.isRetRes())
        {
            sfn->instructions().push_back(parseRetRes(*this, in_));
        }
        else if (token.isHalt())
        {
            sfn->instructions().push_back(new HaltNode());
        }
        else if (token.isIexh())
        {
            sfn->instructions().push_back(parseInstallExHandler(*this, in_));;
        }
        else if (token.isRaise())
        {
            sfn->instructions().push_back(parseRaiseException(*this, in_));;
        }
        else if (token.isJmp())
        {
            sfn->instructions().push_back(parseJump(*this, in_));
        }
        else if (token.isSet())
        {
            sfn->instructions().push_back(parseSet(*this, in_));
        }
        else if (token.isIdentifier() && in_.getNextToken().isColon())
        {           
            LabelNode* ln = new LabelNode();
            ln->name(new SymbolicRef(token.asString()));
            sfn->instructions().push_back(ln);
        }
        else
        {
            throw std::runtime_error(printParseError("parseSimpleFnBody, must not reach here: ", token));
        }

        token = in_.getNextToken();
    }

    return sfn;
}

ObjectObjectDefNode* Parser::parseObjectBody()
{
    in_.discardToken(TokenType::LCurly);

    Node* metaObject = parseSymbolicRefOrObjectDef();

    NodeVec remaining;
    parseCommaSeparatedNodes(remaining);

    return new ObjectObjectDefNode(metaObject, remaining);
}

BytearrayDefNode* Parser::parseByteArray()
{
    in_.discardToken(TokenType::LCurly);
    in_.ungetToken(Token(TokenType::Comma, -1));

    Token token(in_.getNextToken());

    NodeVec nodes;

    while (!token.isRCurly() && !token.isEof())
    {
        if (!token.isComma())
        {
            throw std::runtime_error("expecting a comma");
        }

        Node* element = parseSymbolicRefOrObjectDef();

        if (dynamic_cast<SymbolicRef*>(element) != 0)
        {
        }
        else if (dynamic_cast<IntegerLiteralNode*>(element) != 0)
        {
            IntegerLiteralNode* iln = (IntegerLiteralNode*) element;

            if (iln->literal() < 0 || iln->literal() > 255)
            {
                throw std::runtime_error("byte array element must be in range [0,255]");
            }
        }
        else
        {
            throw std::runtime_error("byte array element must be a symbolic ref or integer");
        }

        nodes.push_back(element);
        token = in_.getNextToken();
    }

    if (!token.isRCurly())
    {
        throw std::runtime_error("was expecting an closing curly brace");
    }

    BytearrayDefNode* badn = new BytearrayDefNode();
    badn->elements(nodes);
    return badn;
}
