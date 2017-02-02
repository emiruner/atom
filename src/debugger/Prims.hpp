#ifndef ATOM_DEBUGGER_PRIMS_HPP_INCLUDED
#define ATOM_DEBUGGER_PRIMS_HPP_INCLUDED

#include <vm/Thread.hpp>

extern "C"
{

void fn_printCString(atom::Thread*, int, atom::Ref);
void fn_println(atom::Thread*, int, atom::Ref);
void fn_readInt(atom::Thread*, int, atom::Ref);
void fn_addInt(atom::Thread*, int, atom::Ref);
void fn_eqInt(atom::Thread*, int, atom::Ref);
void fn_modInt(atom::Thread*, int, atom::Ref);
void fn_intLessThan(atom::Thread*, int, atom::Ref);
void fn_intGreaterThan(atom::Thread*, int, atom::Ref);
void fn_subInt(atom::Thread*, int, atom::Ref);
void fn_multInt(atom::Thread*, int, atom::Ref);
void fn_divInt(atom::Thread*, int, atom::Ref);
void fn_addOne(atom::Thread*, int, atom::Ref);
void fn_is_object(atom::Thread*, int, atom::Ref);
void fn_is_byte_array(atom::Thread*, int, atom::Ref);
void fn_is_int_object(atom::Thread*, int, atom::Ref);
void fn_exitVm(atom::Thread*, int, atom::Ref);
void fn_arrayCopy(atom::Thread*, int, atom::Ref);

} // extern "C"

#endif /* ATOM_DEBUGGER_PRIMS_HPP_INCLUDED */
