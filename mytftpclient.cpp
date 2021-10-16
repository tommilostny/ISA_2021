/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <iostream>
#include <algorithm>

#include "argumentparser.hpp"

int main()
{
    // Load arguments until end of file (loading file redirected to stdin).
    while (!std::cin.eof())
    {
        ArgumentParser* arg_parser;
        std::string args = "";

        std::cout << "> ";
        std::getline(std::cin, args);

        // Skip and load args again if there are none in stdin.
        if (args.empty() || std::all_of(args.begin(), args.end(), isspace))
            continue;

        try // Some args loaded, parse them and store in ArgumentParser class properties.
        {
            arg_parser = new ArgumentParser(args);
        }
        catch (const std::invalid_argument& exception)
        {
            std::cerr << exception.what() << std::endl;
            continue;
        }
        // Debugging messages.
        #ifdef DEBUG
        std::cout << "Read mode:\t"   << arg_parser->get_read_mode()        << std::endl;
        std::cout << "Write mode:\t"  << arg_parser->get_write_mode()       << std::endl;
        std::cout << "Destination:\t" << arg_parser->get_destination_path() << std::endl;
        std::cout << "Mode:\t\t"      << arg_parser->get_transfer_mode()    << std::endl;
        std::cout << "Timeout:\t"     << arg_parser->get_timeout()          << std::endl;
        std::cout << "Size:\t\t"      << arg_parser->get_size()             << std::endl;
        std::cout << "Multicast:\t"   << arg_parser->get_multicast()        << std::endl;
        std::cout << "Address:\t"     << arg_parser->get_address_string()   << std::endl;
        std::cout << "IP version:\t"  << arg_parser->get_address_version()  << std::endl;
        std::cout << "Port:\t\t"      << arg_parser->get_port()             << std::endl;
        #endif
        // Free the memory used by the class before the next iteration.
        delete arg_parser;
    }
    return 0;
}
