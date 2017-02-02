/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_ASSEMBLER_TOKENTYPE_HPP_INCLUDED
#define ATOM_ASSEMBLER_TOKENTYPE_HPP_INCLUDED

namespace atom
{

namespace assembler
{

namespace TokenType
{
    enum type
    {
        Invalid,
        Eof,
        Comma,
        LSquare,
        RSquare,
        Gt,
        StringLiteral,
        IntegerLiteral,
        Identifier,
        Store,
        In,
        To,
        With,
        Aat,
        Alen,
        Aput,
        Croa,
        Cro,
        Send,
        If1,
        Ifnot1,
        Ret,
        RetRes,
        LCurly,
        RCurly,
        Semicolon,
        FloatLiteral,
        SimplefnRaw,
        Simplefn,
        Objarr,
        Object,
        Bytearr,
        Colon,
        Equal,
        Var,
        Halt,
        Iexh,
        Raise,
        Jmp,
        Set,
    };

    inline char const* toStr(type t)
    {
        switch(t)
        {
            case Invalid: return "Invalid";
            case Eof: return "Eof";
            case Comma: return "Comma";
            case LSquare: return "LSquare";
            case RSquare: return "RSquare";
            case Gt: return "Gt";
            case StringLiteral: return "StringLiteral";
            case IntegerLiteral: return "IntegerLiteral";
            case Identifier: return "Identifier";
            case Store: return "Store";
            case In: return "In";
            case To: return "To";
            case With: return "With";
            case Aat: return "Aat";
            case Alen: return "Alen";
            case Aput: return "Aput";
            case Croa: return "Croa";
            case Cro: return "Cro";
            case Send: return "Send";
            case If1: return "If1";
            case Ifnot1: return "Ifnot1";
            case Ret: return "Ret";
            case RetRes: return "RetRes";
            case LCurly: return "LCurly";
            case RCurly: return "RCurly";
            case Semicolon: return "Semicolon";
            case FloatLiteral: return "FloatLiteral";
            case SimplefnRaw: return "SimplefnRaw";
            case Simplefn: return "Simplefn";
            case Objarr: return "Objarr";
            case Object: return "Object";
            case Bytearr: return "Bytearr";
            case Colon: return "Colon";
            case Equal: return "Equal";
            case Var: return "Var";
            case Halt: return "Halt";
            case Iexh: return "Iexh";
            case Raise: return "Raise";
            case Jmp: return "Jmp";
            case Set: return "Set";
            default: return "<INVALID TOKEN TYPE>";
        }
    }
}

}

}

#endif
