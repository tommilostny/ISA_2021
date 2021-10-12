#include <getopt.h>
#include <string.h>
#include <stdexcept>
#include "argumentparser.h"

ArgumentParser::ArgumentParser(int argc, char **argv)
{
    bool destFlag, timeoutFlag, sizeFlag, modeFlag, addrFlag;
    int option;

    while ((option = getopt(argc, argv, "RWd:t:s:mc:a:")) != -1)
    {
        switch (option)
        {
        case 'R':   ParseRead();                        break;
        case 'W':   ParseWrite();                       break;
        case 'd':   ParseDestination(destFlag, optarg); break;
        case 't':   ParseTimeout(timeoutFlag, optarg);  break;
        case 's':   ParseSize(sizeFlag, optarg);        break;
        case 'm':   ParseMulticast();                   break;
        case 'c':   ParseMode(modeFlag, optarg);        break;
        case 'a':   ParseAddress(addrFlag, optarg);     break;
        default:
            throw std::invalid_argument("Invalid argument: '" + std::to_string(option) + "'.");
            break;
        }
    }
    //TODO: #2 Checks for required arguments.
    if (!ReadMode && !WriteMode)
    {
        throw std::invalid_argument("Missing required argument -R (read mode) or -W (write mode).");
    }
    if (!destFlag)
    {
        throw std::invalid_argument("Missing required argument -d <file-path>.");
    }
}

void ArgumentParser::ParseRead()
{
    if (ReadMode || WriteMode)
    {
        throw std::invalid_argument("Argument -W or -R is already set.");
    }
    ReadMode = true;
}

void ArgumentParser::ParseWrite()
{
    if (WriteMode || ReadMode)
    {
        throw std::invalid_argument("Argument -W or -R is already set.");
    }
    WriteMode = true;
}

void ArgumentParser::ParseDestination(bool& destinationFlag, char* optarg)
{
    if (destinationFlag)
    {
        throw std::invalid_argument("Argument -d is already set to '" + DestinationPath + "'.");
    }
    DestinationPath = optarg;
    destinationFlag = true;
}

void ArgumentParser::ParseTimeout(bool& timeoutFlag, char* optarg)
{
    if (timeoutFlag)
    {
        throw std::invalid_argument("Argument -t is already set to '" + std::to_string(Timeout) + "'.");
    }
    try
    {
        Timeout = std::stoi(optarg);
        if (Timeout < 0)
            throw std::exception();
    }
    catch (const std::exception &e)
    {
        throw std::invalid_argument("Invalud value for argument -t: " + (std::string)optarg);
    }
    timeoutFlag = true;
}

void ArgumentParser::ParseSize(bool& sizeFlag, char* optarg)
{
    if (sizeFlag)
    {
        throw std::invalid_argument("Argument -s is already set to '" + std::to_string(Size) + "'.");
    }
    try
    {
        Size = std::stoi(optarg);
        if (Size < 0)
            throw std::exception();
    }
    catch (const std::exception &e)
    {
        throw std::invalid_argument("Invalud value for argument -s: " + (std::string)optarg);
    }
    sizeFlag = true;
}

void ArgumentParser::ParseMulticast()
{
    if (Multicast)
    {
        throw std::invalid_argument("Argument -m is already set.");
    }
    Multicast = true;
}

void ArgumentParser::ParseMode(bool& modeFlag, char* optarg)
{
    if (modeFlag)
    {
        throw std::invalid_argument("Argument -c is already set to '" + std::to_string(Mode) + "'.");
    }
    if (strcmp(optarg, "binary") == 0 || strcmp(optarg, "octet") == 0)
    {
        Mode = BINARY;
    }
    else if (strcmp(optarg, "ascii") == 0 || strcmp(optarg, "netascii") == 0)
    {
        Mode = ASCII;
    }
    else
        throw std::invalid_argument("Invalid value for argument -c: " + (std::string)optarg);
    modeFlag = true;
}

void ArgumentParser::ParseAddress(bool& addressFlag, char* optarg)
{
    if (addressFlag)
    {
        throw std::invalid_argument("Argument -a already set to '" + Address + "'.");
    }
    //TODO: #1 IP address parsing...
    Address = optarg;
    addressFlag = true;
}

bool ArgumentParser::GetReadMode()
{
    return ReadMode;
}

bool ArgumentParser::GetWriteMode()
{
    return WriteMode;
}

std::string ArgumentParser::GetDestinationPath()
{
    return DestinationPath;
}

int ArgumentParser::GetTimeout()
{
    return Timeout;
}

int ArgumentParser::GetSize()
{
    return Size;
}

bool ArgumentParser::GetMulticast()
{
    return Multicast;
}

enum Mode ArgumentParser::GetMode()
{
    return Mode;
}

std::string ArgumentParser::GetAddress()
{
    return Address;
}
