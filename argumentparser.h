#pragma once
#include <string>

enum Mode
{
    BINARY, ASCII
};

class ArgumentParser
{
    public:
        ArgumentParser(int argc, char **argv);
        bool GetReadMode();
        bool GetWriteMode();
        std::string GetDestinationPath();
        int GetTimeout();
        int GetSize();
        bool GetMulticast();
        enum Mode GetMode();
        std::string GetAddress();

    protected:
        bool ReadMode;                  /** Argument -R (required if -W is not set, otherwise forbidden) */
        bool WriteMode;                 /** Argument -W (required if -R is not set, otherwise forbidden) */
        std::string DestinationPath;    /** Argument -d (required) */ 
        int Timeout;                    /** Argument -t */
        int Size;                       /** Argument -s, max size of blocks in octets */
        bool Multicast;                 /** Argument -m, enables multicast communication */
        enum Mode Mode;                 /** Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii" */
        std::string Address;            /** Argument -a, IPv4 or IPv6 address */

    private:
        void ParseRead();
        void ParseWrite();
        void ParseDestination(bool& destinationFlag, char* optarg);
        void ParseTimeout(bool& timeoutFlag, char* optarg);
        void ParseSize(bool& sizeFlag, char* optarg);
        void ParseMulticast();
        void ParseMode(bool& modeFlag, char* optarg);
        void ParseAddress(bool& addressFlag, char* optarg);
};
