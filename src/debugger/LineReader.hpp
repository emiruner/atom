/**
 * This file is part of the Atom VM.
 * 
 * Copyright (C) 2009, Emir Uner
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_DEBUGGER_LINE_READER_HPP_INCLUDED
#define ATOM_DEBUGGER_LINE_READER_HPP_INCLUDED

#include <stdlib.h>

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>
#include <vector>

namespace atom
{

namespace debugger {

typedef std::vector<std::string> string_vector;

struct LineReader
{   
    static const int MAX_LINE_LENGTH = 1024;
    
    const std::string prompt_;
    
    LineReader(std::string prompt, string_vector commands);
    
    std::string readLine()
    {
        while(true)
        {
            char* lineRead = readline(prompt_.c_str());
            
            if(lineRead == 0)
            {
                return std::string();
            }
            
            if(lineRead[0] != '\0')
            {
                add_history(lineRead);
                return std::string((char const*) lineRead);
            }

            free(lineRead);
        }
    }
};


} // namespace debugger

} // namespace atom

#endif // ATOM_DEBUGGER_LINE_READER_HPP_INCLUDED
