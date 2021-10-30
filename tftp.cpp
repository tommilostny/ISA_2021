/**
 * @brief TFTP class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include "tftp.hpp"

#ifdef DEBUG
#include <iostream>
#endif

Tftp::Tftp(ArgumentParser* args)
{
    Args = args;
    ClientSocket = -1;

    std::string filemode = args->ReadMode ? "r" : "w";
    if (args->TransferMode == "octet")
        filemode += "b";

    if ((Source = fopen(args->DestinationPath.c_str(), filemode.c_str())) == NULL)
    {
        throw std::invalid_argument("Unable to open file " + args->DestinationPath + ".");
    }
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
        throw std::runtime_error("Could not connect to " + Args->AddressStr + " on port " + std::to_string(Args->Port) + ".");
    }
    //TODO: send, recv with the server

    #ifdef DEBUG
    std::cout << "Transfering " << Args->DestinationPath << "..." << std::endl;
    #endif

    RequestPacket();

    
    return true;
}

//PREREQUISITE: Socket is created (called only from Transfer method, ok :) )
//Create bytes and send them
//Return on response?

void _CopyOpcodeToPacket(char* packetPtr, uint16_t opcode)
{
    memcpy(packetPtr, &opcode, sizeof(uint16_t));
}

void Tftp::RequestPacket()
{
    /*
    2 bytes     string     1 byte     string   1 byte
    --------------------------------------------------
    | Opcode |  Filename  |   0  |    Mode    |   0  |
    --------------------------------------------------
                Figure 5-1: RRQ/WRQ packet
    */
    auto packetSize = Args->DestinationPath.size() + Args->TransferMode.size() + 4;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));

    _CopyOpcodeToPacket(packetPtr, Args->ReadMode ? 1U : 2U);
    strcpy(packetPtr + 2, Args->DestinationPath.c_str());
    strcpy(packetPtr + 3 + Args->DestinationPath.size(), Args->TransferMode.c_str());

    #ifdef DEBUG
    for (size_t i = 0; i < packetSize; i++)
    {
        if (i < 2 || packetPtr[i] == 0)
            printf("%d\n", packetPtr[i]);
        else
            printf("%c", packetPtr[i]);
    }
    #endif

    //TODO: send via socket

    free(packetPtr);
}

void Tftp::DataPacket(size_t n, void* data)
{
    /*
    2 bytes     2 bytes      n bytes
    ------------------------------------
    | Opcode |   Block #  |   Data     |
    ------------------------------------
        Figure 5-2: DATA packet
    */
    static uint16_t blockN = 0U;
    auto packetSize = n + 4;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));

    _CopyOpcodeToPacket(packetPtr, 3U);
    blockN++;
    memcpy(packetPtr + 2, &blockN, sizeof(uint16_t));
    memcpy(packetPtr + 4, data, n);

    //TODO: send via socket

    free(packetPtr);
}

void Tftp::AcknowledgmentPacket(uint16_t blockN)
{
    /*
    2 bytes     2 bytes
    -----------------------
    | Opcode |   Block #  |
    -----------------------
    Figure 5-3: ACK packet
    */
   char packetPtr[4];
   _CopyOpcodeToPacket(packetPtr, 4U);
   memcpy(packetPtr + 2, &blockN, sizeof(uint16_t));

   //TODO: send via socket
}
