#include <sstream>
#include <iostream>
#include <stdexcept>

#include <vm/Thread.hpp>
#include <os/ObjectStore.hpp>

using namespace atom;

template <typename T>
bool readFromStr(char const* input, T& t)
{
    std::istringstream iss(input);

    iss >> t;
    return !iss.fail() && iss.eof();
}

void printCString(Thread* thread, int resultTmp, Ref message)
{
    if(is_byte_array(message))
    {
        ByteArray* ba = cast<ByteArray>(message);
        char const* str = (char*) ba->data();

        for(int i = 0; i < ba->size() && *str != '\0'; ++i, ++str)
        {
            std::cout << *str;
        }

        std::cout << '\n';
    }
    else
    {
        std::cout << "!! error: message sent to me must be a byte array representing a c-string\n";
    }
}

void proceed(char const* osFileName, int objectIndex)
{
    Memory mem(1048576);
    Thread thread(&mem);
    mem.setThread(&thread);

    ObjectStoreReader osr(osFileName, &mem);
    PtrHandle<Object> objects(osr.readAll(), &mem);

    thread.prepareInitialSend(RefHandle(objects->at(objectIndex), &mem), RefHandle(mem.createStartupMessage(mem.createNativeFunction(&printCString)), &mem));
    thread.execute();
    
    std::cerr << "Number of message sends executed: " << thread.sendCount_ << "\n";
    std::cerr << "Number of bytecodes executed: " << thread.bytecodeCount_ << "\n";
}

int main(int argc, char* argv[])
{
    try
    {
        if(argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " object_store_file runnable_object_index\n";
            return 1;
        }

        char const* osFileName = argv[1];
        int objectIndex;

        if(!readFromStr(argv[2], objectIndex))
        {
            std::cerr << "Given object index is not a valid positive integer.\n";
            return 1;
        }

        proceed(osFileName, objectIndex);
    }
    catch(std::exception const& e)
    {
        std::cerr << "An error occured: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
