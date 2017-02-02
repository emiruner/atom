#ifndef ATOM_SHARED_LIBRARY_HPP_INCLUDED
#define ATOM_SHARED_LIBRARY_HPP_INCLUDED

#include "Thread.hpp"

namespace atom
{

/**
 * Loads shared library with the given name. The message must be a 0 terminated byte array containing
 * the name of the shared library to load.
 */
void fn_loadLibrary(atom::Thread* thread, int resultTmp, atom::Ref message);

/**
 * Resolves a symbol and returns it as a NativeFunction. Assumes that the symbol corresponds to a "C"
 * function with the required signature of a NativeFunction.
 *
 * @message must be an object array of the format: [library handle returned from fn_loadLibrary, function name byte array]
 */
void fn_resolveFunction(atom::Thread* thread, int resultTmp, atom::Ref message);

}

#endif /* ATOM_SHARED_LIBRARY_HPP_INCLUDED */
