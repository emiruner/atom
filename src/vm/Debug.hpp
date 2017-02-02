/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_DEBUG_HPP_INCLUDED
#define ATOM_DEBUG_HPP_INCLUDED

#ifdef ATOM_DEBUG

#include <iostream>
#define DEBUG(x) std::clog << x

#else

#define DEBUG(x)
#define NDEBUG

#endif // ATOM_DEBUG

#endif
