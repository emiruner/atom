/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2010, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_OBJECT_STORE_HPP_INCLUDED
#define ATOM_OBJECT_STORE_HPP_INCLUDED

#include <stdio.h>

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include <vm/Object.hpp>
#include <vm/Memory.hpp>

namespace atom
{
    class ObjectStoreReader
    {
        typedef std::vector<RefHandle> RefVec;

        struct FixupRecord
        {
            uword objectTableIndex;
            uword arrayIndex;
            PtrHandle<Object> ptr;
            bool arracc;

            inline FixupRecord(uword pobjectTableIndex, uword parrayIndex, PtrHandle<Object> const& pptr, bool parracc)
            : objectTableIndex(pobjectTableIndex), arrayIndex(parrayIndex), ptr(pptr), arracc(parracc)
            {
            }

            void fixUp(RefVec const& objects)
            {
                if(objectTableIndex >= objects.size())
                {
                    throw std::runtime_error("Invalid object table offset.");
                }

                if(arracc)
                {
                    ptr->atPut(arrayIndex, set_array_access(objects[objectTableIndex].ref()));
                }
                else
                {
                    ptr->atPut(arrayIndex, objects[objectTableIndex].ref());
                }
            }
        };

        typedef std::vector<FixupRecord> FixupRecordVec;

        FILE* in_;
        Memory* mem_;
        RefVec objects_;
        FixupRecordVec fixups_;

        template <typename T>
        inline bool readItem(T& t)
        {
            return readBytes(&t, sizeof(T));
        }

        inline bool readBytes(void* buf, int size)
        {
            if(fread(buf, size, 1, in_) == 0)
            {
                if(feof(in_))
                {
                    return false;
                }

                throw std::runtime_error("cannot read from file");
            }

            return true;
        }

        bool readNextObject();
        Ref readRef();
        void applyFixups();
        void readByteArray(ObjectHeader const& oh);
        void readObject(ObjectHeader const& oh);
        bool readFloat();

    public:
        ObjectStoreReader(char const* filename, Memory* mem);
        Object* readAll();
    };

    class ObjectStoreWriter
    {
        typedef std::vector<Ref> RefVec;

        FILE* out_;
        RefVec objects_;
        RefVec objectTable_;
        bool flushed_;

        void writeObject(Ref ref);
        void storeObjectArray(Object* oa);
        void storeByteArray(ByteArray* ba);
        void storeObjectHeader(ObjectHeader* oh);
        void storeFloat(FloatObject* fo);

        uword idForObject(Ref ref);
        void populateObjectTable();
        void populateObjectTableWith(Ref ref);

        template <typename T>
        inline void writeItem(T const& t)
        {
            writeBytes(&t, sizeof(T));
        }

        inline void writeBytes(void const* buf, int size)
        {
            if(fwrite(buf, size, 1, out_) == 0)
            {
                throw std::runtime_error("cannot write to file");
            }
        }

    public:
        ObjectStoreWriter(char const* filename);
        ~ObjectStoreWriter();

        void addObject(Ref ref);
        void flush();
    };

} // namespace atom

#endif /* ATOM_OBJECT_STORE_HPP_INCLUDED */
