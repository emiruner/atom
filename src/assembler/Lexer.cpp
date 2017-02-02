/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Lexer.hpp"

#include <ctype.h>

#include <map>
#include <cassert>
#include <sstream>
#include <stdexcept>

using namespace atom;
using namespace atom::assembler;

namespace
{
    typedef std::map<std::string, TokenType::type> StringTokenTypeMap;

    /**
     * If returns true the given character is a character
     * that ends any token when encountered.
     */
    inline bool separatorChar(int ch)
    {
        return isspace(ch) ||
            ch == ',' ||
            ch == '[' ||
            ch == ']' ||
            ch == '{' ||
            ch == '}' ||
            ch == '>' ||
            ch == ';' ||
            ch == ':' ||
            ch == '=' ||
            ch == '\'' || // string literal beginning
            ch == '/' || // comment beginning
            ch == std::char_traits<char>::eof();
    }

    /**
     * Reads an invalid token until finding a separator character.
     */
    inline std::string consumeInvalid(CharBuffer& in)
    {
        std::string result;
        int ch;

        while((ch = in.get()) != std::char_traits<char>::eof())
        {
            if(separatorChar(ch))
            {
                in.unget();
                break;
            }

            result += ch;
        }

        return result;
    }

    inline bool consumeNumber(CharBuffer& in, std::string& result, std::string& invalid)
    {
        std::string strnum;
        int ch;
        bool dotRead = false;
        bool minusRead = false;
        
        while((ch = in.get()) != std::char_traits<char>::eof())
        {
            if(ch == '-')
            {
                if(minusRead)
                {
                    return false;
                }
                
                minusRead = true;
                strnum += '-';
            }
            else if(isdigit(ch))
            {
                strnum += ch;
            }
            else if(ch == '.')
            {
                if(dotRead)
                {
                    return false;
                }

                dotRead = true;
                strnum += '.';
            }
            else if(separatorChar(ch))
            {
                in.unget();
                break;
            }
            else
            {
                in.unget();
                invalid = strnum + consumeInvalid(in);
                return false;
            }
        }

        result = strnum;
        return true;
    }

    /**
     * Reads a string literal whose opening quote is read.
     * Returns false if an error occurs before string literal completes.
     */
    bool readStringLiteral(std::istream& in, int& lineNo, std::string& result)
    {
        int ch;

        while(true)
        {
            ch = in.get();
            if(in.fail() || in.eof())
            {
                return false;
            }

            if(ch == '\n')
            {
                ++lineNo;
            }
            else if(ch == '"')
            {
                break;
            }
            else
            {
                result += ch;
            }
        }

        return true;
    }

    /**
     * Reads a comment whose first character '/' is read. It can handle
     * both single and multi line comments. If an error occurs return false
     * and the invalid input in result otherwise returns true and
     * result is empty.
     */
    bool readComment(std::istream& in, int& lineNo, std::string& result)
    {
        int ch;

        ch = in.get();
        if(!in.fail())
        {
            if(ch == '/')
            {
                while((ch = in.get()) != std::char_traits<char>::eof())
                {
                    if(ch == '\n')
                    {
                        ++lineNo;
                        break;
                    }
                }
                return true;
            }
            else if(ch == '*')
            {
                result += ch;

                while((ch = in.get()) != std::char_traits<char>::eof())
                {
                    result += ch;

                    if(ch == '*')
                    {
                        ch = in.get();
                        if(!in.fail() && ch != std::char_traits<char>::eof())
                        {
                            result += ch;
                        }

                        if(in.fail())
                        {
                            return false;
                        }
                        else if(ch == '/')
                        {
                            result = "";
                            return true;
                        }
                        else if(ch == '\n')
                        {
                            ++lineNo;
                        }
                    }
                    else if(ch == '\n')
                    {
                        ++lineNo;
                    }
                }
                return false;
            }
            else
            {
                result = "";
                result += ch;
                return false;
            }
        }

        result = "";
        result += ch;
        return false;
    }

    /**
     * Tries to read an identifier whose first letter is read.
     */
    bool consumeIdentifierOrKeyword(CharBuffer& in, std::string& result)
    {
        int ch;
        result = "";

        while((ch = in.get()) != std::char_traits<char>::eof())
        {
            if(isdigit(ch) || isalpha(ch))
            {
                result += ch;
            }
            else if(separatorChar(ch))
            {
                in.unget();
                return true;
            }
            else
            {
                break;
            }
        }

        if(ch == std::char_traits<char>::eof())
        {
            return true;
        }

        return false;
    }

    /**
     * Token from identifier of keyword string.
     */
    Token tokenFromIdentifier(std::string id, int lineNo)
    {
        StringTokenTypeMap keywords;

        keywords["!store"] = TokenType::Store;
        keywords["!aat"] = TokenType::Aat;
        keywords["!alen"] = TokenType::Alen;
        keywords["!aput"] = TokenType::Aput;
        keywords["!send"] = TokenType::Send;
        keywords["!if1"] = TokenType::If1;
        keywords["!ifnot1"] = TokenType::Ifnot1;
        keywords["!ret"] = TokenType::Ret;
        keywords["!retres"] = TokenType::RetRes;
        keywords["!in"] = TokenType::In;
        keywords["!to"] = TokenType::To;
        keywords["!with"] = TokenType::With;
        keywords["!cro"] = TokenType::Cro;
        keywords["!croa"] = TokenType::Croa;
        keywords["!simplefnraw"] = TokenType::SimplefnRaw;
        keywords["!simplefn"] = TokenType::Simplefn;
        keywords["!objarr"] = TokenType::Objarr;
        keywords["!object"] = TokenType::Object;
        keywords["!bytearr"] = TokenType::Bytearr;
        keywords["!var"] = TokenType::Var;
        keywords["!halt"] = TokenType::Halt;
        keywords["!iexh"] = TokenType::Iexh;
        keywords["!raise"] = TokenType::Raise;
        keywords["!jmp"] = TokenType::Jmp;
        keywords["!set"] = TokenType::Set;

        if(keywords.find(id) != keywords.end())
        {
            return Token(keywords[id], lineNo);
        }
        else
        {
            return Token(TokenType::Identifier, lineNo, id.c_str());
        }
    }
} // namespace <unnamed>

Lexer::Lexer(std::istream& in, std::string filename)
: inStream_(in), in_(in), filename_(filename), lineNo_(1)
{
}

Token Lexer::expectAnyToken()
{
    Token result(getNextToken());

    if(result.getType() == TokenType::Invalid || result.getType() == TokenType::Eof)
    {
        raiseError("a token is needed", result);
    }

    return result;
}

Token Lexer::expectToken(TokenType::type t)
{
    Token result(getNextToken());

    if(result.getType() != t)
    {
        raiseError("unexpected token", result);
    }

    return result;
}

Token Lexer::expectToken(TokenType::type t1, TokenType::type t2)
{
    Token result(getNextToken());

    if(result.getType() != t1 && result.getType() != t2)
    {
        raiseError("unexpected token", result);
    }

    return result;
}

Token Lexer::expectToken(TokenType::type t1, TokenType::type t2, TokenType::type t3)
{
    Token result(getNextToken());

    if(result.getType() != t1 && result.getType() != t2 && result.getType() != t3)
    {
        raiseError("unexpected token", result);
    }

    return result;
}

void Lexer::discardToken(TokenType::type t)
{
    Token token(getNextToken());

    if(token.getType() != t)
    {
        raiseError("unexpected token", token);
    }
}

void Lexer::raiseError(std::string message, Token const& offending) const
{
    std::ostringstream oss;

    oss << getFilename() << ':' << offending.getLineNo()
        << ": error: " << message << ": " << offending.asString();

    throw std::runtime_error(oss.str());
}

bool Lexer::discardIfMatches(TokenType::type type)
{
    Token token(getNextToken());

    if(token.getType() == type)
    {
        return true;
    }
    else
    {
        ungetToken(token);
        return false;
    }
}

Token Lexer::getNextToken()
{
    if(tokens_.size() > 0)
    {
        Token front(tokens_.top());
        tokens_.pop();
        return front;
    }

    Token token(Token::invalidToken());

    int ch = 3;

    while((ch = in_.get()) != std::char_traits<char>::eof())
    {
        switch(ch)
        {

        case '\n':
            ++lineNo_;
            break;

        case '\r':
        case '\t':
        case ' ':
            break;

        case ',':
            return Token(TokenType::Comma, lineNo_);

        case '[':
            return Token(TokenType::LSquare, lineNo_);

        case ']':
            return Token(TokenType::RSquare, lineNo_);

        case '{':
            return Token(TokenType::LCurly, lineNo_);

        case '}':
            return Token(TokenType::RCurly, lineNo_);

        case '>':
            return Token(TokenType::Gt, lineNo_);

        case ';':
            return Token(TokenType::Semicolon, lineNo_);

        case ':':
            return Token(TokenType::Colon, lineNo_);

        case '=':
            return Token(TokenType::Equal, lineNo_);

        case '\"':
        {
            std::string sl;

            if(readStringLiteral(inStream_, lineNo_, sl))
            {
                return Token(TokenType::StringLiteral, lineNo_, sl.c_str());
            }
            else
            {
                return Token(TokenType::Invalid, lineNo_, ("\"" + sl).c_str());
            }
        }

        case '/':
        {
            std::string sl;

            if(readComment(inStream_, lineNo_, sl))
            {
                continue;
            }
            else
            {
                return Token(TokenType::Invalid, lineNo_, ("/" + sl).c_str());
            }
        }

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        {
            std::string offending;
            std::string str;

            in_.unget();

            if(consumeNumber(in_, str, offending))
            {
                std::istringstream iss(str);

                if(str.find('.') == std::string::npos)
                {
                    int n;
                    iss >> n;
                    return Token(TokenType::IntegerLiteral, lineNo_, n);
                }
                else
                {
                    double n;
                    iss >> n;
                    return Token(TokenType::FloatLiteral, lineNo_, n);
                }
            }
            else
            {
                return Token(TokenType::Invalid, lineNo_, offending.c_str());
            }
        }

        default:
        {
            std::string result;

            if(consumeIdentifierOrKeyword(in_, result))
            {
                return tokenFromIdentifier((char)ch + result, lineNo_);
            }

            // If we are here an unrecognized input is present.
            return Token(TokenType::Invalid, lineNo_, ((char)ch + result + consumeInvalid(in_)).c_str());
        }
        }
    }

    return Token(TokenType::Eof, lineNo_);
}
