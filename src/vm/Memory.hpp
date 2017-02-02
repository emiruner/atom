/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_MEMORY_HPP_INCLUDED
#define ATOM_MEMORY_HPP_INCLUDED

#include "Ref.hpp"
#include "ObjectHeader.hpp"
#include "Object.hpp"

#include <list>
#include <vector>
#include <algorithm>
#include <iostream>

namespace atom
{
    struct Thread;
    struct RefHandle;
    struct PtrHandleBase;
    struct Memory;
    
    typedef std::list<RefHandle*> RefHandleList;
    typedef std::list<PtrHandleBase*> PtrHandleList;
    
    struct MemSpace
    {
        Ref* start_;
        Ref* free_;
        Ref* end_;
        
        MemSpace(word size)
        {
            start_ = free_ = new Ref[size];
            end_ = start_ + size;
        }
    
        ~MemSpace()
        {
            reset();
            delete[] start_;
        }
        
        /**
         * Returns the space to empty status. Frees garbage-collected byte arrays.
         */
        void reset();
        
        inline bool contains(Ref r)
        {
            if(is_int(r))
            {
                return false;
            }
            
            Ref* rp = (Ref*) ptr_val(r);
            return rp >= start_ && rp < end_;            
        }
        
        inline bool canAllocate(int slotCount) const
        {
            return (free_ + slotCount) < end_;
        }
        
        /**
         * Allocates and zero initializes given number of slots. Does not check whether
         * there is enough memory.
         * 
         * @pre canAllocate(sz) == true
         */
        inline Ref* alloc(int sz)
        {
            Ref* result = free_;

            for(int i = 0; i < sz; ++i)
            {
                *free_ = zeroRef();
                ++free_;
            }
    
            return result;
        }
        
        /**
         * Allocates a new object with the same header contents with the given object.
         */
        inline Ref* allocSame(Ref orig)
        {
            ObjectBase* origBase = cast<ObjectBase>(orig);
            ObjectBase* newBase = (ObjectBase*) alloc(origBase->header_.size);
            
            newBase->header_ = origBase->header_;
            
            return (Ref*) newBase;
        }
        
        Ref copy(Ref r, RefHandleList& externalRefs, PtrHandleList& externalPtrs);
        
        inline int size() const
        {
            return end_ - start_;
        }
        
        inline int freeSize() const
        {
            return free_ - start_;
        }
    };
    
    struct PtrHandleBase
    {
        void* ptr_;
        Memory* mem_;
        
        void ptr(void* ptr);
        
        explicit PtrHandleBase(void* ptr, Memory* mem);      
        PtrHandleBase(PtrHandleBase const& other);
        ~PtrHandleBase();
        PtrHandleBase& operator=(PtrHandleBase const& other);
    };
    
    /**
     * Structure to hold a ptr to a Memory heap objects.
     * When the corresponding object is relocated in memory the pointer is updated.
     */
    template <typename T>
    struct PtrHandle : public PtrHandleBase    
    {
        explicit PtrHandle(T* ptr, Memory* mem)
        : PtrHandleBase(ptr, mem)
        {
        }
        
        T* operator->()
        {
            return (T*) ptr_;
        }
        
        T* ptr()
        {
            return (T*) ptr_;
        }
    };

    struct Memory
    {
        MemSpace* toSpace_;
        MemSpace* fromSpace_;
        RefHandleList refHandles_;
        PtrHandleList ptrHandles_;
        
        Thread* thread_;
        
        // Metaobjects of known object types.
        RefHandle objectArrayMo_;
        RefHandle integerMo_;
        RefHandle byteArrayMo_;
        RefHandle floatMo_;

        Memory(word size)
        : toSpace_(new MemSpace(size)), fromSpace_(new MemSpace(size)), thread_(0)
        {
        }

        ~Memory()
        {
            delete toSpace_;
            delete fromSpace_;
        }
        
        template <typename T>
        T* ptrcopy(T* t)
        {
            return cast<T>(toSpace_->copy(ptr2ref(t), refHandles_, ptrHandles_));
        }

        inline Ref findMetaObject(Ref target)
        {
            if(is_int(target))
            {
                return integerMo_.ref();
            }

            switch(obj_type(target))
            {
                case ObjectType::OBJECT: return cast<Object>(target)->metaObject_;
                case ObjectType::OBJECT_ARRAY: return objectArrayMo_.ref();
                case ObjectType::BYTE_ARRAY: return byteArrayMo_.ref();
                case ObjectType::FLOAT: return floatMo_.ref();
            }

            // Return object itself as metaobject to signal the error.
            return target;
        }

        Ref* alloc(int slotCount);
        void flipSpaces();
        
        Ref createPrim(int type, void* ptr, uword size, bool managed);
        Ref createFloat(double d);
        Ref createObject(uword size, int type);

        template <typename T>
        inline T* createObject()
        {
            return createObject<T>(sizeof(T) / sizeof(Ref));
        }

        template <typename T>
        inline T* createObject(int size)
        {
            return cast<T>(createObject(size, T::type));
        }

        inline Ref createManagedPrim(int type, void* ptr, int size)
        {
            return createPrim(type, ptr, size, true);
        }

        inline Ref createNativeFunction(NativeFunction::FunT ptr)
        {
            return createUnmanagedPrim(ObjectType::NATIVE_FUNCTION, (void*) ptr, 0);
        }

        inline Ref createUnmanagedPrim(int type, void* ptr, int size)
        {
            return createPrim(type, ptr, size, false);
        }

        inline ObjectArray* createObjectArray(int size)
        {
            return createObject<ObjectArray>(sizeof(ObjectHeader) / sizeof(Ref) + size);
        }

        inline ObjectArray* createObjectArray(RefHandle item0, RefHandle item1)
        {
            ObjectArray* oa = createObjectArray(2);
            
            oa->atPut(0, item0.ref());
            oa->atPut(1, item1.ref());
            
            return oa;
        }
    
        inline ObjectArray* createObjectArray(RefHandle item0, RefHandle item1, RefHandle item2)
        {
            ObjectArray* oa = createObjectArray(3);
            
            oa->atPut(0, item0.ref());
            oa->atPut(1, item1.ref());
            oa->atPut(2, item2.ref());
            
            return oa;
        }
    
        inline Ref createManagedByteArray(word size)
        {
            byte* b = new byte[size];
            return createManagedPrim(ObjectType::BYTE_ARRAY, b, size);
        }

        inline Ref createUnmanagedByteArray(byte* data, word size)
        {
            return createUnmanagedPrim(ObjectType::BYTE_ARRAY, data, size);
        }

        inline Ref createOpaqueNativeHandle(void* ptr)
        {
            return createUnmanagedPrim(ObjectType::BYTE_ARRAY, ptr, 0);
        }

        Ref createByteArrayFromString(char const* str);

        /**
         * Creates an object array with contents compatible with simple startup protocol.
         *
         * @printStringFn the print string function that may be specified
         */
        Ref createStartupMessage(Ref printStringFn);

        /**
         * Used to specify implicit meta objects for Object Array's, ByteArray's Integer's, and
         * Float's. This native function accepts an array of 3 elements.
         * [memoryRef, index, newMoRef]
         *
         * index is interpreted to be compatible with ObjectType enumeration.
         */
        static void memoryControllerFn(Thread* thread, int resultTmp, Ref message);

        inline void setThread(Thread* thread)
        {
            thread_ = thread;
        }
        
        void registerRefHandle(RefHandle* refHandle)
        {
            refHandles_.push_back(refHandle);
        }
        
        void unregisterRefHandle(RefHandle* refHandle)
        {
            refHandles_.remove(refHandle);
        }
        
        void registerPtrHandle(PtrHandleBase* ptrHandle)
        {
            ptrHandles_.push_back(ptrHandle);
        }
        
        void unregisterPtrHandle(PtrHandleBase* ptrHandle)
        {
            ptrHandles_.remove(ptrHandle);
        }
    };    
}

#endif /* ATOM_MEMORY_HPP_INCLUDED */
