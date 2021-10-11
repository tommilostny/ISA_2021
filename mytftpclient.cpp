#include <iostream>
#include <getopt.h>
#include "argumentparser.h"

int main(int argc, char** argv)
{
    ArgumentParser* argParser;
    try
    {
        argParser = new ArgumentParser(argc, argv);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::cout << "Read mode:\t" << argParser->ReadMode << std::endl;
    std::cout << "Destination:\t" << argParser->DestinationPath << std::endl;
    std::cout << "Mode:\t\t" << argParser->Mode << std::endl;
    std::cout << "Timeout:\t" << argParser->Timeout << std::endl;
    std::cout << "Size:\t\t" << argParser->Size << std::endl;
    delete argParser;
    return 0;
}
