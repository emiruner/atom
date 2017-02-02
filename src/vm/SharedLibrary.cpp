#include "SharedLibrary.hpp"

#include <dlfcn.h>
#include <memory.h>

using namespace atom;

namespace
{

inline bool checkTwoByteArrayInAObjectArray(Ref message)
{
    if(!is_object_array(message))
    {
        return false;
    }

    ObjectArray* oa = cast<ObjectArray>(message);

    if(oa->size() < 2)
    {
        return false;
    }

    return is_byte_array(oa->at(0)) && is_byte_array(oa->at(1));
}

inline void byteArrayInfoBuf(ByteArray* ba, char* buf)
{
    memcpy(buf, ba->data(), ba->size());
    buf[ba->size()] = '\0';
}

} // namespace <unnamed>

namespace atom
{

void fn_loadLibrary(Thread* thread, int resultTmp, Ref message)
{
    if(!is_byte_array(message))
    {
        DEBUG("loadLibrary: message must be byte array\n");
        thread->setResult(resultTmp, word2ref(1L));
        return;
    }

    char buf[400];
    byteArrayInfoBuf(cast<ByteArray>(message), buf);
    void* handle = dlopen(buf, RTLD_LAZY);

    if(handle == 0)
    {
        DEBUG("loadLibrary: cannot load library: " << buf << '\n');
        thread->setResult(resultTmp, word2ref(1L));
        return;
    }

    thread->setResult(resultTmp, thread->memory_->createOpaqueNativeHandle(handle));

    DEBUG("loadLibrary: shared library loaded successfully: " << buf << '\n');
}

void fn_resolveFunction(Thread* thread, int resultTmp, Ref message)
{
    if(!checkTwoByteArrayInAObjectArray(message))
    {
        DEBUG("resolveFunction: message must be an object array containing two byte arrays\n");
        thread->setResult(resultTmp, word2ref(1L));
        return;
    }

    ObjectArray* oa = cast<ObjectArray>(message);
    void* handle = (void*) cast<ByteArray>(oa->at(0))->data();
    char buf[400];
    byteArrayInfoBuf(cast<ByteArray>(oa->at(1)), buf);

    // Clear any previous errors.
    dlerror();

    void* ptr = dlsym(handle, buf);
    char* errStr = dlerror();

    if(errStr != 0)
    {
        DEBUG("resolveSymbol: error occurred while resolving symbol: " << errStr << '\n');
        thread->setResult(resultTmp, word2ref(1L));
        return;
    }
    else
    {
        DEBUG("resolvedSymbol: " << buf << "\n");
    }
    
    if(ptr == 0)
    {
        DEBUG("resolveSymbol: error, symbol\'s value is NULL: " << buf << '\n');
        thread->setResult(resultTmp, word2ref(1L));
        return;
    }

    thread->setResult(resultTmp, thread->memory_->createNativeFunction((NativeFunction::FunT) ptr));
}

} // namespace atom
