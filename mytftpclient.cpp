/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <algorithm>
#include <iostream>
#include "argumentparser.hpp"
#include "tftp.hpp"

ArgumentParser* ParsePromptArgs()
{
    ArgumentParser* argParser;
    std::string args;

    std::cout << "> ";
    std::getline(std::cin, args);

    // Skip and load args again if there are none in stdin.
    if (args.empty() || std::all_of(args.begin(), args.end(), isspace))
        return NULL;

    try // Some args loaded, parse them and store in ArgumentParser class properties.
    {
        argParser = new ArgumentParser(args);
    }
    catch (const std::invalid_argument& exc)
    {
        std::cerr << exc.what() << std::endl;
        return NULL;
    }
    // Debugging messages.
    #ifdef DEBUG
    std::cout << "Read mode:\t"   << argParser->ReadMode        << std::endl;
    std::cout << "Write mode:\t"  << argParser->WriteMode       << std::endl;
    std::cout << "Destination:\t" << argParser->DestinationPath << std::endl;
    std::cout << "Mode:\t\t"      << argParser->TransferMode    << std::endl;
    std::cout << "Timeout:\t"     << argParser->Timeout         << std::endl;
    std::cout << "Size:\t\t"      << argParser->Size            << std::endl;
    std::cout << "Multicast:\t"   << argParser->Multicast       << std::endl;
    std::cout << "Address:\t"     << argParser->AddressStr      << std::endl;
    std::cout << "Port:\t\t"      << argParser->Port            << std::endl;
    std::cout << "Domain:\t"      << (argParser->Domain == AF_INET ? "AF_INET" : "AF_INET6") << std::endl;
    #endif
    return argParser;
}

int main()
{
    // Load arguments until end of file (loading file redirected to stdin).
    while (!std::cin.eof())
    {
        auto argParser = ParsePromptArgs();
        if (argParser == NULL)
            continue;

        Tftp* tftp;
        try //Arguments are loaded correctly, create a new TFTP object and perform transfer.
        {
            tftp = new Tftp(argParser);
            tftp->Transfer();
        }
        catch (const std::invalid_argument& exc) //Invalid filename in read mode.
        {
            std::cerr << exc.what() << std::endl;
            delete argParser;
            continue;
        }
        catch (const std::runtime_error& exc) //Transfer error.
        {
            std::cerr << exc.what() << std::endl;
        }
        //Free resources before next iteration and prompt.
        delete tftp;
        delete argParser;        
    }
    return 0;
}
