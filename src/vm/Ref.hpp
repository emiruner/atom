/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_REF_HPP_INCLUDED
#define ATOM_REF_HPP_INCLUDED

#include "Config.hpp"
#include "Integral.hpp"
#include "ObjectHeader.hpp"
#include "Debug.hpp"
#include "Exceptions.hpp"

#include <cassert>

namespace atom
{

#if defined ATOM_LITTLE_ENDIAN

#if defined ATOM_VM_64BITS

#define ATOM_REF_DATA_BITS 62

typedef atom_int64_t word;
typedef atom_uint64_t uword;

#endif // defined ATOM_VM_64BITS

#if defined ATOM_VM_32BITS

#define ATOM_REF_DATA_BITS 30

typedef atom_int32_t word;
typedef atom_uint32_t uword;

#endif // defined ATOM_VM_32BITS

struct Ref
{
    uword is_int_  : 1;
    uword arr_acc_ : 1;
    word  data_    : ATOM_REF_DATA_BITS;
};

#undef ATOM_REF_DATA_BITS

inline bool operator==(Ref const& lhs, Ref const& rhs)
{
    return lhs.is_int_ == rhs.is_int_ && lhs.data_ == rhs.data_;
}

inline bool operator!=(Ref const& lhs, Ref const& rhs)
{
    return !(lhs == rhs);
}

inline Ref word2ref(word w)
{
    Ref r;
   
    r.is_int_ = 1;
    r.arr_acc_ = 0;
    r.data_ = w;
    
    return r;
}

inline Ref zeroRef()
{
    return word2ref(0);
}

inline word int_val(Ref ref)
{
    return ref.data_;
}

inline bool array_access(Ref ref)
{
    return ref.arr_acc_ == 1;
}

inline Ref set_array_access(Ref ref)
{
    if(ref.is_int_ == 0)
    {
        ref.arr_acc_ = 1;
    }
    
    return ref;
}

inline Ref ptr2ref(void* ptr, bool arrayAccess = false)
{
    Ref r;

    r.is_int_ = 0;
    r.arr_acc_ = arrayAccess ? 1 : 0;
    r.data_ = ((uword) ptr) >> 2;

    return r;
}

inline void* ptr_val(Ref ref)
{
    return (void*) (ref.data_ << 2);
}

inline bool is_int(Ref ref)
{
    return ref.is_int_ == 1;
}

inline Ref clear_array_access(Ref ref)
{    
    if(array_access(ref))
    {
        ref.arr_acc_ = 0;
    }

    return ref;
}

#else
#error Cannot handle big endian machines yet.
#endif // defined ATOM_LITTLE_ENDIAN

/**
 * Casts given ref to specified pointer with less strict type checking than
 * 
 * @warning Assumes that ref is NOT an integer ref.
 */
template <typename T>
inline T* cast(Ref ref)
{
    return (T*) ptr_val(ref);
}

/**
 * @warning Assumes that ref is NOT an integer ref.
 */
inline int obj_type(Ref ref)
{
    return ((ObjectHeader*) ptr_val(ref))->objtype;
}

inline bool is_native_fn(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::NATIVE_FUNCTION;
}

inline bool is_simple_fn(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::SIMPLE_FUNCTION;
}

inline bool is_object(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::OBJECT;
}

inline bool is_object_array(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::OBJECT_ARRAY;
}

inline bool is_byte_array(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::BYTE_ARRAY;
}

inline bool is_float(Ref ref)
{
    return !is_int(ref) && obj_type(ref) == ObjectType::FLOAT;
}

struct Memory;

/**
 * Structure to hold a Ref in C++ code.
 * When the references relocated in memory RefHandle's contents are updated.
 */
struct RefHandle
{
    Ref ref_;
    Memory* mem_;
    
    RefHandle()
    {
        ref_ = zeroRef();
    }
    
    Ref ref() const
    {
        return ref_;
    }
    
    void ref(Ref const& ref);
    
    explicit RefHandle(Ref ref, Memory* mem);
    RefHandle(RefHandle const& other);
    ~RefHandle();
    RefHandle& operator=(RefHandle const& other);
};

} // namespace atom

#endif /* ATOM_REF_HPP_INCLUDED */
