/**
 * This file is part of the Atom VM.
 *
 * Copyright (C) 2011, Emir Uner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation with the exceptions specified in COPYING file.
 */

#ifndef ATOM_EXCEPTIONS_HPP_INCLUDED
#define ATOM_EXCEPTIONS_HPP_INCLUDED

#include <stdexcept>
#include <sstream>

namespace atom
{
    namespace VmErrorCode
    {
        enum
        {
            ARRAY_OUT_OF_BOUNDS,
            INVALID_BYTEARRAY_ELEMENT,
            OBJECT_TYPE_DOES_NOT_MATCH,
            NOT_ENOUGH_BYTECODES,
            INVALID_BYTECODES_REF,
            INVALID_TEMP_INDEX,
            INVALID_OPCODE,
            NON_INTEGER_ARRAY_INDEX,
            ARRAY_ACCESS_DISABLED,
            NON_INDEXABLE_OBJECT,
            INVALID_SIMPLE_FUNCTION_STRUCTURE,
            RECURSIVE_METAOBJECT,
            VM_IS_ALREADY_HALTED,
            DOUBLE_CONDITIONAL,
            MEMORY_EXHAUSTED,
            CANNOT_CAST_INTREF_TO_POINTER,
            NON_INTEGER_JUMP_OFFSET,
            JUMP_OFFSET_NOT_IN_BOUNDS
        };
    }
    
    class vm_error : public std::runtime_error
    {
        int code_;
        
    public:
        inline explicit vm_error(int code, const std::string& msg)
        : std::runtime_error(msg), code_(code)
        {
        }
        
        inline int code() const
        {
            return code_;
        }
    };
    
    class array_out_of_bounds_error : public vm_error
    {
    public:
        inline array_out_of_bounds_error()
        : vm_error(VmErrorCode::ARRAY_OUT_OF_BOUNDS, "array out of bounds")
        {
        }
    };

    class invalid_bytearray_element_error : public vm_error
    {
    public:
        inline invalid_bytearray_element_error()
        : vm_error(VmErrorCode::INVALID_BYTEARRAY_ELEMENT, "only integers with value in range [0, 255] can be stored in byte array")
        {
        }
    };
    
    class object_type_does_not_match_error : public vm_error
    {
        static std::string createMessage(int expected, int found)
        {
            std::ostringstream oss;
            
            oss << "object type does not match, expected: " << expected << ", found: " << found;
            
            return oss.str();
        }
    public:
        inline object_type_does_not_match_error(int expected, int found)
        : vm_error(VmErrorCode::OBJECT_TYPE_DOES_NOT_MATCH, createMessage(expected, found))
        {
        }                
    };
    
    class not_enough_bytecodes_error : public vm_error
    {
    public:
        inline not_enough_bytecodes_error()
        : vm_error(VmErrorCode::NOT_ENOUGH_BYTECODES, "not enough bytecodes")
        {
        }
    };
    
    class invalid_bytecodes_ref_error : public vm_error
    {
    public:
        inline invalid_bytecodes_ref_error()
        : vm_error(VmErrorCode::INVALID_BYTECODES_REF, "bytecodes must be a byte array")
        {
        }
    };
        
    class invalid_temp_index_error : public vm_error
    {
    public:
        inline invalid_temp_index_error()
        : vm_error(VmErrorCode::INVALID_TEMP_INDEX, "invalid temp index")
        {
        }
    };
        
    class invalid_opcode_error : public vm_error
    {
    public:
        inline invalid_opcode_error()
        : vm_error(VmErrorCode::INVALID_OPCODE, "invalid opcode")
        {
        }
    };
        
    class non_integer_array_index_error : public vm_error
    {
    public:
        inline non_integer_array_index_error()
        : vm_error(VmErrorCode::NON_INTEGER_ARRAY_INDEX, "array index must be an integer")
        {
        }
    };
        
    class array_access_disabled_error : public vm_error
    {
    public:
        inline array_access_disabled_error()
        : vm_error(VmErrorCode::ARRAY_ACCESS_DISABLED, "reference is not enabled for array access")
        {
        }
    };

    class non_indexable_object_error : public vm_error
    {
    public:
        inline non_indexable_object_error()
        : vm_error(VmErrorCode::NON_INDEXABLE_OBJECT, "object is not indexable (neither bytearray nor contains slots)")
        {
        }
    };
        
    class invalid_simple_function_structure_error : public vm_error
    {
    public:
        inline invalid_simple_function_structure_error()
        : vm_error(VmErrorCode::INVALID_SIMPLE_FUNCTION_STRUCTURE, "simple function's structure is not valid")
        {
        }
    };
            
    class recursive_metaobject_error : public vm_error
    {
    public:
        inline recursive_metaobject_error()
        : vm_error(VmErrorCode::RECURSIVE_METAOBJECT, "object cannot be its own metaobject")
        {
        }
    };
            
    class vm_was_halted_error : public vm_error
    {
    public:
        inline vm_was_halted_error()
        : vm_error(VmErrorCode::VM_IS_ALREADY_HALTED, "VM was halted. cannot reanimate")
        {
        }
    };
                
    class double_conditional_error : public vm_error
    {
    public:
        inline double_conditional_error()
        : vm_error(VmErrorCode::DOUBLE_CONDITIONAL, "double conditional opcodes are not allowed")
        {
        }
    };
                   
    class memory_exhausted_error : public vm_error
    {
    public:
        inline memory_exhausted_error()
        : vm_error(VmErrorCode::MEMORY_EXHAUSTED, "memory exhausted")
        {
        }
    };
    
    class cannot_cast_intref_to_pointer_error : public vm_error
    {
    public:
        inline cannot_cast_intref_to_pointer_error()
        : vm_error(VmErrorCode::CANNOT_CAST_INTREF_TO_POINTER, "cannot cast int reference to a pointer")
        {
        }
    };
    
    class non_integer_jump_offset : public vm_error
    {
    public:
        inline non_integer_jump_offset()
        : vm_error(VmErrorCode::NON_INTEGER_JUMP_OFFSET, "jump offset must be an integer")
        {
        }
    };
    
    class jump_offset_not_in_bounds : public vm_error
    {
    public:
        inline jump_offset_not_in_bounds()
        : vm_error(VmErrorCode::JUMP_OFFSET_NOT_IN_BOUNDS, "jump offset must be an integer")
        {
        }
    };
}

#endif
