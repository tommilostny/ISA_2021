/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <algorithm>
#include <iostream>
#include "ArgumentParser.hpp"
#include "MessagePrinter.hpp"
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

int main()
{
    // Load arguments until end of file (loading file redirected to stdin).
    while (!std::cin.eof())
    {
        auto argParser = ParsePromptArgs();
        if (argParser == NULL || argParser->HelpFlag) //Displayed error message or help. Skip and load args again.
        {
            delete argParser;
            continue;
        }
        if (argParser->ExitFlag) // On "quit" or "exit" end the while loop and the program.
        {
            delete argParser;
            break;
        }

        Tftp* tftp;
        try //Arguments are loaded correctly, create a new TFTP object and perform transfer.
        {
            tftp = new Tftp(argParser);
            tftp->Transfer();
        }
        catch (const std::invalid_argument& exc) //Invalid filename in read mode.
        {
            MessagePrinter::PrintError(exc.what());
            std::cerr << "Type \"help\" to display help." << std::endl;
            delete argParser;
            continue;
        }
        catch (const std::runtime_error& exc) //Transfer error.
        {
            MessagePrinter::PrintError(exc.what());
            std::cerr << "Type \"help\" to display help." << std::endl;
        }
        //Free resources before next iteration and prompt.
        delete tftp;
        delete argParser;        
    }
    return 0;
}
