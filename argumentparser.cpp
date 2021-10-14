#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include <stdexcept>
#include <vector>
#include <regex>
#include "argumentparser.h"

/// Initialize argv and argc from string for getopt function.
void _ArgsForGetopt(std::string args, int& argc, char**& argv)
{
    // Initialize argc to 1 and first argv item to program name.
    argc = 1;
    auto arg1 = (char*)malloc(13 * sizeof(char));
    if (arg1 == NULL)
        throw std::runtime_error("Unable to allocate memory.");

    // Fill program name array memory, set it as first item of a temporaty vector of loaded arguments.
    strcpy(arg1, "mytftpclient");
    std::vector<char*> argsVect = { arg1 };

    while (!args.empty())
    {
        // Get a substing from start to first whitespace character
        // or port with ',' and whitespaces for IP addresses.
        std::smatch match;
        std::regex_search(args, match, std::regex("\\s+(,\\s*.{1,5})?"));
        auto spacePos = match.position() + match.length();
        auto arg = std::regex_replace(args.substr(0, spacePos), std::regex("\\s"), "");

        if (!arg.empty()) // Loaded an actual option? Skip unnecessary whitespaces.
        {
            // Copy substring to a new character array and save it in the temporary vector.
            auto array = (char*)malloc((arg.length() + 1) * sizeof(char));
            if (array == NULL)
                throw std::runtime_error("Unable to allocate memory.");

            strcpy(array, arg.c_str());
            argsVect.push_back(array);
            argc++;
        }
        if (spacePos == -1) // No more whitespaces found (end of string), make it empty.
        {
            args = "";
            break;
        }
        // Erase matched option from the string before loading another.
        args.erase(0, spacePos);
    }
    // All options loaded, save them to a new char pointer array argv.
    if ((argv = (char**)malloc(argc * sizeof(char*))) == NULL)
        throw std::runtime_error("Unable to allocate memory.");

    for (int i = 0; i < argc; i++)
        argv[i] = argsVect[i];
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
    // Initialize class attributes default values.
    ReadMode = WriteMode = Multicast = false;
    Timeout = 0;
    Size = 512;
    Address = "127.0.0.1";
    IpVersion = 4;
    Port = 69;
    Mode = BINARY;

    // Load argc and argv for getopt from string.
    int argc; char** argv;
    try
    {
        _ArgsForGetopt(args, argc, argv);
    }
    catch (const std::runtime_error& e) // Catch possible memory allocation error.
    {
        throw std::invalid_argument(e.what());
    }
    // Arguments loaded, prepare flags and reset getopt.
    bool destFlag = false, timeoutFlag = false, sizeFlag = false, modeFlag = false, addrFlag = false;
    int option;
    optind = 0;
    try
    {
        // Parse arguments using getopt.
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
    catch (const std::invalid_argument& e)
    {
        // Exception thrown while parsing an argument, free the memory before rethrowing.
        _FreeArgv(argc, argv);
        throw e;
    }
    _FreeArgv(argc, argv);
}

void ArgumentParser::ParseRead()
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

void ArgumentParser::ParseDestination(bool& destinationFlag, std::string optionArg)
{
    if (destinationFlag)
        throw std::invalid_argument("Argument -d is already set to '" + DestinationPath + "'.");

    DestinationPath = optionArg;
    destinationFlag = true;
}

void ArgumentParser::ParseTimeout(bool& timeoutFlag, std::string optionArg)
{
    if (timeoutFlag)
        throw std::invalid_argument("Argument -t is already set to '" + std::to_string(Timeout) + "'.");
    try
    {
        Timeout = std::stoi(optionArg);
        if (Timeout <= 0)
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
        Size = std::stoi(optionArg);
        if (Size < 0)
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

void ArgumentParser::ParseMode(bool& modeFlag, std::string optionArg)
{
    if (modeFlag)
        throw std::invalid_argument("Argument -c is already set to '" + std::to_string(Mode) + "'.");

    if (optionArg == "binary" || optionArg == "octet")
        Mode = BINARY;

    else if (optionArg == "ascii" || optionArg == "netascii")
        Mode = ASCII;

    else throw std::invalid_argument("Invalid value for argument -c: " + (std::string)optionArg);
    modeFlag = true;
}

// Helper function for parsing IP address argument.
// If address contains a port number after ':' it is extracted and saved in destination parameter.
void _ExtractPort(std::string& address, int& portDest)
{
    int colonPos = address.find_last_of(',');
    if (colonPos != -1)
    {
        auto portStr = address.substr(colonPos + 1);
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
        address.erase(colonPos);
    }
}

// Helper function for parsing an IP address argument.
// Checks whether given string is a valid IP version 4 address.
bool _IsIPv4(std::string address)
{
    int partsCount = 0;
    try
    {
        while (!address.empty())
        {
            int dotPosition = address.find('.');
            auto part = address.substr(0, dotPosition);

            int partVal = std::stoi(part);
            if (partVal > 255 || partVal < 0)
                return false;
            partsCount++;
            
            if (dotPosition == -1)
                address = "";
            else
                address.erase(0, dotPosition + 1);
        }
    }
    catch (const std::exception&)
    {
        return false;
    }
    if (partsCount != 4)
        return false;

    return true;
}

// Helper function for parsing an IP address argument.
// Checks whether given string is a valid IP version 6 address.
bool _IsIPv6(std::string address)
{
    return false;
}

void ArgumentParser::ParseAddress(bool& addressFlag, std::string optionArg)
{
    if (addressFlag)
        throw std::invalid_argument("Argument -a already set to '" + Address + "'.");

    _ExtractPort(optionArg, Port);
    IpVersion = _IsIPv4(optionArg) ? 4 : _IsIPv6(optionArg) ? 6 : throw std::invalid_argument("Bad IP address format: " + optionArg);
    Address = optionArg;
    addressFlag = true;
}

bool ArgumentParser::GetReadMode()                  { return ReadMode; }

bool ArgumentParser::GetWriteMode()                 { return WriteMode; }

std::string ArgumentParser::GetDestinationPath()    { return DestinationPath; }

int ArgumentParser::GetTimeout()                    { return Timeout; }

int ArgumentParser::GetSize()                       { return Size; }

bool ArgumentParser::GetMulticast()                 { return Multicast; }

enum Mode ArgumentParser::GetMode()                 { return Mode; }

std::string ArgumentParser::GetAddress()            { return Address; }

int ArgumentParser::GetAddressVersion()             { return IpVersion; }

int ArgumentParser::GetPort()                       { return Port; }
