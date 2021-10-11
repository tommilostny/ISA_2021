#pragma once

#include <string>

enum Mode
{
    BINARY, ASCII
};

class ArgumentParser
{
    public:
        /**
         * @brief Argument parser constructor, processes program arguments.
         */
        ArgumentParser(int argc, char **argv);

        bool ReadMode;                  /** Argument -R (required if -W is not set, otherwise forbidden) */
        bool WriteMode;                 /** Argument -W (required if -R is not set, otherwise forbidden) */

        bool DestinationSet;            /** Is argument -d set and DestinationPath valid? */
        std::string DestinationPath;    /** Argument -d (required) */ 

        bool TimeoutSet;                /** Is argument -t set and Timeout valid? */
        int Timeout;                    /** Argument -t */

        bool SizeSet;                   /** Is argument -s set and Size valid? */
        int Size;                       /** Argument -s, max size of blocks in octets */

        bool Multicast;                 /** Argument -m, enables multicast communication */
        
        bool ModeSet;                   /** Is argument -c set and Mode valid? */
        enum Mode Mode;                 /** Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii" */

        bool AddressSet;                /** Is argument -a set and Address valid? */
        std::string Address;            /** Argument -a, IPv4 or IPv6 address */

    private:
        void ParseRead();
        void ParseWrite();
        void ParseDestination(char* optarg);
        void ParseTimeout(char* optarg);
        void ParseSize(char* optarg);
        void ParseMulticast();
        void ParseMode(char* optarg);
        void ParseAddress(char* optarg);
};
