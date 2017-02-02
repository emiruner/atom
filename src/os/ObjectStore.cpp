/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "ObjectStore.hpp"

#if defined ATOM_VM_32BITS
#define WORD_BYTE_COUNT 4
#endif

#if defined ATOM_VM_64BITS
#define WORD_BYTE_COUNT 8
#endif


using namespace atom;

namespace atom
{
    ObjectStoreWriter::ObjectStoreWriter(char const* filename)
    {
        out_ = fopen(filename, "wb");

        if(out_ == 0)
        {
            throw std::runtime_error("cannot open object store file for writing");
        }

        flushed_ = false;
    }

    ObjectStoreWriter::~ObjectStoreWriter()
    {
        if(out_ != 0)
        {
            flush();
            fclose(out_);
        }
    }

    void ObjectStoreWriter::addObject(Ref ref)
    {
        objects_.push_back(ref);
    }

    inline uword ref_as_uword(Ref r)
    {
        return *((uword*) ((void*) &r));
    }
    
    uword ObjectStoreWriter::idForObject(Ref ref)
    {
        if(is_int(ref))
        {
            return ref_as_uword(ref);
        }

        uword idx = 0;
        for(RefVec::const_iterator it = objectTable_.begin(); it != objectTable_.end(); ++it, ++idx)
        {
            if(*it == ref)
            {
                return ref_as_uword(ptr2ref((void*) (idx << 2), array_access(ref)));
            }
        }

        throw std::runtime_error("possible bug. cannot find object in object table");
    }

    void ObjectStoreWriter::storeObjectArray(Object* oa)
    {
        for(word i = 0; i < oa->size(); ++i)
        {
            writeItem(idForObject(oa->at(i)));
        }
    }

    void ObjectStoreWriter::storeObjectHeader(ObjectHeader* oh)
    {
        writeItem(*oh);
    }

    void ObjectStoreWriter::storeFloat(FloatObject* f)
    {
        writeItem(f->value_);
    }

    void ObjectStoreWriter::storeByteArray(ByteArray* ba)
    {
        ObjectHeader oh;

        oh.objtype = ObjectType::BYTE_ARRAY;
        oh.size = ba->size();

        storeObjectHeader(&oh);

        writeBytes(ba->data(), ba->size());

        char buf[WORD_BYTE_COUNT];
        *((word *) &buf[0]) = 0;

        if(ba->size() % WORD_BYTE_COUNT != 0)
        {
            writeBytes(buf, WORD_BYTE_COUNT - (ba->size() % WORD_BYTE_COUNT));
        }
    }

    void ObjectStoreWriter::writeObject(Ref ref)
    {
        if(is_int(ref))
        {
            return;
        }

        if(is_byte_array(ref))
        {
            storeByteArray(cast<ByteArray>(ref));
        }
        else if(is_float(ref))
        {
            storeObjectHeader(static_cast<ObjectHeader*>(ptr_val(ref)));
            storeFloat(cast<FloatObject>(ref));
        }
        else
        {
            // We can treat all other objects as Object's
            storeObjectHeader(static_cast<ObjectHeader*>(ptr_val(ref)));
            storeObjectArray(static_cast<Object*>(ptr_val(ref)));
        }
    }

    void ObjectStoreWriter::populateObjectTableWith(Ref ref)
    {
        if(std::find(objectTable_.begin(), objectTable_.end(), ref) != objectTable_.end())
        {
            return;
        }

        if(is_int(ref))
        {
            // Do nothing
        }
        else if(is_byte_array(ref) || is_float(ref))
        {
            objectTable_.push_back(ref);
        }
        else
        {
            objectTable_.push_back(ref);

            // We can cast to Object because physical structure of all objects are same
            // and we are in C++ land.
            Object* oh = static_cast<Object*>(ptr_val(ref));
            int sz = oh->size();

            for(int i = 0; i < sz; ++i)
            {
                populateObjectTableWith(oh->at(i));
            }
        }
    }

    void ObjectStoreWriter::populateObjectTable()
    {
        objectTable_.clear();

        for(RefVec::const_iterator it = objects_.begin(); it != objects_.end(); ++it)
        {
            populateObjectTableWith(*it);
        }
    }

    void ObjectStoreWriter::flush()
    {
        if(flushed_)
        {
            return;
        }
        else
        {
            flushed_ = true;
        }

        populateObjectTable();

        for(RefVec::const_iterator it = objectTable_.begin(); it != objectTable_.end(); ++it)
        {
            writeObject(*it);
        }
    }

    ObjectStoreReader::ObjectStoreReader(char const* filename, Memory* mem)
    {
        in_ = fopen(filename, "rb");

        if(in_ == 0)
        {
            throw std::runtime_error("cannot open object store file for reading");
        }

        mem_ = mem;
    }

    Ref ObjectStoreReader::readRef()
    {
        Ref ref;

        if(!readItem(ref))
        {
            throw std::runtime_error("eof");
        }

        return ref;
    }

    void ObjectStoreReader::readByteArray(ObjectHeader const& oh)
    {
        RefHandle bufRef(mem_->createManagedByteArray(oh.size), mem_);
        byte* buf = cast<ByteArray>(bufRef.ref())->data();

        byte junk[WORD_BYTE_COUNT];

        if(!readBytes(buf, oh.size))
        {
            throw std::runtime_error("eof");
        }

        if((oh.size % WORD_BYTE_COUNT) != 0)
        {
            readBytes(junk, WORD_BYTE_COUNT - (oh.size % WORD_BYTE_COUNT));
        }

        objects_.push_back(bufRef);
    }

    void ObjectStoreReader::readObject(ObjectHeader const& oh)
    {
        if(!ObjectType::isValid(oh.objtype))
        {
            throw std::runtime_error("unknown object type read");
        }

        PtrHandle<Object> oa((Object*) ptr_val(mem_->createObject(oh.size, oh.objtype)), mem_);
        objects_.push_back(RefHandle(ptr2ref(oa.ptr()), mem_));

        for(uword i = 0; i < (oh.size - 1); ++i)
        {
            Ref item = readRef();

            if(is_int(item))
            {
                oa->atPut(i, item);
            }
            else
            {
                bool arracc = array_access(item);
                int objectTableIndex = item.data_;

                fixups_.push_back(FixupRecord(objectTableIndex, i, oa, arracc));
            }
        }
    }

    bool ObjectStoreReader::readFloat()
    {
        double d;

        if(!readItem(d))
        {
            return false;
        }

        objects_.push_back(RefHandle(mem_->createFloat(d), mem_));
        return true;
    }

    bool ObjectStoreReader::readNextObject()
    {
        ObjectHeader oh;

        if(!readItem(oh))
        {
            return false;
        }

        if(oh.objtype == ObjectType::BYTE_ARRAY)
        {
            readByteArray(oh);
        }
        else if(oh.objtype == ObjectType::FLOAT)
        {
            return readFloat();
        }
        else
        {
            readObject(oh);
        }

        return true;
    }

    Object* ObjectStoreReader::readAll()
    {
        while(readNextObject());
        applyFixups();

        PtrHandle<Object> all(mem_->createObject<Object>(objects_.size() + 1), mem_);

        int index = 0;
        for(RefVec::const_iterator it = objects_.begin(); it != objects_.end(); ++it, ++index)
        {
            all->atPut(index, it->ref());
        }

        return all.ptr();
    }

    void ObjectStoreReader::applyFixups()
    {
        for(FixupRecordVec::iterator it = fixups_.begin(); it != fixups_.end(); ++it)
        {
            it->fixUp(objects_);
        }
    }
} // namespace atom

