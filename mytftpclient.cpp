/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <algorithm>
#include <iostream>
#include "ArgumentParser.hpp"
#include "StampMessagePrinter.hpp"
#include "Tftp.hpp"

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
        std::cerr << "Type \"help\" to display help." << std::endl;
        return NULL;
    }
    return argParser;
}

void RunTftpClient(ArgumentParser* argParser)
{
    Tftp* tftp;
    try
    {
        tftp = new Tftp(argParser);
        tftp->Transfer();
    }
    catch (const std::runtime_error& exc) //Transfer error.
    {
        StampMessagePrinter::PrintError(exc.what());
        std::cerr << "Type \"help\" to display help." << std::endl;
    }
    //Free resources before next iteration and prompt.
    delete tftp;
}

int main()
{
    // Load arguments until end of file (loading file redirected to stdin).
    while (!std::cin.eof())
    {
        //Show and parse the command prompt.
        auto argParser = ParsePromptArgs();
        //Displayed error message or help. Skip and load args again.
        if (argParser == NULL || argParser->HelpFlag)
        {
            delete argParser;
            continue;
        }
        // On "quit" or "exit", end the while loop and the program.
        if (argParser->ExitFlag)
        {
            delete argParser;
            break;
        }
        //Arguments are loaded correctly, create a new TFTP object and perform transfer.
        RunTftpClient(argParser);  
        delete argParser;   
    }
    return 0;
}
