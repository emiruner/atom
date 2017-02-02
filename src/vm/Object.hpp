/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_OBJECT_HPP_INCLUDED
#define ATOM_OBJECT_HPP_INCLUDED

#include "Ref.hpp"
#include "ObjectHeader.hpp"
#include "Debug.hpp"
#include "Exceptions.hpp"

namespace atom
{
    struct ObjectBase
    {
        ObjectHeader header_;
    };

    struct PrimDataObject : public ObjectBase
    {
#if defined ATOM_VM_64BITS
        uword size_ : 63;
#endif

#if defined ATOM_VM_32BITS
        uword size_ : 31;
#endif

        uword managed_ : 1;
        void* data_;

        inline word size() const
        {
            return size_;
        }

        inline void copy(PrimDataObject* other)
        {
            size_ = other->size_;
            managed_ = other->managed_;
            data_ = other->data_;
        }
    };

    template <typename T>
    inline void checkArrayIndex(T* t, int idx)
    {
        if(idx < 0 || idx >= t->size())
        {
            throw array_out_of_bounds_error();
        }
    }
    
    template <int TYPE>
    struct RefArrayObjectBase : public ObjectBase
    {
        static const int type = TYPE;

        inline word size() const
        {
            return this->header_.size - 1;
        }

        inline void atPut(int idx, Ref ref)
        {
            *(refAt(idx)) = ref;
        }

        inline Ref at(int idx)
        {
            return *(refAt(idx));
        }

    private:
        inline Ref* refAt(int idx)
        {
            checkArrayIndex(this, idx);
            return ((Ref*) this) + sizeof(ObjectHeader) / sizeof(Ref) + idx;
        }
    };

    struct ObjectArray : public RefArrayObjectBase<ObjectType::OBJECT_ARRAY>
    {
        // Ref ref0_;
        // Ref ref1_;
        // ...
        // Ref refn_;
    };

    struct Object : public RefArrayObjectBase<ObjectType::OBJECT>
    {
        Ref metaObject_;
        // Ref ref1_;
        // Ref ref2_;
        // ...
        // Ref refn_;
    };

    struct SimpleFunction : public RefArrayObjectBase<ObjectType::SIMPLE_FUNCTION>
    {
        Ref bytecodes_;
        Ref tempCount_;
        // There can be any number of known object slots.
        // Ref knownObjects_[??];
    };

    struct FloatObject : public ObjectBase
    {
        static const int type = ObjectType::FLOAT;
        double value_;

        inline void copy(FloatObject* other)
        {
            value_ = other->value_;
        }
    };

    /**
     * @warning Assumes ref is a float.
     */
    inline double float_val(Ref ref)
    {
        return cast<FloatObject>(ref)->value_;
    }

    struct ByteArray : public PrimDataObject
    {
        static const int type = ObjectType::BYTE_ARRAY;

        inline byte* data() const
        {
            return (byte*) data_;
        }

        inline Ref at(int idx)
        {
            checkArrayIndex(this, idx);
            return word2ref(data()[idx]);
        }

        inline void atPut(int idx, Ref byteVal)
        {
            checkArrayIndex(this, idx);

            if(!is_int(byteVal))
            {
                throw invalid_bytearray_element_error();
            }

            data()[idx] = int_val(byteVal);
        }
    };

    class Thread;

    /**
     * The function pointed by the funaddr_ must have the signature:
     *
     * Ref fun(Ref, Ref)
     */
    struct NativeFunction : public PrimDataObject
    {
        static const int type = ObjectType::NATIVE_FUNCTION;

        typedef void (*FunT)(Thread*, int resultTmp, Ref);

        inline void call(Thread* thread, int resultTmp, Ref message) const
        {
            ((FunT) data_)(thread, resultTmp, message);
        }
    };

    struct CallContext : public RefArrayObjectBase<ObjectType::OBJECT_ARRAY>
    {
        Ref parent_;
        Ref exceptionHandler_;
        Ref exceptionObject_;
        Ref ip_;
        Ref bytecodes_;
        Ref temps_;
        Ref resultTmp_;
        
        inline CallContext* initNew(RefHandle const& parent, RefHandle const& exceptionHandler, RefHandle const& exceptionObject)
        {
            parent_ = parent.ref();
            exceptionHandler_ = exceptionHandler.ref();
            exceptionObject_ = exceptionObject.ref();
            
            return this;
        }
        
        inline CallContext* initBytecodesTemps(Ref bytecodes, ObjectArray* temps, word ip = 0)
        {
            if(!is_byte_array(bytecodes))
            {
                throw invalid_bytecodes_ref_error();
            }
            
            bytecodes_ = bytecodes;
            temps_ = ptr2ref(temps);
            ip_ = word2ref(ip);
            
            return this;
        }
        
        inline CallContext* parent()
        {
            return cast<CallContext>(parent_);
        }
        
        inline ByteArray* bytecodes()
        {
            return cast<ByteArray>(bytecodes_);
        }
        
        inline ObjectArray* temps()
        {
            return cast<ObjectArray>(temps_);
        }
        
        inline word ip()
        {
            return int_val(ip_);
        }
        
        inline void ip(word w)
        {
            ip_ = word2ref(w);
        }
        
        inline int validateTemp(int index)
        {
            if(index < 0 || index >= temps()->size())
            {
                throw invalid_temp_index_error();
            }
            
            return index;
        }
        
        inline void inc_ip()
        {
            ip(ip() + 1);
        }

        inline void ensureEnoughBytecodes(int count)
        {
            if(!hasMoreBytecodes(count))
            {
                throw not_enough_bytecodes_error();
            }
        }
        
        inline bool hasMoreBytecodes(int count)
        {
            return (ip() + count) <= bytecodes()->size();
        }

        byte nextbytecode()
        {            
            ensureEnoughBytecodes(1);

            byte result = int_val(bytecodes()->at(ip()));
            inc_ip();

            return result;
        }

        inline byte peekbytecode(int n = 0)
        {
            ensureEnoughBytecodes(n);
            return int_val(bytecodes()->at(ip() + n));
        }

        void skipBytecodes(int n)
        {
            ensureEnoughBytecodes(n);
            ip(ip() + n);
        }

        inline Ref nextTempFromBytecode()
        {
            return getTemp(validateTemp(nextbytecode()));
        }
       
        inline Ref getTemp(int idx)
        {
            return temps()->at(idx);
        }

        inline void setTemp(int idx, Ref ref)
        {
            return temps()->atPut(idx, ref);
        }
        
        inline word resultTmp()
        {
            return int_val(resultTmp_);
        }
    };
}

#endif
