#pragma once
/**
 * @brief Argument parser class module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <arpa/inet.h>
#include <string>

/// Holds either IPv4 or IPv6 struct value.
typedef union socket_hint
{
    struct sockaddr_in  v4;
    struct sockaddr_in6 v6;
}
SocketHint;

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
        bool          read_mode;        // Argument -R, read mode (required if -W is not set, otherwise forbidden).
        bool          write_mode;       // Argument -W, write mode (required if -R is not set, otherwise forbidden).
        std::string   destination_path; // Argument -d, destination file to (read to)/(write from) (required).
        int           timeout;          // Argument -t, timeout in seconds.
        int           size;             // Argument -s, max size of blocks in octets.
        bool          multicast;        // Argument -m, enables multicast communication.
        std::string   transfer_mode;    // Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii".
        SocketHint    socket_hint;      // Parsed hint structure from argument -a, IPv4 or IPv6 address.
        int           domain;           // AF_INET or AF_INET6
        int           port;             // Argument -a after ',' symbol
        std::string   addr_str;         // Address in string form

    private: // Private parsing methods for constructor design and simplification.
        void parse_read();
        void parse_write();
        void parse_destination(bool& destination_flag, std::string option_arg);
        void parse_timeout(bool& timeout_flag, std::string option_arg);
        void parse_size(bool& size_flag, std::string option_arg);
        void parse_multicast();
        void parse_mode(bool& mode_flag, std::string option_arg);
        void parse_address(bool& address_flag, std::string option_arg);
};
