/**
 * @brief TFTP class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <unistd.h>
#include <stdexcept>
#include "tftp.hpp"

#include <iostream> //TODO: remove :D

Tftp::Tftp(ArgumentParser* args)
{
    Args = args;
    Source = fopen(args->DestinationPath.c_str(), args->ReadMode ? "r" : "w");
    ClientSocket = -1;
}

Tftp::~Tftp()
{
    if (Source != NULL)
        fclose(Source);

    if (ClientSocket != -1)
        close(ClientSocket);
}

bool Tftp::Transfer()
{
    if ((ClientSocket = socket(Args->Domain, SOCK_DGRAM, 0)) == -1)
    {
        throw std::runtime_error("Could not create socket.");
    }
    int connectResult = connect(ClientSocket,
                                (sockaddr*)&Args->SocketHint,
                                Args->Domain == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
    if (connectResult == -1)
    {
        throw std::invalid_argument("Could not connect to " + Args->AddressStr + " on port " + std::to_string(Args->Port) + ".");
    }
    //TODO: send, recv with the server
    std::cout << "Transfering " << Args->DestinationPath << "..." << std::endl;
    return true;
}

uint8_t* Tftp::RequestPacket()
{
    return NULL;
}

uint8_t* Tftp::DataPacket(size_t n, void* data)
{
    return NULL;
}

uint8_t* Tftp::AcknowledgmentPacket()
{
    return NULL;
}

uint8_t* Tftp::ErrorPacket(std::string errorMessage)
{
    return NULL;
}
