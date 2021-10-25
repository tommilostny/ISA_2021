/**
 * @brief TFTP client main module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <algorithm>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include "argumentparser.hpp"

int main()
{
    // Load arguments until end of file (loading file redirected to stdin).
    while (!std::cin.eof())
    {
        ArgumentParser* argParser;
        std::string args = "";

        std::cout << "> ";
        std::getline(std::cin, args);

        // Skip and load args again if there are none in stdin.
        if (args.empty() || std::all_of(args.begin(), args.end(), isspace))
            continue;

        try // Some args loaded, parse them and store in ArgumentParser class properties.
        {
            argParser = new ArgumentParser(args);
        }
        catch (const std::invalid_argument& exc)
        {
            std::cerr << exc.what() << std::endl;
            continue;
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

        int tftpClientSocket = socket(argParser->Domain, SOCK_DGRAM, 0);
        if (tftpClientSocket == -1)
        {
            std::cerr << "Could not create socket." << std::endl;
            return 1;
        }
        int connectResult = connect(tftpClientSocket,
                                    (sockaddr*)&argParser->SocketHint,
                                    argParser->Domain == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
        if (connectResult == -1)
        {
            std::cerr << "Could not connect to " << argParser->AddressStr << "." << std::endl;
            continue;
        }
        //TODO: send, recv with the server

        // Close socket and free the memory used by argument parser before the next prompt.
        close(tftpClientSocket);
        delete argParser;
    }
    return 0;
}
