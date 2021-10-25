/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <iostream>
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>
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
        std::cout << "Read mode:\t"   << arg_parser->read_mode        << std::endl;
        std::cout << "Write mode:\t"  << arg_parser->write_mode       << std::endl;
        std::cout << "Destination:\t" << arg_parser->destination_path << std::endl;
        std::cout << "Mode:\t\t"      << arg_parser->transfer_mode    << std::endl;
        std::cout << "Timeout:\t"     << arg_parser->timeout          << std::endl;
        std::cout << "Size:\t\t"      << arg_parser->size             << std::endl;
        std::cout << "Multicast:\t"   << arg_parser->multicast        << std::endl;
        std::cout << "Address:\t"     << arg_parser->addr_str         << std::endl;
        std::cout << "IP version:\t"  << (arg_parser->domain == AF_INET ? "AF_INET" : "AF_INET6") << std::endl;
        std::cout << "Port:\t\t"      << arg_parser->port        << std::endl;
        #endif

        int sock = socket(arg_parser->domain, SOCK_DGRAM, 0);
        if (sock == -1)
        {
            std::cerr << "Could not create socket." << std::endl;
            return 1;
        }
        int conn = connect(sock, (sockaddr*)&arg_parser->socket_hint, arg_parser->domain == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
        if (conn == -1)
        {
            std::cerr << "Could not connect to " << arg_parser->addr_str << "." << std::endl;
            continue;
        }
        //TODO: send, recv with the server

        // Close socket and free the memory used by argument parser before the next prompt.
        close(sock);
        delete arg_parser;
    }
    return 0;
}
