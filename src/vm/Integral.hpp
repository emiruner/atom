/**
 * This file is part of the Atom VM.
 * 
 * Copyright (C) 2009, Emir Uner
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */
 
#ifndef ATOM_INTEGRAL_HPP_INCLUDED
#define ATOM_INTEGRAL_HPP_INCLUDED

/**
 * Exact width integral type definitions.
 * Hacked by D. Turner to support most systems without requiring boost.
 */

# include <stddef.h>    // for size_t

# if defined(_MSC_VER)

typedef unsigned __int8     atom_uint8_t;
typedef __int8              atom_int8_t;
typedef unsigned __int16    atom_uint16_t;
typedef __int16             atom_int16_t;
typedef unsigned __int32    atom_uint32_t;
typedef __int32             atom_int32_t;
typedef unsigned __int64    atom_uint64_t;
typedef __int64             atom_int64_t;

# elif defined(__hpux) || defined(__FreeBSD__) || defined(__IBMCPP__)
#define ATOM_INTEGRALS_FROM_HEADER
#  include <inttypes.h>
# elif defined(__linux)
#define ATOM_INTEGRALS_FROM_HEADER
#  include <stdint.h>
# elif defined (__CYGWIN__)
#define ATOM_INTEGRALS_FROM_HEADER
#  include <sys/types.h>
# else

#  include <limits.h>

#  if UCHAR_MAX == 0xff
typedef unsigned char   atom_uint8_t;
typedef char            atom_int8_t;
#  else
#   error Cannot find an 8-bit type!
#  endif

#  if USHRT_MAX == 0xffff
typedef unsigned short  atom_uint16_t;
typedef short           atom_int16_t;
#  else
#   error Cannot find a 16-bit type!
#  endif

#  if ULONG_MAX == 0xffffffff
typedef unsigned long   atom_uint32_t;
typedef long            atom_int32_t;
#  elif UINT_MAX == 0xffffffff
typedef unsigned int    atom_uint32_t;
typedef long            atom_int32_t;
#  else
#   error Cannot find a 32-bit type!
#  endif

#  if (defined(ULLONG_MAX) || defined(ULONG_LONG_MAX) || defined(ULONGLONG_MAX))
typedef unsigned long long  atom_uint64_t;
typedef long long           atom_int64_t;
#  elif ULONG_MAX != 0xffffffff
#   if ULONG_MAX == 18446744073709551615
typedef unsigned long       atom_uint64_t;
typedef long                atom_int64_t;
#   else
#    error Cannot find a 64-bit type!
#   endif
#  else
#   error Cannot find a 64-bit type!
#  endif
# endif

#ifdef ATOM_INTEGRALS_FROM_HEADER

typedef int8_t atom_int8_t;
typedef uint8_t atom_uint8_t;
typedef int16_t atom_int16_t;
typedef uint16_t atom_uint16_t;
typedef int32_t atom_int32_t;
typedef uint32_t atom_uint32_t;
typedef int64_t atom_int64_t;
typedef uint64_t atom_uint64_t;

#endif
    
namespace atom
{
    typedef atom_uint8_t byte;
}

#endif
