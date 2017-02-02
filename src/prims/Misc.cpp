#include <cstdlib>
#include <iostream>
#include <string.h>
#include <vm/Thread.hpp>

using namespace atom;

namespace
{

inline bool isArrayLike(Ref o)
{
    return is_object(o) || is_object_array(o);
}

} // namespace <unnamed>

extern "C"
{

void fn_is_object(Thread* thread, int resultTmp, Ref message)
{
    thread->setResult(resultTmp, is_object(message) ? word2ref(1L) : zeroRef());
}

void fn_is_object_array(Thread* thread, int resultTmp, Ref message)
{
    thread->setResult(resultTmp, is_object_array(message) ? word2ref(1L) : zeroRef());
}

void fn_is_byte_array(Thread* thread, int resultTmp, Ref message)
{
    thread->setResult(resultTmp, is_byte_array(message) ? word2ref(1L) : zeroRef());
}

void fn_is_int_object(Thread* thread, int resultTmp, Ref message)
{
    thread->setResult(resultTmp, is_int(message) ? word2ref(1L) : zeroRef());
}

void fn_exitVm(Thread* thread, int resultTmp,  Ref message)
{
    exit(0);
}

void fn_arrayCopy(Thread* thread, int resultTmp, Ref message)
{
    if(!is_object(message))
    {
        std::cerr << "Message must be an object array.\n";
        return;
    }

    Object* oa = cast<Object>(message);

    if(oa->size() != 5)
    {
        std::cerr << "Array must contain at least 5 elements.\n";
        return;
    }

    if(!isArrayLike(oa->at(0)) || !is_int(oa->at(1)) || !isArrayLike(oa->at(2)) || !is_int(oa->at(3)) || !is_int(oa->at(4)))
    {
        return;
    }

    Object* src = cast<Object>(oa->at(0));
    int srcBegin = int_val(oa->at(1));

    Object* target = cast<Object>(oa->at(2));
    int targetBegin = int_val(oa->at(3));

    int len = int_val(oa->at(4));

    for(int i = 0; i < len; ++i)
    {
        target->atPut(targetBegin + i, src->at(srcBegin + i));
    }
}

void fn_cstreq(Thread* thread, int resulttmp, Ref message)
{
    if(resulttmp == -1)
    {
        return;
    }
    
    if(!is_object_array(message) || !array_access(message))
    {
        thread->raiseError(RefHandle(thread->memory_->createByteArrayFromString("message must be an object array with access enabled"), thread->memory_));
        return;
    }
    
    ObjectArray* oa = cast<ObjectArray>(message);
    
    if(oa->size() < 2 || !is_byte_array(oa->at(0)) || !is_byte_array(oa->at(1)))
    {
        thread->raiseError(RefHandle(thread->memory_->createByteArrayFromString("message must contain two byte array's"), thread->memory_));
        return;
    }
    
    ByteArray* first = cast<ByteArray>(oa->at(0));
    ByteArray* second = cast<ByteArray>(oa->at(1));
    
    if(first->size() != second->size())
    {
        thread->setResult(resulttmp, zeroRef());
    }
    else
    {
        thread->setResult(resulttmp, memcmp(first->data(), second->data(), first->size()) == 0 ? word2ref(1) : zeroRef());
    }
}

} // extern "C"
