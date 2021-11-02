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
        size_t Request();

        /**
         * @brief Creates a data packet with n bytes from data.
         * @param data Pointer to start of data array.
         */
        bool SendDataBlock(size_t totalFileSize);

        void ReceiveData(size_t totalFileSize);

        /**
         * @brief Creates and sends an ACK packet.
         * @param blockN Block number read from server.
         */
        void SendAcknowledgment(uint16_t blockN);

        void SendError(uint16_t errorCode, std::string message);
};
