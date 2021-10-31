/**
 * @brief TFTP class module.
 * @author Tomáš Milostný (xmilos02)
 */
#pragma once
#include "argumentparser.hpp"

#define OPCODE_RRQ      1U
#define OPCODE_WRQ      2U
#define OPCODE_DATA     3U
#define OPCODE_ACK      4U
#define OPCODE_ERROR    5U

/**
 * @brief Class for TFTP communication controlled by ArgumentParser attributes.
 */
class Tftp
{
    public:
        /**
         * @exception std::invalid_argument
         */
        Tftp(ArgumentParser* args);
        ~Tftp();

        /**
         * @brief Start communication with the TFTP server and perform request.
         * @exception std::runtime_error
         * @returns Communication result (true => ok, false => failed).
         */
        void Transfer();

    private:
        ArgumentParser* Args; //Argument parser object holding required information.
        FILE* Source;         //Open destination file.
        int ClientSocket;     //Socket file descriptor used for communication.
        socklen_t SocketLength;

        /**
         * @brief Creates and sends a RRQ/WRQ request packet based on Destination and Read/Write mode attrributes from args parameter.
         * @returns tsize option (returned from server for Read request), aka total count of data bytes. -1 on error.
         */
        int RequestPacket();

        /**
         * @brief Creates a data packet with n bytes from data.
         * @param n Count of bytes of data to be sent.
         * @param data Pointer to start of data array.
         */
        void DataPacket(char* data);

        /**
         * @brief Creates and sends an ACK packet.
         * @param blockN Block number read from server.
         */
        void AcknowledgmentPacket(uint16_t blockN);

        void ErrorPacket(uint16_t errorCode, std::string message);
};
