/**
 * This file is part of the Atom VM.
 * 
 * Copyright (C) 2009, Emir Uner
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <cstring>
#include <iostream>
#include "LineReader.hpp"

using namespace atom;
using namespace atom::debugger;

namespace
{

string_vector commands;

std::string findCommonPrefix(string_vector const& sv)
{
    std::string common;
    int pos = 0;
    
    while(true)
    {
        std::string current = sv.begin()->substr(pos, 1);
        bool allSame = true;
        
        for(string_vector::const_iterator it = sv.begin(); it != sv.end(); ++it)
        {
            if(current != it->substr(pos, 1))
            {
                allSame = false;
                break;
            }           
        }
        
        if(allSame)
        {
            common += current;
        }
        else
        {
            break;
        }
        
        ++pos;
    }

    return common;
}
char** commandCompletion(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;
    string_vector found;
    
    for(string_vector::const_iterator it = commands.begin(); it != commands.end(); ++it)
    {
        std::string cmd = *it;
        
        if(cmd.find(text, 0) == 0)
        {
            found.push_back(cmd);
        }
    }

    if(found.empty()) 
    {
        return 0;
    }
    
    if(found.size() > 1)
    {
        std::cout << "\n";
        
        for(string_vector::const_iterator it = found.begin(); it != found.end(); ++it)
        {
            std::cout << it->c_str() << "\n";
        }
        
        rl_on_new_line();
        
        std::string common = findCommonPrefix(found);
        rl_insert_text(common.substr(strlen(text), std::string::npos).c_str());
        
        return 0;
    }
    
    char** possible = (char**) malloc(sizeof(char*) * (found.size() + 1));
    int idx = 0;    
    for(string_vector::const_iterator it = found.begin(); it != found.end(); ++it)
    {
        std::string str = *it;
        
        possible[idx] = (char*) malloc(str.length() + 1);
        strcpy(possible[idx], str.c_str());
        
        ++idx;
    }
    
    possible[idx] = 0;
        
    return possible;
}

}

namespace atom
{

namespace debugger
{

LineReader::LineReader(std::string prompt, string_vector cmds)
: prompt_(prompt)
{
    std::swap(commands, cmds);
    rl_attempted_completion_function = commandCompletion;
}

} // namespace debugger

} // namespace atom
