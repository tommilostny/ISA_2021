#pragma once
#include <string>

// Aliases for -c argument options.
enum Mode
{
    BINARY, ASCII
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
        std::string GetAddress();
        int GetAddressVersion();
        int GetPort();
        
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
        std::string Address;            // Argument -a, IPv4 or IPv6 address.
        int IpVersion;                  // 4 or 6 for valid Address.
        int Port;

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
