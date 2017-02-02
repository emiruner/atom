/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_VM_CONFIG_HPP_INCLUDED
#define ATOM_VM_CONFIG_HPP_INCLUDED

#define ATOM_VM_64BITS
//#define ATOM_VM_32BITS

#define ATOM_LITTLE_ENDIAN
//#define ATOM_BIG_ENDIAN

#if !defined ATOM_BIG_ENDIAN && !defined ATOM_LITTLE_ENDIAN
#error Do not know the endianness.
#endif

#if !defined ATOM_VM_64BITS && !defined ATOM_VM_32BITS
#error Do not know the word size of this machine.
#endif

#define ATOM_DEBUG

#endif
