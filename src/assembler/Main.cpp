/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstdlib>

#include <vm/Thread.hpp>
#include <vm/Opcode.hpp>
#include <os/ObjectStore.hpp>

#include "Assembler.hpp"

using namespace atom;
using namespace atom::assembler;

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " infile outfile\n";
        return 1;
    }

    std::ifstream in(argv[1]);

    if(!in.is_open())
    {
        std::cerr << "Cannot open input file: '" << argv[1] << "'\n";
        return 1;
    }

    Lexer lexer(in, argv[1]);
    Parser parser(lexer);

    try
    {
        parser.parse();
    }
    catch(std::runtime_error const& e)
    {
        std::cerr << "An error occured: " << e.what() << "\n";
    }

    Memory mem(10240);
    Assembler* as = new Assembler(mem, parser.toplevelNodes());
    as->assemble();

    ObjectArray* oa = as->result();

    ObjectStoreWriter osw(argv[2]);

    for(unsigned int i = 0; i < oa->size(); ++i)
    {
        osw.addObject(oa->at(i));
    }

    osw.flush();
}
