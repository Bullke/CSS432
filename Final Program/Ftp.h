////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// FTP class for handling FTP interaction header file                                      \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#ifndef _FTP_H_
#define _FTP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/poll.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include "Parser.h"

#define DEFAULT_PORT 21
#define NULL_SD -1

class Ftp
{
    public:
        Ftp();
        ~Ftp();
        void closeSocket(int socketSD);
        int openSocket(const char serverName[], int portNum);
        int openPassive(int portNum);
        int sendCommand(const std::string message) const;
        int readyFile(const char fileName[]);
        int sendFile();
        int makeFile(const char fileName[]);
        void closeFile();
        int receiveMessage();
        int receiveStream(int socketSD);
        int receiveFile();
        std::string getMessage() const;
        std::string getAscii() const;
        int getReply() const;
        int getStream() const;
        std::string getServerName() const;
        int getClientSD() const;
        int getPassiveSD() const;
        bool isActive() const;
        
    private:
        int createSocket(const char server[], int destPort);
        std::string serverName, replyMessage, asciiStream;
        int hostPort, clientSD, clientFD, passivevSD;
        bool active;
        struct sockaddr_in sendSockAddr;
};

#endif