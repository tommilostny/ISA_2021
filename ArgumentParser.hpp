/**
 * @brief Argument parser class module.
 * @author Tomáš Milostný (xmilos02)
 */
#pragma once
#include <arpa/inet.h>
#include <string>

/// Holds either IPv4 or IPv6 struct value.
union ServerAddress
{
    struct sockaddr_in  v4;
    struct sockaddr_in6 v6;
};

/**
 * @brief TFTP client argument parsing class.
 * @exception std::invalid_argument
 */
class ArgumentParser
{
    public:        
        // Parse given program parameters into ArgumentParser class attributes.
        ArgumentParser(std::string args);

        // Fields for application arguments.
        bool             ReadMode;        // Argument -R, read mode (required if -W is not set, otherwise forbidden).
        bool             WriteMode;       // Argument -W, write mode (required if -R is not set, otherwise forbidden).
        std::string      DestinationPath; // Argument -d, destination file to (read to)/(write from) (required).
        int              Timeout;         // Argument -t, timeout in seconds.
        size_t           Size;            // Argument -s, max size of blocks in octets.
        bool             Multicast;       // Argument -m, enables multicast communication.
        std::string      TransferMode;    // Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii".
        union ServerAddress ServerAddress;// Parsed hint structure from argument -a, IPv4 or IPv6 address.
        int              Domain;          // AF_INET or AF_INET6
        int              Port;            // Argument -a after ',' symbol
        std::string      AddressStr;      // Address in string form

        bool             ExitFlag;        // Exit or quit command flag.
        bool             HelpFlag;        // Help command flag.

    private: // Private parsing methods for constructor design and simplification.
        void ParseReadMode();
        void ParseWrite();
        void ParseDestination(bool& destinationFlag, std::string optionArg);
        void ParseTimeout(bool& timeoutFlag, std::string optionArg);
        void ParseSize(bool& sizeFlag, std::string optionArg);
        void ParseMulticast();
        void ParseMode(bool& modeFlag, std::string optionArg);
        void ParseAddress(bool& addressFlag, std::string optionArg);

        void DisplayHelp();
};
