/**
 * @brief TFTP class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <iomanip>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include "StampMessagePrinter.hpp"
#include "Tftp.hpp"

//TFTP protocol packet opcodes.
#define OPCODE_RRQ      1
#define OPCODE_WRQ      2
#define OPCODE_DATA     3
#define OPCODE_ACK      4
#define OPCODE_ERROR    5

//Tftp class wide socket shortcut macros.
#define SEND(buffer, size)      sendto(ClientSocket, buffer, size, 0, (sockaddr*)&Args->ServerAddress, SocketLength)
#define RECEIVE(buffer, size)   recvfrom(ClientSocket, buffer, size, 0, (sockaddr*)&Args->ServerAddress, &SocketLength)

//Request packet option constants.
const char* blksizeReqOptStr = "blksize";
const char* timeoutReqOptStr = "timeout";
const char* tsizeReqOptStr = "tsize";

Tftp::Tftp(ArgumentParser* args)
{
    Args = args;
    ClientSocket = -1;
    DestinationFile = NULL;
}

Tftp::~Tftp()
{
    if (DestinationFile != NULL)
        fclose(DestinationFile);

    if (ClientSocket != -1)
        close(ClientSocket);
}

void _OpenFile(FILE*& file, ArgumentParser* args, char fopenMode)
{
    char fileMode[3] = { fopenMode, '\0', '\0' };

    if (args->TransferMode == "octet")
        fileMode[1] = 'b';

    if ((file = fopen(args->DestinationPath.c_str(), fileMode)) == NULL)
        throw std::runtime_error("Cannot open file" + args->DestinationPath + ".");
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
        struct timeval timeValue = { Args->Timeout, 0 };
        if (setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeValue, sizeof(timeValue)) == -1)
        {
            throw std::runtime_error("Could not set socket timeout.");
        }
    }
    //Open file if it is going to be read on the client side (WRQ).
    if (Args->WriteMode)
        _OpenFile(DestinationFile, Args, 'r');

    //Get total file size from server response.
    size_t totalSize = Request();

    //Open file if it is going to be written to the server side (RRQ).
    if (Args->ReadMode)
        _OpenFile(DestinationFile, Args, 'w');

    //Transfer file.
    if (Args->ReadMode)
        ReceiveData(totalSize);
    else
        SendData(totalSize);
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
    StampMessagePrinter::Print(ss.str());
    /*
    2 bytes     string     1 byte     string   1 byte
    --------------------------------------------------
    | Opcode |  Filename  |   0  |    Mode    |   0  | + NULL teminated options/values
    --------------------------------------------------
                Figure 5-1: RRQ/WRQ packet
    */
    //Load options values.
    auto tsizeValStr = _GetTransferSize(DestinationFile, Args->WriteMode);
    auto timeoutValStr = std::to_string(Args->Timeout);
    auto blksizeValStr = std::to_string(Args->Size);
    
    //Store options sizes for memory allocation and logic if option is in the packet.
    int tsizeOptSize = (Args->TransferMode == "octet") * (strlen(tsizeReqOptStr) + tsizeValStr.size() + 2);
    int timeoutOptSize = (Args->Timeout != 0) * (strlen(timeoutReqOptStr) + timeoutValStr.size() + 2);
    int blksizeOptSize = (Args->Size != 512) * (strlen(blksizeReqOptStr) + blksizeValStr.size() + 2);

    int optionsSize = tsizeOptSize + timeoutOptSize + blksizeOptSize;
    
    //Allocate dynamically sized packet (without unnecessary options).
    auto packetSize = 4 + Args->DestinationPath.size() + Args->TransferMode.size() + optionsSize;
    auto packetPtr = (char*)calloc(packetSize, sizeof(char));
    if (packetPtr == NULL)
    {
        throw std::runtime_error("Could not allocate memory for request packet.");
    }
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
    }
    SEND(packetPtr, packetSize);
    free(packetPtr);

    auto bufferSize = 42 + optionsSize;
    auto responseBuffer = (char*)calloc(bufferSize, sizeof(char));
    if (responseBuffer == NULL)
    {
        throw std::runtime_error("Could not allocate memory for response buffer.");
    }
    auto received = RECEIVE(responseBuffer, bufferSize);
    if (received == -1)
    {
        free(responseBuffer);
        throw std::runtime_error("Server did not respond.");
    }
    //Received an error packet, display message from server and end with error.
    if (ntohs(*(uint16_t*)responseBuffer) == OPCODE_ERROR)
    {
        /*
        2 bytes     2 bytes      string    1 byte
        -------------------------------------------
        | Opcode |  ErrorCode |   ErrMsg   |   0  |
        -------------------------------------------
                Figure 5-4: ERROR packet
        */
        auto message = "Error from the server:  " + std::string(responseBuffer + 4);
        free(responseBuffer);
        throw std::runtime_error(message);
    }
    size_t tsizeFromResponse;
    //Received an OACK packet, read tsize option from it if transfer mode is octet.
    if (Args->TransferMode == "octet")
    {
        //Get tsize option from the response packet (useful for reading from the server).
        std::string responseTsize = responseBuffer + 3 + strlen(tsizeReqOptStr);
        tsizeFromResponse = std::stoul(responseTsize);
    }
    else tsizeFromResponse = std::stoul(tsizeValStr);
    free(responseBuffer);
    return tsizeFromResponse;
}

void Tftp::SendData(size_t totalFileSize)
{
    size_t totalSent = 0;
    size_t blockN = 0;
    //Send data packets to the server while the whole file is not read.
    while (totalSent < totalFileSize)
    {
        /*
        2 bytes     2 bytes      n bytes
        ------------------------------------
        | Opcode |   Block #  |   Data     |
        ------------------------------------
            Figure 5-2: DATA packet
        */
        //Increment block number, mark total send.
        blockN++;
        size_t totalToSend = totalSent + Args->Size <= totalFileSize ? Args->Size : totalFileSize - totalSent;

        //Create packet with dynamic length (smaller for the last chunk of data).
        auto packetSize = 4 + totalToSend;
        auto packetPtr = (char*)calloc(packetSize, sizeof(char));
        if (packetPtr == NULL)
        {
            throw std::runtime_error("Could not allocate memory for data packet.");
        }
        totalSent += totalToSend;
        std::stringstream ss;
        ss << "Sending DATA #" << blockN <<" ... " << totalSent << " B of " << totalFileSize << " B.";
        StampMessagePrinter::Print(ss.str());

        //Fill in packet header.
        _CopyOpcodeToPacket(packetPtr, OPCODE_DATA);
        blockN = htons(blockN);
        memcpy(packetPtr + 2, &blockN, sizeof(uint16_t));
        blockN = ntohs(blockN);

        //Read data from file and send them.
        fread(packetPtr + 4, sizeof(char), totalToSend, DestinationFile);
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
        if (received == -1 || ntohs(*(uint16_t*)ackBuffer) != OPCODE_ACK)
        {
            throw std::runtime_error("Error while transfering data.");
        }
    } 
}

void Tftp::ReceiveData(size_t totalFileSize)
{
    size_t blockN = 0;
    size_t totalReceived = 0;
    /*
    2 bytes     2 bytes      n bytes
    ------------------------------------
    | Opcode |   Block #  |   Data     |
    ------------------------------------
        Figure 5-2: DATA packet
    */
    size_t bufferSize = 4 + Args->Size;
    auto buffer = (char*)malloc(bufferSize);
    if (buffer == NULL)
    {
        throw std::runtime_error("Could not allocate memory for data buffer.");
    }
    int received;
    do
    {
        SendAcknowledgment(blockN++);
        memset(buffer, 0, bufferSize);
        
        if ((received = RECEIVE(buffer, bufferSize)) == -1)
        {
            free(buffer);
            throw std::runtime_error("Lost connection to the server.");
        }
        if (ntohs(((uint16_t*)buffer)[1]) != blockN)
        {
            blockN--;
            continue;
        }
        auto dataLength = received - 4;
        totalReceived += dataLength;

        std::stringstream ss;
        ss << "Received DATA #" << blockN << " ... " << totalReceived << " B";
        if (totalFileSize > 0)
            ss << " of " << totalFileSize << " B.";
        else
            ss << '.';
        StampMessagePrinter::Print(ss.str());

        fwrite(buffer + 4, sizeof(char), dataLength, DestinationFile);
    }
    while (received == (int)bufferSize);

    SendAcknowledgment(blockN);
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
