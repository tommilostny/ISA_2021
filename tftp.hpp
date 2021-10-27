/**
 * @brief TFTP class module.
 * @author Tomáš Milostný (xmilos02)
 */
#pragma once

#include "argumentparser.hpp"

/**
 * @brief Class for TFTP communication controlled by ArgumentParser attributes.
 */
class Tftp
{
    public:
        Tftp(ArgumentParser* args);
        ~Tftp();

        /**
         * @brief Start communication with the TFTP server and perform request.
         * @exception std::runtime_error
         * @exception std::invalid_argument
         * @returns Communication result (true => ok, false => failed).
         */
        bool Transfer();

    private:
        ArgumentParser* Args; //Argument parser object holding required information.
        FILE* Source;         //Open destination file.
        int ClientSocket;     //Socket file descriptor used for communication.

        enum class Opcodes
        {
            RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERROR = 5
        };

        /**
         * @brief Creates a RRQ/WRQ request packet based on Destination and Read/Write mode attrributes from args parameter.
         * @returns Array with packet data ready to be sent via a socket.
         */
        uint8_t* RequestPacket();

        /**
         * @brief Creates a data packet with n bytes from data.
         * @param n Count of bytes of data to be sent.
         * @param data Pointer to start of data array.
         * @returns Array with packet data ready to be sent via a socket. 
         */
        uint8_t* DataPacket(size_t n, void* data);

        /**
         * @brief Creates an ACK packet.
         * @returns Array with packet data ready to be sent via a socket.
         */
        uint8_t* AcknowledgmentPacket();

        /**
         * @brief Creates an error packet with entered message.
         * @returns Array with packet data ready to be sent via a socket.
         */
        uint8_t* ErrorPacket(std::string errorMessage);
};
