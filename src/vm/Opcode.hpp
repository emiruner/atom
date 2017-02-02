/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_OPCODE_HPP_INCLUDED
#define ATOM_OPCODE_HPP_INCLUDED

#include "Exceptions.hpp"

namespace atom
{
    namespace Opcode
    {
        enum
        {
            SEND_VAL_TO_VAL           = 0,  // send t to t             format: opcode, msg, target
            SEND_VAL_TO_VAL_WRES      = 1,  // send t to t > t         format: opcode, msg, target, resulttmp
            CONDITIONAL_ONE           = 2,  // if1 t                   format: opcode, condtmp
            CONDITIONAL_NOT_ONE       = 3,  // ifnot1 t                format: opcode, condtmp
            RETURN                    = 4,  // ret                     format: opcode
            RETURN_RESULT             = 5,  // retres t                format: opcode, resulttmp
            ARRAY_AT                  = 6,  // aat t t > t             format: opcode, arraytmp, indextmp, resulttmp
            ARRAY_AT_PUT              = 7,  // aput t t t              format: opcode, arraytmp, indextmp, valuetmp
            ARRAY_LENGTH              = 8,  // alen t > t              format: opcode, arraytmp, resulttmp
            CREATE_OBJECT             = 9,  // cro ([t]) > t           format: opcode, elemcount, elem0, elem1, ..., elemn, resulttmp
            CREATE_OBJECT_ARRAY       = 10, // croa ([t]) > t          format: opcode, elemcount, elem0, elem1, ..., elemn, resulttmp
            HALT                      = 11, // halt                    format: opcode
            INSTALL_EXCEPTION_HANDLER = 12, // iexh t                  format: opcode, exhandlertmp
            RAISE_EXCEPTION           = 13, // raise t                 format: opcode, exceptionobject
            JUMP                      = 14, // jmp t                   format: opcode, newiptmp
            SET_LOCAL                 = 15, // set t t                 format: opcode, targettmp, srctmp
            MAX_OPCODE                = 15, // -- marker --
        };

        inline bool isValid(int opcode)
        {
            return opcode <= MAX_OPCODE;
        }

        inline bool isVariableLength(int opcode)
        {
            return opcode == CREATE_OBJECT_ARRAY || opcode == CREATE_OBJECT;
        }

        inline bool isConditional(int opcode)
        {
            return opcode == CONDITIONAL_ONE || opcode == CONDITIONAL_NOT_ONE;
        }
        
        inline bool isReturn(int opcode)
        {
            return opcode == RETURN || opcode == RETURN_RESULT;
        }
        
        /*
         * @pre isValid(opcode) == true
         */
        inline int instructionSize(int byte0, int byte1)
        {
            switch(byte0)
            {
                case SEND_VAL_TO_VAL             : return 3;
                case SEND_VAL_TO_VAL_WRES        : return 4;
                case CONDITIONAL_ONE             : return 2;
                case CONDITIONAL_NOT_ONE         : return 2;
                case RETURN                      : return 1;
                case RETURN_RESULT               : return 2;
                case ARRAY_AT                    : return 4;
                case ARRAY_AT_PUT                : return 4;
                case ARRAY_LENGTH                : return 3;
                case CREATE_OBJECT               : return 3 + byte1;
                case CREATE_OBJECT_ARRAY         : return 3 + byte1;
                case HALT                        : return 1;
                case INSTALL_EXCEPTION_HANDLER   : return 2;
                case RAISE_EXCEPTION             : return 2;
                case JUMP                        : return 2;
                case SET_LOCAL                   : return 3;
            }

            throw invalid_opcode_error();
        }
    }
} // namespace atom

#endif /* ATOM_OPCODE_HPP_INCLUDED */
