#include <vm/Thread.hpp>

#include <stdio.h>
#include <math.h>

using namespace atom;
using namespace std;

namespace
{

bool checkTwoIntArray(Ref message, char const* diff)
{
    if(!is_object_array(message))
    {
        std::cerr << diff << ": Message must be an object array.\n";
        return false;
    }

    ObjectArray* oa = cast<ObjectArray>(message);

    if(oa->size() < 2)
    {
        std::cerr << diff << ": Array must contain at least two elements.\n";
        return false;
    }
   
    if(!is_int(oa->at(0)) || !is_int(oa->at(1)))
    {
        if(!is_int(oa->at(0)))
        {
            std::cerr << diff << ": First element is not integer. ";
        }
        else
        {
            std::cerr << "first element:[" << int_val(oa->at(0)) << "] ";
        }

        if(!is_int(oa->at(1)))
        {
            std::cerr << diff << ": Second element is not integer. ";
        }
        else
        {
            std::cerr << "second element:[" << int_val(oa->at(1)) << "] ";
        }

        std::cerr << diff << ": Both elements must be integers.\n";
        return false;
    }

    return true;
}

} // namespace <unnamed>

extern "C"
{

void fn_addInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "addInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) + int_val(oa->at(1))));
    }
}

void fn_eqInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "eqInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, int_val(oa->at(0)) == int_val(oa->at(1)) ? word2ref(1L) : zeroRef());
    }
}

void fn_modInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "modInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) % int_val(oa->at(1))));
    }
}

void fn_intLessThan(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "lessThan"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) < int_val(oa->at(1)) ? 1 : 0));
    }
}

void fn_intGreaterThan(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "greaterThan"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) > int_val(oa->at(1)) ? 1 : 0));
    }
}

void fn_subInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "subInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) - int_val(oa->at(1))));
    }
}

void fn_multInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "multInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) * int_val(oa->at(1))));
    }
}

void fn_divInt(Thread* thread, int resultTmp, Ref message)
{
    if(checkTwoIntArray(message, "divInt"))
    {
        ObjectArray* oa = cast<ObjectArray>(message);
        thread->setResult(resultTmp, word2ref(int_val(oa->at(0)) / int_val(oa->at(1))));
    }
}

void fn_addOne(Thread* thread, int resultTmp, Ref message)
{
    if(!is_int(message))
    {
        std::cerr << "Message object must be an integer.\n";
    }

    thread->setResult(resultTmp, word2ref(int_val(message) + 1));
}

void fn_readFloat(Thread* thread, int resultTmp, Ref message)
{
    double val;
    scanf("%lf", &val);

    thread->setResult(resultTmp, thread->memory_->createFloat(val));
}

void fn_sqrt(Thread* thread, int resultTmp, Ref message)
{
    if(!is_float(message))
    {
        std::cerr << "Message object must be a float.\n";
    }
    else
    {
        thread->setResult(resultTmp, thread->memory_->createFloat(sqrt(float_val(message))));
    }
}

} // extern "C"
