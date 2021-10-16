/**
 * @brief Argument parser class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include <stdexcept>
#include <vector>
#include <regex>
#include "argumentparser.h"

/// Initialize argv and argc from string for getopt function.
void _args_for_getopt(std::string args, int& argc, char**& argv)
{
    // Initialize argc to 1 and first argv item to program name.
    argc = 1;
    auto prog_arg = (char*)malloc(13 * sizeof(char));
    if (prog_arg == NULL)
        throw std::runtime_error("Unable to allocate memory.");

    // Fill program name array memory, set it as first item of a temporaty vector of loaded arguments.
    strcpy(prog_arg, "mytftpclient");
    std::vector<char*> args_vect = { prog_arg };

    while (!args.empty())
    {
        // Get a substing from start to first whitespace character
        // or port with ',' and whitespaces for IP addresses.
        std::smatch match;
        std::regex_search(args, match, std::regex("\\s+(,\\s*.{1,5})?"));
        auto space_pos = match.position() + match.length();
        auto arg = std::regex_replace(args.substr(0, space_pos), std::regex("\\s"), "");

        if (!arg.empty()) // Loaded an actual option? Skip unnecessary whitespaces.
        {
            // Copy substring to a new character array and save it in the temporary vector.
            auto arg_array = (char*)malloc((arg.length() + 1) * sizeof(char));
            if (arg_array == NULL)
                throw std::runtime_error("Unable to allocate memory.");

            strcpy(arg_array, arg.c_str());
            args_vect.push_back(arg_array);
            argc++;
        }
        if (space_pos == -1) // No more whitespaces found (end of string), make it empty.
        {
            args = "";
            break;
        }
        // Erase matched option from the string before loading another.
        args.erase(0, space_pos);
    }
    // All options loaded, save them to a new char pointer array argv.
    if ((argv = (char**)malloc(argc * sizeof(char*))) == NULL)
        throw std::runtime_error("Unable to allocate memory.");

    for (int i = 0; i < argc; i++)
        argv[i] = args_vect[i];
}

/// Free all memory used by created argv array.
void _free_argv(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}

ArgumentParser::ArgumentParser(std::string args)
{
    // Initialize class attributes default values.
    _read_mode = _write_mode = _multicast = false;
    _timeout = 0;
    _size = 512;
    _addr_str = "127.0.0.1";
    _ip_version = AF_INET;
    _port = 69;
    _transfer_mode = BINARY;

    // Load argc and argv for getopt from string.
    int argc; char** argv;
    try
    {
        _args_for_getopt(args, argc, argv);
    }
    catch (const std::runtime_error& e) // Catch possible memory allocation error.
    {
        throw std::invalid_argument(e.what());
    }
    // Arguments loaded, prepare flags and reset getopt.
    bool dest_flag = false, timeout_flag = false, size_flag = false, mode_flag = false, addr_flag = false;
    int option;
    optind = 0;
    try
    {
        // Parse arguments using getopt.
        while ((option = getopt(argc, argv, "RWd:t:s:mc:a:")) != -1)
        {
            switch (option)
            {
            case 'R':   parse_read();                           break;
            case 'W':   parse_write();                          break;
            case 'd':   parse_destination(dest_flag, optarg);   break;
            case 't':   parse_timeout(timeout_flag, optarg);    break;
            case 's':   parse_size(size_flag, optarg);          break;
            case 'm':   parse_multicast();                      break;
            case 'c':   parse_mode(mode_flag, optarg);          break;
            case 'a':   parse_address(addr_flag, optarg);       break;
            default:
                throw std::invalid_argument(args);
                break;
            }
        }
        // Check for required -R/-W and -d arguments.
        if (!_read_mode && !_write_mode)
            throw std::invalid_argument("Missing required argument -R (read mode) or -W (write mode).");

        if (!dest_flag)
            throw std::invalid_argument("Missing required argument -d <file-path>.");
    }
    catch (const std::invalid_argument& e)
    {
        // Exception thrown while parsing an argument, free the memory before rethrowing.
        _free_argv(argc, argv);
        throw e;
    }
    if (!addr_flag) // Argument -a was not set, create a struct from default (localhost).
    {
        unsigned char buffer[sizeof(struct in_addr)];
        inet_pton(_ip_version, _addr_str.c_str(), buffer);
        memcpy(&_address, buffer, sizeof(struct in_addr));
    }
    _free_argv(argc, argv);
}

void ArgumentParser::parse_read()
{
    if (_read_mode || _write_mode)
        throw std::invalid_argument("Argument -W/-R can't be set twice.");

    _read_mode = true;
}

void ArgumentParser::parse_write()
{
    if (_write_mode || _read_mode)
        throw std::invalid_argument("Argument -W/-R can't be set twice.");

    _write_mode = true;
}

void ArgumentParser::parse_destination(bool& dest_flag, std::string option_arg)
{
    if (dest_flag)
        throw std::invalid_argument("Argument -d is already set to '" + _destination_path + "'.");

    _destination_path = option_arg;
    dest_flag = true;
}

void ArgumentParser::parse_timeout(bool& timeout_flag, std::string option_arg)
{
    if (timeout_flag)
        throw std::invalid_argument("Argument -t is already set to '" + std::to_string(_timeout) + "'.");
    try
    {
        _timeout = std::stoi(option_arg);
        if (_timeout <= 0)
            throw std::exception();
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument("Invalid value for argument -t: " + (std::string)option_arg);
    }
    timeout_flag = true;
}

void ArgumentParser::parse_size(bool& size_flag, std::string option_arg)
{
    if (size_flag)
        throw std::invalid_argument("Argument -s is already set to '" + std::to_string(_size) + "'.");
    try
    {
        _size = std::stoi(option_arg);
        if (_size < 0)
            throw std::exception();
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument("Invalid value for argument -s: " + (std::string)option_arg);
    }
    size_flag = true;
}

void ArgumentParser::parse_multicast()
{
    if (_multicast)
        throw std::invalid_argument("Argument -m is already set.");

    _multicast = true;
}

void ArgumentParser::parse_mode(bool& mode_flag, std::string option_arg)
{
    if (mode_flag)
        throw std::invalid_argument("Argument -c is already set to '" + std::to_string(_transfer_mode) + "'.");

    if (option_arg == "binary" || option_arg == "octet")
        _transfer_mode = BINARY;

    else if (option_arg == "ascii" || option_arg == "netascii")
        _transfer_mode = ASCII;

    else throw std::invalid_argument("Invalid value for argument -c: " + (std::string)option_arg);
    mode_flag = true;
}

// Helper function for parsing IP address argument.
// If address contains a port number after ':' it is extracted and saved in destination parameter.
void _extract_port(std::string& address, int& port_dest)
{
    int comma_pos = address.find_last_of(',');
    if (comma_pos != -1)
    {
        auto port_str = address.substr(comma_pos + 1);
        try
        {
            port_dest = std::stoi(port_str);
            if (port_dest <= 0 || port_dest > 65535)
                throw std::exception();
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("Invalid port entered: " + port_str);
        }
        address.erase(comma_pos);
    }
}

void ArgumentParser::parse_address(bool& address_flag, std::string option_arg)
{
    if (address_flag)
        throw std::invalid_argument("Argument -a already set to '" + _addr_str + "'.");

    // Extract port part of the address option value to _port field.
    _extract_port(option_arg, _port);

    // Create and check IP address struct, based on "man inet_pton" example.
    unsigned char buffer[sizeof(struct in6_addr)];
    int status;
    char str[INET6_ADDRSTRLEN];

    status = inet_pton(_ip_version, option_arg.c_str(), buffer);
    if (status <= 0)
    {
        status = inet_pton(_ip_version = AF_INET6, option_arg.c_str(), buffer);
        if (status <= 0)
            throw std::invalid_argument("inet_pton: Bad IP address format: " + option_arg);
        
        memcpy(&_address, buffer, sizeof(struct in6_addr));
    }
    else memcpy(&_address, buffer, sizeof(struct in_addr));

    if (inet_ntop(_ip_version, buffer, str, INET6_ADDRSTRLEN) == NULL) {
        throw std::invalid_argument("inet_ntop: Bad IP address format: " + option_arg);
    }
    _addr_str = str;
    address_flag = true;
}

bool ArgumentParser::get_read_mode()               { return _read_mode; }

bool ArgumentParser::get_write_mode()              { return _write_mode; }

std::string ArgumentParser::get_destination_path() { return _destination_path; }

int ArgumentParser::get_timeout()                  { return _timeout; }

int ArgumentParser::get_size()                     { return _size; }

bool ArgumentParser::get_multicast()               { return _multicast; }

TransferMode ArgumentParser::get_transfer_mode()   { return _transfer_mode; }

AddressHolder ArgumentParser::get_address()        { return _address; }

int ArgumentParser::get_address_version()          { return _ip_version; }

int ArgumentParser::get_port()                     { return _port; }

std::string ArgumentParser::get_address_string()   { return _addr_str; }
