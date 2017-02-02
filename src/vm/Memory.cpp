/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2009, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include "Memory.hpp"
#include "Thread.hpp"
#include "SharedLibrary.hpp"
#include "Exceptions.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>

using namespace atom;

namespace
{
    void dumpRef(Ref r)
    {
        std::cerr << "ref: {is_int=" << r.is_int_ << ", arr_acc=" << r.arr_acc_;
        
        if(r.is_int_)
        {
            std::cerr << ", data=" << r.data_;
        }
        else 
        {
            std::cerr << ", ptrEquiv=" << ptr_val(r);
        }
        
        std::cerr << "}";
    }
    
} // unnamed

namespace atom
{
    Ref MemSpace::copy(Ref p, RefHandleList& externalRefs, PtrHandleList& externalPtrs)
    {
        if(is_int(p))
        {
            return p;
        }
        
        int type = obj_type(p);
        
        ForwardedObjectHeader* foh = cast<ForwardedObjectHeader>(p);
        bool forwarded = foh->isForwarded();
        
        if(!forwarded)
        {            
            ObjectBase* pptr = cast<ObjectBase>(p);
            Ref* newP = allocSame(p);
            
            foh->forward(newP);
            
            if(type == ObjectType::BYTE_ARRAY || type == ObjectType::NATIVE_FUNCTION)
            {
                ((PrimDataObject*) newP)->copy((PrimDataObject*) pptr);
            }
            else if(type == ObjectType::FLOAT)
            {
                ((FloatObject*) newP)->copy((FloatObject*) pptr);
            }
            else
            {
                const int n = ((ObjectBase*) newP)->header_.size;
                
                for(int i = 1; i < n; ++i)
                {
                    newP[i] = copy(((Ref*) pptr)[i], externalRefs, externalPtrs);
                }
            }
        }
        
        Ref newRef = ptr2ref(foh->getRealPtr(), array_access(p));
        
        if(!forwarded)
        {
            for(RefHandleList::iterator it = externalRefs.begin(); it != externalRefs.end(); ++it)
            {
                RefHandle* rh = *it;
                
                if(rh->ref() == p)
                {
                    rh->ref(newRef);
                }
            }
            
            for(PtrHandleList::iterator it = externalPtrs.begin(); it != externalPtrs.end(); ++it)
            {
                PtrHandleBase* ph = *it;
                
                if(ph->ptr_ == ptr_val(p))
                {
                    ph->ptr(ptr_val(newRef));
                }
            }
        }        
        
        return newRef;
    }
    
    void MemSpace::reset()
    {
        Ref* current = start_;

        while(current != free_)
        {
            ObjectHeader* oh = (ObjectHeader*) current;
            
            if(!oh->isMarked() && oh->objtype == ObjectType::BYTE_ARRAY)
            {
                ByteArray* ba = (ByteArray*) oh;
                
                if(ba->managed_)
                {
                    delete[] ba->data();
                }
            }
            
            if(oh->isMarked())
            {
                current += ((ObjectHeader*) ((ForwardedObjectHeader*) oh)->getRealPtr())->size;
            }
            else
            {
                current += oh->size;
            }
        }

        free_ = start_;
    }

    RefHandle::RefHandle(Ref ref, Memory* mem)
    : ref_(ref), mem_(mem)
    {
        if(!is_int(ref_))
        {
            mem_->registerRefHandle(this);
        }
    }
        
    RefHandle::~RefHandle()
    {
        if(!is_int(ref_))
        {
            mem_->unregisterRefHandle(this);
        }
    }        
       
    void RefHandle::ref(const atom::Ref& ref)
    {
        if(is_int(ref_) && !is_int(ref))
        {
            mem_->registerRefHandle(this);
        }
        
        ref_ = ref;
    }

    RefHandle::RefHandle(RefHandle const& other)
    : ref_(other.ref_), mem_(other.mem_)
    {
        if(!is_int(ref_))
        {
            mem_->registerRefHandle(this);            
        }
    }
        
    RefHandle& RefHandle::operator=(RefHandle const& other)
    {
        if(!is_int(ref_))
        {
            mem_->unregisterRefHandle(this);
        }
        
        ref_ = other.ref();
        mem_ = other.mem_;
        
        if(!is_int(ref_))
        {
            mem_->registerRefHandle(this);            
        }
        
        return *this;
    }
    
    PtrHandleBase::PtrHandleBase(void* ptr, Memory* mem)
    : ptr_(ptr), mem_(mem)
    {
        if(ptr_ != 0)
        {
            mem_->registerPtrHandle(this);
        }
    }
    
    PtrHandleBase::~PtrHandleBase()
    {
        if(ptr_ != 0)
        {
            mem_->unregisterPtrHandle(this);
        }
    }        
    
    void PtrHandleBase::ptr(void* ptr)
    {
        if(ptr_ != 0 && ptr == 0)        
        {
            mem_->unregisterPtrHandle(this);
        }
        
        if(ptr_ == 0 && ptr != 0)
        {
            mem_->registerPtrHandle(this);
        }
        
        ptr_ = ptr;
    }

    PtrHandleBase::PtrHandleBase(PtrHandleBase const& other)
    : ptr_(other.ptr_), mem_(other.mem_)
    {
        if(ptr_ != 0)
        {
            mem_->registerPtrHandle(this);            
        }
    }
    
    PtrHandleBase& PtrHandleBase::operator=(PtrHandleBase const& other)
    {
        if(ptr_ == 0 && other.ptr_ != 0)
        {
            mem_->registerPtrHandle(this);
        }
        
        ptr_ = other.ptr_;
        return *this;
    }
    
    Ref Memory::createPrim(int type, void* ptr, uword size, bool managed)
    {
        PrimDataObject* pr = (PrimDataObject*) alloc(sizeof(PrimDataObject) / sizeof(Ref));

        pr->header_.init(type, sizeof(PrimDataObject) / sizeof(Ref));

        pr->managed_ = managed;
        pr->size_ = size;
        pr->data_ = ptr;

        return ptr2ref(pr);
    }

    Ref* Memory::alloc(int slotCount)
    {
        if(!toSpace_->canAllocate(slotCount))
        {
            DEBUG("Memory full, running garbage collection.\n");
            
            flipSpaces();
            
            if(!toSpace_->canAllocate(slotCount))
            {
                throw memory_exhausted_error();
            }            
        }

        return toSpace_->alloc(slotCount);
    }

    void Memory::flipSpaces()
    {
        MemSpace* tmp = fromSpace_;
        fromSpace_ = toSpace_;
        toSpace_ = tmp;

        std::vector<Ref> roots;
                
        for(RefHandleList::iterator it = refHandles_.begin(); it != refHandles_.end(); ++it)
        {
            roots.push_back((*it)->ref());
        }
        
        for(PtrHandleList::iterator it = ptrHandles_.begin(); it != ptrHandles_.end(); ++it)
        {
            void* ptr = (*it)->ptr_;
            
            if(ptr != 0)
            {
                roots.push_back(ptr2ref(ptr));
            }
        }
        
        for(std::vector<Ref>::const_iterator it = roots.begin(); it != roots.end(); ++it)
        {
            toSpace_->copy(*it, refHandles_, ptrHandles_);
        }
        
        fromSpace_->reset();
    }

    Ref Memory::createObject(uword size, int type)
    {
        Ref* result = alloc(size);

        ((ObjectHeader*) result)->init(type, size);
        return ptr2ref(result);
    }

    Ref Memory::createFloat(double d)
    {
        FloatObject* result = createObject<FloatObject>();

        result->value_ = d;
        return ptr2ref(result);
    }

    Ref Memory::createStartupMessage(Ref printStringFn)
    {
        PtrHandle<ObjectArray> message(createObjectArray(5), this);

        message->atPut(0, createOpaqueNativeHandle(this));
        message->atPut(1, createNativeFunction(&memoryControllerFn));
        message->atPut(2, printStringFn);
        message->atPut(3, createNativeFunction(&fn_loadLibrary));
        message->atPut(4, createNativeFunction(&fn_resolveFunction));

        return ptr2ref(message.ptr(), true);
    }

    Ref Memory::createByteArrayFromString(char const* str)
    {
        Ref baRef = createManagedByteArray(strlen(str));
        ByteArray* ba = cast<ByteArray>(baRef);
        
        memcpy(ba->data(), str, strlen(str));
        
        return baRef;
    }

    void Memory::memoryControllerFn(Thread* thread, int resultTmp, Ref message)
    {
        if(!is_object_array(message) && !array_access(message))
        {
            std::cerr << "Message must be an object array.\n";
            return;
        }

        ObjectArray* oa = cast<ObjectArray>(message);

        if(oa->size() != 3)
        {
            std::cerr << "Message must contain 3 elements.\n";
        }

        Memory* m = (Memory*) cast<ByteArray>(oa->at(0))->data_;
        word kind = int_val(oa->at(1));
        Ref mo = oa->at(2);

        switch(kind)
        {
        case ObjectType::OBJECT_ARRAY:
            m->objectArrayMo_ = RefHandle(mo, m);
            break;

        case ObjectType::INTEGER:
            m->integerMo_ = RefHandle(mo, m);
            break;

        case ObjectType::BYTE_ARRAY:
            m->byteArrayMo_ = RefHandle(mo, m);
            break;

        case ObjectType::FLOAT:
            m->floatMo_ = RefHandle(mo, m);
            break;

        default:
            std::cerr << "Unknown object type\n";
        }
    }
} // namespace atom

