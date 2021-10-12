#include <iostream>
#include "argumentparser.h"

int main(int argc, char** argv)
{
    ArgumentParser* args;
    try
    {
        args = new ArgumentParser(argc, argv);
    }
    catch (const std::invalid_argument& exception)
    {
        std::cerr << exception.what() << std::endl;
        return 1;
    }
    std::cout << "Read mode:\t" << args->GetReadMode() << std::endl;
    std::cout << "Destination:\t" << args->GetDestinationPath() << std::endl;
    std::cout << "Is dest set?\t" << !(args->GetDestinationPath().empty()) << std::endl;
    std::cout << "Mode:\t\t" << args->GetMode() << std::endl;
    std::cout << "Timeout:\t" << args->GetTimeout() << std::endl;
    std::cout << "Size:\t\t" << args->GetSize() << std::endl;
    std::cout << "Address:\t" << args->GetAddress() << std::endl;
    delete args;
    return 0;
}
