#include <getopt.h>
#include <stdexcept>
#include <string.h>
#include "argumentparser.h"

ArgumentParser::ArgumentParser(int argc, char **argv)
{
    for (int option; (option = getopt(argc, argv, "RWd:t:s:mc:a:")) != -1;)
    {
        switch (option)
        {
        case 'R':
            ParseRead();
            break;
        case 'W':
            ParseWrite();
            break;
        case 'd':
            ParseDestination(optarg);
            break;
        case 't':
            ParseTimeout(optarg);
            break;
        case 's':
            ParseSize(optarg);
            break;
        case 'm':
            ParseMulticast();
            break;
        case 'c':
            ParseMode(optarg);
            break;
        case 'a':
            ParseAddress(optarg);
            break;
        default:
            throw std::invalid_argument("Invalid argument: '" + std::to_string(option) + "'.");
            break;
        }
    }
    //TODO: #2 Checks for required arguments.
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

void ArgumentParser::ParseDestination(char* optarg)
{
    if (DestinationSet)
    {
        throw std::invalid_argument("Argument -d is already set to '" + DestinationPath + "'.");
    }
    DestinationPath = optarg;
    DestinationSet = true;
}

void ArgumentParser::ParseTimeout(char* optarg)
{
    if (TimeoutSet)
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
    TimeoutSet = true;
}

void ArgumentParser::ParseSize(char* optarg)
{
    if (SizeSet)
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
    SizeSet = true;
}

void ArgumentParser::ParseMulticast()
{
    if (Multicast)
    {
        throw std::invalid_argument("Argument -m is already set.");
    }
    Multicast = true;
}

void ArgumentParser::ParseMode(char* optarg)
{
    if (ModeSet)
    {
        throw std::invalid_argument("Argument -c is already set to '" + std::to_string(Mode) + "'.");
    }
    if (strcmp(optarg, "binary") == 0 || strcmp(optarg, "octet") == 0)
    {
        Mode = BINARY;
        return;
    }
    if (strcmp(optarg, "ascii") == 0 || strcmp(optarg, "netascii") == 0)
    {
        Mode = ASCII;
        return;
    }
    throw std::invalid_argument("Invalid value for argument -c: " + (std::string)optarg);
}

void ArgumentParser::ParseAddress(char* optarg)
{
    if (AddressSet)
    {
        throw std::invalid_argument("Argument -a already set to '" + Address + "'.");
    }
    //TODO: #1 IP address parsing...
    Address = optarg;
    AddressSet = true;
}
