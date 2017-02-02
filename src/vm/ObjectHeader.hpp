/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_OBJECTHEADER_HPP_INCLUDED
#define ATOM_OBJECTHEADER_HPP_INCLUDED

#include "Integral.hpp"
#include "Config.hpp"

namespace atom
{
#if defined ATOM_VM_64BITS
#define SIZE_BITS 60
#endif

#if defined ATOM_VM_32BITS
#define SIZE_BITS 28
#endif

#if defined ATOM_LITTLE_ENDIAN

    struct ForwardedObjectHeader
    {
        atom_uint64_t mark    : 1;
        atom_uint64_t realPtr : SIZE_BITS + 3;

        inline bool isForwarded() const
        {
            return mark == 1;
        }

        inline void forward(void* newP)
        {
            realPtr = ((atom_int64_t) newP) >> 1;
            mark = 1;
        }

        inline void* getRealPtr()
        {
            return (void*) (realPtr << 1);
        }
    };

    struct ObjectHeader
    {
        atom_uint64_t mark    : 1;
        atom_uint64_t objtype : 3;
        atom_uint64_t size    : SIZE_BITS;

        inline bool isMarked() const
        {
            return mark == 1;
        }

        inline void setMark()
        {
            mark = 1;
        }

        inline void clearMark()
        {
            mark = 0;
        }

        inline void init(int ty, int sz)
        {
            clearMark();
            objtype = ty;
            size = sz;
        }
    };

#endif

#undef SIZE_BITS

    namespace ObjectType
    {
        enum
        {
            // Primitive data objects.
            BYTE_ARRAY       = 0,
            NATIVE_FUNCTION  = 1,

            // Other objects.
            OBJECT           = 2,
            OBJECT_ARRAY     = 3,
            SIMPLE_FUNCTION  = 4,
            FLOAT            = 5,
            INTEGER          = 6
        };

        inline bool isValid(int v)
        {
            return BYTE_ARRAY == v || NATIVE_FUNCTION == v || OBJECT == v || OBJECT_ARRAY == v || SIMPLE_FUNCTION == v || FLOAT == v;
        }

        inline bool containsSlots(int v)
        {
            return v == OBJECT || v == OBJECT_ARRAY || v == SIMPLE_FUNCTION;
        }
    }
}

#endif /* ATOM_OBJECTHEADER_HPP_INCLUDED */
