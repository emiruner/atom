/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_ASSEMBLER_TOKEN_HPP_INCLUDED
#define ATOM_ASSEMBLER_TOKEN_HPP_INCLUDED

#include <string>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <ostream>

#include "vm/Debug.hpp"
#include "TokenType.hpp"

namespace atom
{

namespace assembler
{

class Token
{
    static inline bool stringToken(TokenType::type type)
    {
        return type == TokenType::StringLiteral || type == TokenType::Identifier;
    }

public:
    static inline Token invalidToken()
    {
        return Token(TokenType::Invalid, -1);
    }

    /**
     * Used with tokens which has no data other than type.
     * lineNo specifies the line number this token encountered
     * in the input stream.
     */
    Token(TokenType::type type, int lineNo)
    : type_(type), lineNo_(lineNo)
    {
        assert(!stringToken(type));
    }

    /**
     * Used for tokens containing a string value and a type.
     */
    Token(TokenType::type type, int lineNo, char const* value)
    : type_(type), lineNo_(lineNo)
    {
#ifdef ATOM_DEBUG
        if(!stringToken(type))
        {
            DEBUG("must be a string token, type = " << type << ", lineNo = " << lineNo << ", value = " << value);
        }

        assert(stringToken(type));
#endif

        strVal_ = value;
    }

    /**
     * Used for IntegerLiteral token.
     */
    Token(TokenType::type type, int lineNo, int value)
    : type_(type), lineNo_(lineNo)
    {
        assert(type == TokenType::IntegerLiteral);
        intVal_ = value;

        std::ostringstream oss;
        oss << value;
        strVal_ = oss.str();
    }

    /**
     * Used for FloatLiteral token.
     */
    Token(TokenType::type type, int lineNo, double value)
    : type_(type), lineNo_(lineNo)
    {
        assert(type == TokenType::FloatLiteral);
        floatVal_ = value;

        std::ostringstream oss;
        oss << value;
        strVal_ = oss.str();
    }

    /**
     * Get string value of the token.
     */
    std::string const& asString() const
    {
        return strVal_;
    }

    /**
     * Get integer value of the token.
     * The token type must be IntegerLiteral
     */
    int asInteger() const
    {
        assert(type_ == TokenType::IntegerLiteral);
        return intVal_;
    }

    double asFloat() const
    {
        assert(type_ == TokenType::FloatLiteral);
        return floatVal_;
    }

    inline int getLineNo() const
    {
        return lineNo_;
    }

    inline TokenType::type getType() const
    {
        return type_;
    }

#include "Token.gen.hpp"

private:
    TokenType::type type_;
    std::string strVal_;
    int intVal_;
    double floatVal_;
    int lineNo_;
};

inline bool operator==(Token const& lhs, Token const& rhs)
{
    return lhs.asString() == rhs.asString();
}

inline std::ostream& operator<<(std::ostream& os, Token const& token)
{
    return os << "Token at line: " << token.getLineNo() << ", type: " << TokenType::toStr(token.getType()) << ", value: " << token.asString() << '\n';
}

} // namespace assembler

} // namespace atom

#endif
