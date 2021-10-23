#pragma once
/**
 * @brief Argument parser class module.
 * @author Tomáš Milostný (xmilos02)
 */
#include <arpa/inet.h>
#include <string>

/// Holds either IPv4 or IPv6 struct value.
typedef union address_holder
{
    struct in_addr  v4;
    struct in6_addr v6;
}
AddressHolder;

/**
 * @brief TFTP client argument parsing class.
 * @exception std::invalid_argument
 */
class ArgumentParser
{
    public: // Getter methods to make attributes read only after being parsed by the constructor.
        bool          get_read_mode();
        bool          get_write_mode();
        std::string   get_destination_path();
        int           get_timeout();
        int           get_size();
        bool          get_multicast();
        std::string   get_transfer_mode();
        AddressHolder get_address();
        int           get_address_version();
        int           get_port();
        std::string   get_address_string();
        
        // Parse given program parameters into ArgumentParser class attributes.
        ArgumentParser(std::string args);

    protected: // Fields for application arguments.
        bool          _read_mode;        // Argument -R, read mode (required if -W is not set, otherwise forbidden).
        bool          _write_mode;       // Argument -W, write mode (required if -R is not set, otherwise forbidden).
        std::string   _destination_path; // Argument -d, destination file to (read to)/(write from) (required).
        int           _timeout;          // Argument -t, timeout in seconds.
        int           _size;             // Argument -s, max size of blocks in octets.
        bool          _multicast;        // Argument -m, enables multicast communication.
        std::string   _transfer_mode;    // Argument -c, mode decoded from "binary"/"octet" and "ascii"/"netascii".
        AddressHolder _address;          // Argument -a, IPv4 or IPv6 address.
        int           _ip_version;       // AF_INET or AF_INET6
        int           _port;             // Argument -a after ',' symbol
        std::string   _addr_str;         // Address in string form

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
