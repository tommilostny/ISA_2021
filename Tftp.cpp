/**
 * @brief TFTP class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include "MessagePrinter.hpp"
#include "Tftp.hpp"

//Tftp class wide socket shortcut macros.
#define SEND(buffer, size)      sendto(ClientSocket, buffer, size, 0, (sockaddr*)&Args->ServerAddress, SocketLength)
#define RECEIVE(buffer, size)   recvfrom(ClientSocket, buffer, size, 0, (sockaddr*)&Args->ServerAddress, &SocketLength);

//Request packet option constants.
const char* blksizeReqOptStr = "blksize";
const char* timeoutReqOptStr = "timeout";
const char* tsizeReqOptStr = "tsize";
//const char* multicastReqOptStr = "multicast";

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
    //Create socket.
    if ((ClientSocket = socket(Args->Domain, SOCK_DGRAM, 0)) == -1)
    {
        throw std::runtime_error("Could not create socket.");
    }
    SocketLength = Args->Domain == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    //Set ClientSocket timeout.
    if (Args->Timeout > 0)
    {
        struct timeval tv = { .tv_sec = Args->Timeout, .tv_usec = 0 };
        if (setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
        {
            throw std::runtime_error("Could not set socket timeout.");
        }
    }
    //Get total file size from server response.
    size_t totalSize = Request();
    BlockN = 0;
    if (Args->ReadMode)
    {
        ReceiveData(totalSize);
        return;
    }
    if (Args->WriteMode)
    {
        //Send data packets to the server while the whole file is not read.
        TotalSentReceived = 0;
        while (SendDataBlock(totalSize));
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

size_t Tftp::Request()
{
    std::stringstream ss;
    ss << "Requesting " << (Args->ReadMode ? "READ from" : "WRITE to");
    ss << " server " << Args->AddressStr << " on port " << Args->Port << ".";
    MessagePrinter::PrintMessage(ss.str());
    /*
    2 bytes     string     1 byte     string   1 byte
    --------------------------------------------------
    | Opcode |  Filename  |   0  |    Mode    |   0  | + NULL teminated options/values
    --------------------------------------------------
                Figure 5-1: RRQ/WRQ packet
    */
    //Load options values.
    auto tsizeValStr = _GetTransferSize(Source, Args->WriteMode);
    auto timeoutValStr = std::to_string(Args->Timeout);
    auto blksizeValStr = std::to_string(Args->Size);
    
    //Store options sizes for memory allocation and logic if option is in the packet.
    int tsizeOptSize = (Args->TransferMode == "octet") * (strlen(tsizeReqOptStr) + tsizeValStr.size() + 2);
    int timeoutOptSize = (Args->Timeout != 0) * (strlen(timeoutReqOptStr) + timeoutValStr.size() + 2);
    int blksizeOptSize = (Args->Size != 512) * (strlen(blksizeReqOptStr) + blksizeValStr.size() + 2);
    //int multicastOptSize = (int)Args->Multicast * (multicastReqOptStr.size() + 1);

    int optionsSize = tsizeOptSize + timeoutOptSize + blksizeOptSize/* + multicastOptSize*/;
    
    //Allocate dynamically sized packet (without unnecessary options).
    auto packetSize = 4 + Args->DestinationPath.size() + Args->TransferMode.size() + optionsSize;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));

    _CopyOpcodeToPacket(packetPtr, Args->ReadMode ? OPCODE_RRQ : OPCODE_WRQ);
    
    //Pointer for copying values to the addresses in packet.
    auto currentPtr = packetPtr + 2;
    
    strcpy(currentPtr, Args->DestinationPath.c_str());
    currentPtr += 1 + Args->DestinationPath.size();

    strcpy(currentPtr, Args->TransferMode.c_str());
    currentPtr += 1 + Args->TransferMode.size();

    if (tsizeOptSize)
    {
        strcpy(currentPtr, tsizeReqOptStr);
        currentPtr += 1 + strlen(tsizeReqOptStr);
        strcpy(currentPtr, tsizeValStr.c_str());
        currentPtr += 1 + tsizeValStr.size();
    }
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
        //TODO before multicast option: currentPtr += 1 + blksizeValStr.size();
    }
    SEND(packetPtr, packetSize);
    free(packetPtr);

    auto responseBuffer = (char*)calloc(42 + optionsSize, sizeof(char));
    auto received = RECEIVE(responseBuffer, 42 + optionsSize);
    if (received == -1)
    {
        free(responseBuffer);
        throw std::runtime_error("Server did not respond.");
    }
    if (responseBuffer[1] == 5) //opcode 5 => error packet
    {
        std::string message = "Error from the server:  " + std::string(responseBuffer + 4);
        free(responseBuffer);
        throw std::runtime_error(message);
    }
    if (Args->TransferMode == "octet")
    {
        //Get tsize option from the response packet (useful for reading from the server).
        std::string responseTsize = responseBuffer + 3 + strlen(tsizeReqOptStr);
        free(responseBuffer);
        return std::stoul(responseTsize);
    }
    return 0;
}

bool Tftp::SendDataBlock(size_t totalFileSize)
{
    /*
    2 bytes     2 bytes      n bytes
    ------------------------------------
    | Opcode |   Block #  |   Data     |
    ------------------------------------
        Figure 5-2: DATA packet
    */
    //Increment block number, mark total send.
    BlockN++;
    size_t totalToSend = TotalSentReceived + Args->Size <= totalFileSize ? Args->Size : totalFileSize - TotalSentReceived;

    //Create packet with dynamic length (smaller for the last chunk of data).
    auto packetSize = 4 + totalToSend;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));

    TotalSentReceived += totalToSend;
    std::stringstream ss;
    ss << "Sending DATA #" << BlockN <<" ... " << TotalSentReceived << " B of " << totalFileSize << " B.";
    MessagePrinter::PrintMessage(ss.str());

    //Fill in packet header.
    _CopyOpcodeToPacket(packetPtr, OPCODE_DATA);
    BlockN = htons(BlockN);
    memcpy(packetPtr + 2, &BlockN, sizeof(uint16_t));
    BlockN = ntohs(BlockN);

    //Read data from file and send them.
    fread(packetPtr + 4, sizeof(char), totalToSend, Source);
    int sendResult;
    do
    {
        sendResult = SEND(packetPtr, packetSize);
    }
    while (sendResult == -1);
    free(packetPtr);

    //Check received acknowledgement.
    char ackBuffer[4];
    auto received = RECEIVE(ackBuffer, 4);
    if (received == -1 || ackBuffer[1] != OPCODE_ACK)
    {
        throw std::runtime_error("Error while transfering data.");
    }
    //Transmission end condition (sent the whole file?).
    return TotalSentReceived < totalFileSize;
}

size_t _DataWriteSize(ArgumentParser* args, char* asciiData, size_t bufferSize)
{
    if (args->TransferMode == "octet")
        return bufferSize - 4;
    return strlen(asciiData);
}

void Tftp::ReceiveData(size_t totalFileSize)
{
    size_t totalReceived = 0;
    size_t bufferSize = 4 + Args->Size;
    auto buffer = (char*)malloc(bufferSize);
    int received;
    do
    {
        SendAcknowledgment(BlockN++);
        memset(buffer, 0, bufferSize);
        received = RECEIVE(buffer, bufferSize);
        if (received == -1)
        {
            BlockN--;
            continue;
        }
        totalReceived += received - 4;

        std::stringstream ss;
        ss << "Receiving DATA #" << BlockN << " ... " << totalReceived << " B";
        if (Args->TransferMode == "octet")
            ss << " of " << totalFileSize << " B.";
        else
            ss << '.';
        MessagePrinter::PrintMessage(ss.str());

        fwrite(buffer + 4, sizeof(char), _DataWriteSize(Args, buffer + 4, bufferSize), Source);
    }
    while (received == (int)bufferSize);

    SendAcknowledgment(BlockN);
    free(buffer);
}

void Tftp::SendAcknowledgment(uint16_t blockN)
{
    /*
    2 bytes     2 bytes
    -----------------------
    | Opcode |   Block #  |
    -----------------------
    Figure 5-3: ACK packet
    */
    char packetPtr[4];
    _CopyOpcodeToPacket(packetPtr, OPCODE_ACK);
    blockN = htons(blockN);
    memcpy(packetPtr + 2, &blockN, sizeof(uint16_t));
    int sendResult;
    do
    {
        sendResult = SEND(packetPtr, 4);
    }
    while (sendResult == -1);
}

void Tftp::SendError(uint16_t errorCode, std::string message)
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

    _CopyOpcodeToPacket(packetPtr, OPCODE_ERROR);
    memcpy(packetPtr + 2, &errorCode, sizeof(uint16_t));
    strcpy(packetPtr + 4, message.c_str());

    SEND(packetPtr, packetSize);
    free(packetPtr);
}
