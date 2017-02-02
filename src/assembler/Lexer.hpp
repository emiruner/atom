/**
 * This file is part of Atom project.
 *
 * Copyright (C) 2006 Emir Uner
 */

#ifndef ATOM_ASSEMBLER_LEXER_HPP_INCLUDED
#define ATOM_ASSEMBLER_LEXER_HPP_INCLUDED

#include <stack>

#include "Token.hpp"
#include "CharBuffer.hpp"

namespace atom
{

namespace assembler
{

/**
 * Analyzes the given input stream and return a stream
 * of tokens. Supports one token push back capability.
 */
class Lexer
{
public:
    /**
     * Construct a lexer that uses the given stream as source.
     */
    Lexer(std::istream& in, std::string filename);

    /**
     * Get next token.
     * Returns Eof token when end of file reached and
     * throws std::runtime_error when encounters an error.
     */
    Token getNextToken();

    /**
     * Discards the following token if matches otherwise no change.
     */
    bool discardIfMatches(TokenType::type type);

    /**
     * Return the next token. If an invalid token encountered or EOF
     * std::runtime_error is thrown.
     */
    Token expectAnyToken();

    /**
     * Return the next token. If and error occurs or the next token is not
     * one of the given types throw exception.
     */
    Token expectToken(TokenType::type t);
    Token expectToken(TokenType::type t1, TokenType::type t2);
    Token expectToken(TokenType::type t1, TokenType::type t2, TokenType::type t3);

    /**
     * Raise an exception prepending the filename and line number and
     * appending token string with the associated token to the given message.
     * This method can be used by other object to report an error at a
     * specified position in the token stream.
     */
    void raiseError(std::string message, Token const& offending) const;

    /**
     * Read and discard incoming token. If an error occurs or the next token
     * is not one of the given types throw exception.
     */
    void discardToken(TokenType::type t);

    /**
     * Get current line number.
     */
    inline int getLineNumber() const
    {
        return lineNo_;
    }

    /**
     * Get the name of the file.
     */
    inline std::string const& getFilename() const
    {
        return filename_;
    }

    inline void ungetToken(Token token)
    {
        tokens_.push(token);
    }

    /**
     * Get a reference to the input stream of the lexer.
     */
    inline std::istream const& getStream() const
    {
        return inStream_;
    }

private:
    typedef std::stack<Token> TokenStack;

    std::istream& inStream_;
    CharBuffer in_;
    std::string filename_;
    int lineNo_;
    TokenStack tokens_;
};

} // namespace assembler

} // namespace atom

#endif
