#include <iostream>
#include "argumentparser.h"

int main()
{
    while (!std::cin.eof())
    {
        ArgumentParser* argParser;
        std::string args = "";

        std::cout << "\n> ";
        std::getline(std::cin, args);
        if (args.empty())
            continue;
        try
        {
            argParser = new ArgumentParser(args);
        }
        catch (const std::invalid_argument& exception)
        {
            std::cerr << exception.what() << std::endl;
            continue;
        }
        std::cout << "Read mode:\t" << argParser->GetReadMode() << std::endl;
        std::cout << "Write mode:\t" << argParser->GetWriteMode() << std::endl;
        std::cout << "Destination:\t" << argParser->GetDestinationPath() << std::endl;
        std::cout << "Is dest set?\t" << !(argParser->GetDestinationPath().empty()) << std::endl;
        std::cout << "Mode:\t\t" << argParser->GetMode() << std::endl;
        std::cout << "Timeout:\t" << argParser->GetTimeout() << std::endl;
        std::cout << "Size:\t\t" << argParser->GetSize() << std::endl;
        std::cout << "Multicast?\t" << argParser->GetMulticast() << std::endl;
        std::cout << "Address:\t" << argParser->GetAddress() << std::endl;
        std::cout << "IP version:\t" << argParser->GetAddressVersion() << std::endl;
        delete argParser;
    }
    return 0;
}
