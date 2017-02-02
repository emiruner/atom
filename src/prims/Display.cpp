#include <vm/Thread.hpp>

#include <cstdio>
#include <unistd.h>

using namespace atom;
using namespace std;

extern "C"
{

void fn_printCString(Thread* thread, int resultTmp, Ref message)
{
    if(is_byte_array(message))
    {
        ByteArray* ba = cast<ByteArray>(message);
        
        for(int i = 0; i < ba->size(); ++i)
        {
            std::cout << (char) ba->data()[i];
        }
    }
    else
    {
        std::cout << "!! error: message sent to me must be a byte array representing a c-string\n";
    }
}

void fn_print(Thread* thread, int resultTmp, Ref message)
{
    if(is_int(message))
    {
        std::cout << int_val(message);
    }
    else if(is_float(message))
    {
        printf("%lf", float_val(message));
    }
    else if(is_byte_array(message))
    {
        fn_printCString(thread, resultTmp, message);
    }
    else
    {
        std::cout << "don\'t know how to print this";
    }
}

void fn_println(Thread* thread, int resultTmp, Ref message)
{
    fn_print(thread, resultTmp, message);
    std::cout << '\n';
}

void fn_readInt(Thread* thread, int resultTmp, Ref message)
{
    word x;
    std::cin >> x;
    Ref result;

    if(std::cin.fail())
    {
        result = ptr2ref(thread->memory_->createObjectArray(0));
    }
    else
    {
        result = word2ref(x);
    }

    thread->setResult(resultTmp, result);
}

void fn_readByte(Thread* thread, int resultTmp, Ref message)
{
    if(is_int(message))
    {
        int fd = int_val(message);
        int buf = 0;

        ssize_t r = read(fd, &buf, 1);
        if(r <= 0)
        {
            thread->setResult(resultTmp, word2ref(-1));
        }
        else
        {
            thread->setResult(resultTmp, word2ref(buf));
        }
    }
}

} // extern "C"
