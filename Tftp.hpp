/**
 * @brief TFTP class module.
 * @author Tomáš Milostný (xmilos02)
 */
#pragma once
#include "ArgumentParser.hpp"

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
         */
        void Transfer();

    private:
        ArgumentParser* Args; //Argument parser object holding required information.
        FILE* DestinationFile;         //Open destination file.
        int ClientSocket;     //Socket file descriptor used for communication.
        socklen_t SocketLength;

        /**
         * @brief Creates and sends a RRQ/WRQ request packet based on Destination and Read/Write mode attrributes from args parameter.
         * @returns tsize option (returned from server for Read request), aka total count of data bytes. -1 on error.
         */
        size_t Request();

        void SendData(size_t totalFileSize);

        void ReceiveData(size_t totalFileSize);

        void SendAcknowledgment(uint16_t blockN);
};
