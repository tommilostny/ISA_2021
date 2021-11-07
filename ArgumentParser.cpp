/**
 * @brief Argument parser class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <getopt.h>
#include <malloc.h>
#include <regex>
#include <stdexcept>
#include <string.h>
#include <vector>
#include "ArgumentParser.hpp"

/// Initialize argv and argc from string for getopt function.
void _ArgsForGetopt(std::string args, int& argc, char**& argv)
{
    // Initialize argc to 1 and first argv item to program name.
    argc = 1;
    auto progArg = (char*)malloc(13 * sizeof(char));
    if (progArg == NULL)
    {
        throw std::runtime_error("Memory allocation error.");
    }
    // Fill program name array memory, set it as first item of a temporaty vector of loaded arguments.
    strcpy(progArg, "mytftpclient");
    std::vector<char*> argsVect = { progArg };

    while (!args.empty())
    {
        // Get a substing from start to first whitespace character
        // or port with ',' and whitespaces for IP addresses.
        std::smatch match;
        std::regex_search(args, match, std::regex("\\s+(,\\s*\\d{1,5})?"));
        auto spacePos = match.position() + match.length();
        auto arg = std::regex_replace(args.substr(0, spacePos), std::regex("\\s"), "");

        if (!arg.empty()) // Loaded an actual option? Skip unnecessary whitespaces.
        {
            // Copy substring to a new character array and save it in the temporary vector.
            auto argArray = (char*)malloc((arg.length() + 1) * sizeof(char));
            if (argArray == NULL)
            {
                throw std::runtime_error("Memory allocation error.");
            }
            strcpy(argArray, arg.c_str());
            argsVect.push_back(argArray);
            argc++;
        }
        if (spacePos == -1) // No more whitespaces found (end of string).
            break;

        // Erase matched option from the string before loading another.
        args.erase(0, spacePos);
    }
    // All options loaded, save them to a new char pointer array argv.
    if ((argv = (char**)malloc(argc * sizeof(char*))) == NULL)
    {
        throw std::runtime_error("Memory allocation error.");
    }
    for (int i = 0; i < argc; i++)
    {
        argv[i] = argsVect[i];
    }
}

/// Free all memory used by created argv array.
void _FreeArgv(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}

ArgumentParser::ArgumentParser(std::string args)
{
    // Initialize class attributes to default values.
    ReadMode = WriteMode = Multicast = false;
    Timeout = 0;
    Size = 512;
    AddressStr = "127.0.0.1";
    //AddressStr = "192.168.1.203";
    Domain = AF_INET;
    Port = 69;
    TransferMode = "octet";

    // Load argc and argv for getopt from string.
    int argc; char** argv;
    try
    {
        _ArgsForGetopt(args, argc, argv);
    }
    catch (const std::runtime_error& exc) // Catch possible memory allocation error.
    {
        throw std::invalid_argument(exc.what());
    }
    // Arguments loaded, prepare flags and reset getopt.
    bool destFlag = false, timeoutFlag = false, sizeFlag = false, transModeFlag = false, addrFlag = false;
    int option;
    optind = 0;
    try
    {
        // Parse arguments using getopt.
        while ((option = getopt(argc, argv, "RWd:t:s:mc:a:")) != -1)
        {
            switch (option)
            {
            case 'R':   ParseReadMode();                    break;
            case 'W':   ParseWrite();                       break;
            case 'd':   ParseDestination(destFlag, optarg); break;
            case 't':   ParseTimeout(timeoutFlag, optarg);  break;
            case 's':   ParseSize(sizeFlag, optarg);        break;
            case 'm':   ParseMulticast();                   break;
            case 'c':   ParseMode(transModeFlag, optarg);   break;
            case 'a':   ParseAddress(addrFlag, optarg);     break;
            default:
                throw std::invalid_argument(args);
                break;
            }
        }
        // Check for required -R/-W and -d arguments.
        if (!ReadMode && !WriteMode)
            throw std::invalid_argument("Missing required argument -R (read mode) or -W (write mode).");

        if (!destFlag)
            throw std::invalid_argument("Missing required argument -d <file-path>.");
    }
    catch (const std::invalid_argument&)
    {
        // Exception thrown while parsing an argument, free the memory before rethrowing.
        _FreeArgv(argc, argv);
        throw;
    }
    if (!addrFlag) // Argument -a was not set, create a struct from default (localhost).
    {
        ServerAddress.v4.sin_family = AF_INET;
        ServerAddress.v4.sin_port = htons(Port);
        inet_pton(Domain, AddressStr.c_str(), &ServerAddress.v4.sin_addr);
    }
    _FreeArgv(argc, argv);
}

void ArgumentParser::ParseReadMode()
{
    if (ReadMode || WriteMode)
        throw std::invalid_argument("Argument -W/-R can't be set twice.");

    ReadMode = true;
}

void ArgumentParser::ParseWrite()
{
    if (WriteMode || ReadMode)
        throw std::invalid_argument("Argument -W/-R can't be set twice.");

    WriteMode = true;
}

void ArgumentParser::ParseDestination(bool& destFlag, std::string optionArg)
{
    if (destFlag)
        throw std::invalid_argument("Argument -d is already set to '" + DestinationPath + "'.");

    DestinationPath = optionArg;
    destFlag = true;
}

void ArgumentParser::ParseTimeout(bool& timeoutFlag, std::string optionArg)
{
    if (timeoutFlag)
        throw std::invalid_argument("Argument -t is already set to '" + std::to_string(Timeout) + "'.");
    try
    {
        Timeout = std::stoi(optionArg);
        if (Timeout < 1 || Timeout > 255)
            throw std::exception();
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument("Invalid value for argument -t: " + (std::string)optionArg);
    }
    timeoutFlag = true;
}

void ArgumentParser::ParseSize(bool& sizeFlag, std::string optionArg)
{
    if (sizeFlag)
        throw std::invalid_argument("Argument -s is already set to '" + std::to_string(Size) + "'.");
    try
    {
        Size = std::stoul(optionArg);
        if (Size < 8 || Size > 65464)
            throw std::exception();
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument("Invalid value for argument -s: " + (std::string)optionArg);
    }
    sizeFlag = true;
}

void ArgumentParser::ParseMulticast()
{
    if (Multicast)
        throw std::invalid_argument("Argument -m is already set.");

    Multicast = true;
}

void ArgumentParser::ParseMode(bool& transModeFlag, std::string optionArg)
{
    if (transModeFlag)
        throw std::invalid_argument("Argument -c is already set to '" + TransferMode + "'.");

    if (optionArg == "binary" || optionArg == "octet")
        TransferMode = "octet";

    else if (optionArg == "ascii" || optionArg == "netascii")
        TransferMode = "netascii";

    else throw std::invalid_argument("Invalid value for argument -c: " + (std::string)optionArg);
    transModeFlag = true;
}

// Helper function for parsing IP address argument.
// If address contains a port number after ':' it is extracted and saved in destination parameter.
void _ExtractPort(std::string& address, int& portDest)
{
    int commaPos = address.find_last_of(',');
    if (commaPos != -1)
    {
        auto portStr = address.substr(commaPos + 1);
        try
        {
            portDest = std::stoi(portStr);
            if (portDest <= 0 || portDest > 65535)
                throw std::exception();
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("Invalid port entered: " + portStr);
        }
        address.erase(commaPos);
    }
}

void ArgumentParser::ParseAddress(bool& addressFlag, std::string optionArg)
{
    if (addressFlag)
        throw std::invalid_argument("Argument -a already set to '" + AddressStr + "'.");

    // Extract port part of the address option value to port field.
    _ExtractPort(optionArg, Port);

    // Create and check IP address struct with inet_pton.
    int status;
    char strBuffer[INET6_ADDRSTRLEN];

    status = inet_pton(Domain, optionArg.c_str(), &ServerAddress.v4.sin_addr);
    if (status <= 0)
    {
        status = inet_pton(Domain = AF_INET6, optionArg.c_str(), &ServerAddress.v6.sin6_addr);
        if (status <= 0)
            throw std::invalid_argument("inet_pton: Bad IP address format: " + optionArg);        
    }
    // Based on result AF, fill information for the socket hint structure.
    switch (Domain)
    {
    case AF_INET:
        ServerAddress.v4.sin_family = AF_INET;
        ServerAddress.v4.sin_port = htons(Port);
        inet_ntop(Domain, &ServerAddress.v4.sin_addr, strBuffer, INET_ADDRSTRLEN);
        break;
    case AF_INET6:
        ServerAddress.v6.sin6_family = AF_INET6;
        ServerAddress.v6.sin6_port = htons(Port);
        inet_ntop(Domain, &ServerAddress.v6.sin6_addr, strBuffer, INET6_ADDRSTRLEN);
        break;
    }
    AddressStr = strBuffer;
    addressFlag = true;
}
