/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_ASSEMBLER_CHARBUFFER_HPP_INCLUDED
#define ATOM_ASSEMBLER_CHARBUFFER_HPP_INCLUDED

#include <istream>
#include <cassert>

namespace atom
{

namespace assembler
{


/**
 * A character buffer with one character putback capability.
 */
class CharBuffer
{
public:
    /**
     * A character buffer associated with the given stream.
     */
    inline CharBuffer(std::istream& in)
    : in_(in), lastChar_(std::char_traits<char>::eof()), ungot_(false)
    {
    }

    /**
     * Return next character.
     * In case of eof return std::char_traits<char>::eof().
     */
    inline int get()
    {
        if(ungot_)
        {
            ungot_ = false;
            return lastChar_;
        }

        return lastChar_ = in_.get();
    }

    /**
     * Return the last character read back.
     * This function may be called at most once without
     * calling get consequtively.
     */
    inline void unget()
    {
        assert(ungot_ == false);
        ungot_ = true;
    }

    /**
     * Return true if last operation succeeded.
     */
    inline bool good() const
    {
        return in_.good();
    }

private:
    std::istream& in_;
    int lastChar_;
    bool ungot_;
};

} // namespace assembler

} // namespace atom

#endif // ATOM_ASSEMBLER_CHARBUFFER_HPP_INCLUDED
