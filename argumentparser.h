#pragma once
#include <arpa/inet.h>
#include <string>

// Aliases for -c argument options.
enum Mode
{
    BINARY, ASCII
};

// Holds either IPv4 or IPv6 struct value.
union AddressHolder
{
    struct in_addr V4;
    struct in6_addr V6;
};

class ArgumentParser
{
    public: // Getter metods to make attributes read only after being parsed by the constructor.
        bool GetReadMode();
        bool GetWriteMode();
        std::string GetDestinationPath();
        int GetTimeout();
        int GetSize();
        bool GetMulticast();
        enum Mode GetMode();
        union AddressHolder GetAddress();
        int GetAddressVersion();
        int GetPort();
        std::string GetAddressString();
        
        // Parse given program parameters into ArgumentParser class attributes.
        ArgumentParser(std::string args);

    protected:
        bool ReadMode;                  // Argument -R, read mode (required if -W is not set, otherwise forbidden).
        bool WriteMode;                 // Argument -W, write mode (required if -R is not set, otherwise forbidden).
        std::string DestinationPath;    // Argument -d, destination file to (read to)/(write from) (required).
        int Timeout;                    // Argument -t, timeout in seconds.
        int Size;                       // Argument -s, max size of blocks in octets.
        bool Multicast;                 // Argument -m, enables multicast communication.
        enum Mode Mode;                 // Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii".
        union AddressHolder Address;    // Argument -a, IPv4 or IPv6 address.
        int IpVersion;                  // AF_INET or AF_INET6
        int Port;                       // Argument -a after ',' symbol
        std::string AddrStr;            // Address in string form

    private: // Private parsing methods for constructor design and simplification.
        void ParseRead();
        void ParseWrite();
        void ParseDestination(bool& destinationFlag, std::string optionArg);
        void ParseTimeout(bool& timeoutFlag, std::string optionArg);
        void ParseSize(bool& sizeFlag, std::string optionArg);
        void ParseMulticast();
        void ParseMode(bool& modeFlag, std::string optionArg);
        void ParseAddress(bool& addressFlag, std::string optionArg);
};
