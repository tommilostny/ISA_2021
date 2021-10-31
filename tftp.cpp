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

#define SEND(buffer, size)      sendto(ClientSocket, buffer, size, 0, (sockaddr*)&Args->SocketHint, SocketLength)
#define RECEIVE(buffer, size)   recvfrom(ClientSocket, buffer, size, 0, (sockaddr*)&Args->SocketHint, &SocketLength);

const char* blksizeReqOptStr = "blksize";
const char* timeoutReqOptStr = "timeout";
const char* tsizeReqOptStr = "tsize";
//conts std::string multicastReqOptStr = "multicast";

Tftp::Tftp(ArgumentParser* args)
{
    Args = args;
    ClientSocket = -1;

    std::string filemode = args->ReadMode ? "w" : "r";
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

void Tftp::Transfer()
{
    if ((ClientSocket = socket(Args->Domain, SOCK_DGRAM, 0)) == -1)
    {
        throw std::runtime_error("Could not create socket.");
    }
    SocketLength = Args->Domain == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    #ifdef DEBUG
    std::cout << "Transfering " << Args->DestinationPath << "..." << std::endl;
    #endif

    RequestPacket();

    if (Args->ReadMode)
    {

        return;
    }
    if (Args->WriteMode)
    {

    }
}

std::string _GetTransferSize(FILE* file, bool isWriteMode)
{
    if (isWriteMode)
    {
        fseek(file, 0, SEEK_END);
        auto fSizeAsString = std::to_string(ftell(file));
        rewind(file);
        return fSizeAsString;
    }
    return "0";
}

void _CopyOpcodeToPacket(char* packetPtr, uint16_t opcode)
{
    opcode = htons(opcode);
    memcpy(packetPtr, &opcode, sizeof(uint16_t));
}

int Tftp::RequestPacket()
{
    /*
    2 bytes     string     1 byte     string   1 byte
    --------------------------------------------------
    | Opcode |  Filename  |   0  |    Mode    |   0  | + NULL teminated options/values
    --------------------------------------------------
                Figure 5-1: RRQ/WRQ packet
    */
    auto tsizeValStr = _GetTransferSize(Source, Args->WriteMode);
    auto timeoutValStr = std::to_string(Args->Timeout);
    auto blksizeValStr = std::to_string(Args->Size);
    
    int tsizeOptSize = strlen(tsizeReqOptStr) + tsizeValStr.size() + 2;
    int timeoutOptSize = (Args->Timeout != 0) * (strlen(timeoutReqOptStr) + timeoutValStr.size() + 2);
    int blksizeOptSize = (Args->Size != 512) * (strlen(blksizeReqOptStr) + blksizeValStr.size() + 2);
    //int multicastOptSize = (int)Args->Multicast * (multicastReqOptStr.size() + 1);

    int optionsSize = tsizeOptSize + timeoutOptSize + blksizeOptSize/* + multicastOptSize*/;
    
    auto packetSize = 4 + Args->DestinationPath.size() + Args->TransferMode.size() + optionsSize;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));
    auto currentPtr = packetPtr;

    _CopyOpcodeToPacket(packetPtr, Args->ReadMode ? 1U : 2U);
    currentPtr += 2;
    
    strcpy(currentPtr, Args->DestinationPath.c_str());
    currentPtr += 1 + Args->DestinationPath.size();

    strcpy(currentPtr, Args->TransferMode.c_str());
    currentPtr += 1 + Args->TransferMode.size();

    strcpy(currentPtr, tsizeReqOptStr);
    currentPtr += 1 + strlen(tsizeReqOptStr);
    strcpy(currentPtr, tsizeValStr.c_str());
    currentPtr += 1 + tsizeValStr.size();

    if (timeoutOptSize)
    {
        strcpy(currentPtr, timeoutReqOptStr);
        currentPtr += 1 + strlen(timeoutReqOptStr);
        strcpy(currentPtr, timeoutValStr.c_str());
        currentPtr += 1 + timeoutValStr.size();
    }
    if (blksizeOptSize)
    {
        strcpy(currentPtr, blksizeReqOptStr);
        currentPtr += 1 + strlen(blksizeReqOptStr);
        strcpy(currentPtr, blksizeValStr.c_str());
        //TODO befotr multicast option: currentPtr += 1 + blksizeValStr.size();
    }
    #ifdef DEBUG
    for (size_t i = 0; i < packetSize; i++)
    {
        if (i < 2 || packetPtr[i] == 0)
            printf("(%d)\n", packetPtr[i]);
        else
            printf("%c", packetPtr[i]);
    }
    std::cout << "Sending request" << std::endl;
    #endif

    SEND(packetPtr, packetSize);

    #ifdef DEBUG
    std::cout << "Receiving request answer." << std::endl;
    #endif

    auto responseBuffer = (char*)calloc(2 + optionsSize, sizeof(char));
    auto received = RECEIVE(responseBuffer, 2 + optionsSize);
    std::string responseTsize = "0";
    if (received != -1)
    {
        responseTsize = responseBuffer + 3 + strlen(tsizeReqOptStr);
        #ifdef DEBUG
        std::cout << "TSIZE: " << responseTsize << std::endl;

        for (int i = 0; i < received; i++)
        {
            if (i < 2 || responseBuffer[i] == 0)
                printf("(%d)\n", responseBuffer[i]);
            else
                printf("%c", responseBuffer[i]);
        }
        #endif
    }
    //else
    //{
        #ifdef DEBUG
        std::cout << "Teminanting." << std::endl;
        #endif
        ErrorPacket(8, "testing");
        //return -1;
    //}
    free(responseBuffer);
    free(packetPtr);
    return std::stoi(responseTsize);
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

void Tftp::ErrorPacket(uint16_t errorCode, std::string message)
{
    /*
    2 bytes     2 bytes      string    1 byte
    -------------------------------------------
    | Opcode |  ErrorCode |   ErrMsg   |   0  |
    -------------------------------------------
            Figure 5-4: ERROR packet
    */
    auto packetSize = 5 + message.size();
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));

    _CopyOpcodeToPacket(packetPtr, 5U);
    memcpy(packetPtr + 2, &errorCode, sizeof(uint16_t));
    strcpy(packetPtr + 4, message.c_str());

    SEND(packetPtr, packetSize);
    free(packetPtr);
}
