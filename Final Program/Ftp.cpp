////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// The Parser class interacts with the system which includes inputs and outputs to the     \\
// TCP/IP socket and disk (for file reading/storage). The interaction is in the form of    \\
// particular function invocation by the Commander class depending on the user commands.   \\
// This class also handles TCP/IP connection opening and closing.                          \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include "Ftp.h"

// Constructor with initializers, including default port 21
Ftp::Ftp()
{
    hostPort = DEFAULT_PORT;  // port 21
    clientSD = NULL_SD;
    clientFD = NULL_SD;
    passivevSD = NULL_SD;
    active = false;
}

/* Destructor
** Needed to properly close connections in case the program is improperly terminated
*/
Ftp::~Ftp()
{
    if (clientFD != NULL_SD)
    {
        close(clientSD);
    }
    else if (passivevSD != NULL_SD)
    {
        close(passivevSD);
    }
    else if (clientSD != NULL_SD)
    {
        close(clientFD);
    }
}

/* createSocket
** Creates and opens a TCP connection (sequence as specified by previous assignments)
*/
int Ftp::createSocket(const char serverName[], int pornNum)
{
    struct hostent *server = gethostbyname(serverName);
    if (server == NULL)
    {
        fprintf(stderr, "%s: No address associated with hostname!\n", serverName);
        return NULL_SD;
    }
    bzero((char *)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) (*server->h_addr_list)));
    sendSockAddr.sin_port = htons(pornNum);
    int newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket < 0)
    {
        fprintf(stderr, "Socket failed!\n");
        close(newSocket);
        return NULL_SD;
    }
    int returnCode = connect(newSocket, (sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
    if (returnCode < 0)
    {
        fprintf(stderr, "Connection denied!\n");
        close(newSocket);
        return NULL_SD;
    }
    return newSocket;
}

/* closeSocket
** Disconnects from the specified socket
*/
void Ftp::closeSocket(int socketSD)
{
    shutdown(socketSD, SHUT_WR);
    close(socketSD);
    if (socketSD == clientSD)
    {
        clientSD = NULL_SD;
        active = false;
    }
    else
    {
        passivevSD = NULL_SD;
    }
}

/* openSocket
** Opens a connection to the specified server and port # for command exchange
*/
int Ftp::openSocket(const char server[], int portNum)
{
    // Call to the function above for full socket creating and opening
    clientSD = createSocket(server, portNum);
    if (clientSD == NULL_SD)
    {
        return NULL_SD;
    }
    active = true;
    serverName = server;
    return clientSD;
}

/* openPassive
** Opens a (passive) connection to the specified server and port # for data exchange
*/
int Ftp::openPassive(int portNum)
{
    passivevSD = createSocket((const char *)serverName.c_str(), portNum);
    if (passivevSD == NULL_SD)
    {
        return NULL_SD;
    }
    return passivevSD;
}

/* readyFile
** Opens and prepares a file with the read only mode in the local system location
** Used for sending files with 'put' command
*/
int Ftp::readyFile(const char fileName[])
{
    clientFD = open(fileName, O_RDONLY);
    return clientFD;
}

/* sendFile
 ** Readies a local file to send to remote location
 ** Used for sending files with 'put' command
 */
int Ftp::sendFile()
{
    int size = lseek(clientFD, 0, SEEK_END);
    lseek(clientFD, 0, 0);
    char *sendBuffer = new char[size];
    read(clientFD, sendBuffer, size);
    int sentValue = write(passivevSD, sendBuffer, size);
    delete[] sendBuffer;
    sendBuffer = NULL;
    return sentValue;
}

/* readyFile
** Creates and prepares a new file with the specified modes in the local system location
** Used for receiving files with 'get' command
*/
int Ftp::makeFile(const char fileName[])
{
    // File modes for the file received from the server
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    clientFD = open(fileName, O_WRONLY | O_CREAT, mode);
    
    return clientFD;
}

/* closeFile
** Closes the file
*/
void Ftp::closeFile()
{
    close(clientFD);
    clientFD = NULL_SD;
}

/* sendCommand
** Sends a command to the server as specified by RFC 959
*/
int Ftp::sendCommand(const std::string command) const
{
    int messSize = (command.length() + 2);
    char commandBuffer[messSize];
    bzero(commandBuffer, sizeof(commandBuffer));
    strcpy(commandBuffer, command.c_str());
    strcat(commandBuffer, "\r\n");
    return write(clientSD, commandBuffer, messSize);
}

/* receiveMessage
** Receive a a message from the server in a single ASCII line format
*/
int Ftp::receiveMessage()
{
    char buffer[8192];
    bzero(buffer, sizeof(buffer));
    int length = read(clientSD, buffer, sizeof(buffer));
    replyMessage = buffer;
    return length;
}

/* receiveStream
** Receive a a message from the server in an ASCII multi-line format
*/
int Ftp::receiveStream(int socketSD)
{
    // Polling socket for data multi-stream data receiving
    struct pollfd ufd;
    ufd.fd = socketSD;  // Specifies what socket to receive data from
    ufd.events = POLLIN;
    ufd.revents = 0;
    char streamBuffer[8192];
    asciiStream.clear();
    int messSize = 0;
    int totalSize = 0;
    // Poll for 1000 ms
    while (poll(&ufd, 1, 1000) > 0)
    {
        bzero(streamBuffer, sizeof(streamBuffer));
        messSize = read(socketSD, streamBuffer, sizeof(streamBuffer));
        if (messSize == 0)
        {
            break;
        }
        asciiStream.append(streamBuffer);
        totalSize += messSize;
    }
    return totalSize;
}

/* receiveFile
** Receives a file from the server and writes to the local disk
** Used for receiving files with 'get' command
*/
int Ftp::receiveFile()
{
    // Polling socket for data multi-stream data receiving
    struct pollfd ufd;
    ufd.fd = passivevSD;     // Specifies what socket to receive data from
    ufd.events = POLLIN;
    ufd.revents = 0;
    char recBuffer[8192];
    int receivedNum = 0;
    int totalSize = 0;
    // Poll for 1000 ms, and receive data in blocks
    while (poll(&ufd, 1, 1000) > 0)
    {
        bzero(recBuffer, sizeof(recBuffer));
        receivedNum = read(passivevSD, recBuffer, sizeof(recBuffer));
        if (receivedNum == 0)
        {
            break;
        }
        lseek(clientFD, 0, SEEK_END);
        write(clientFD, recBuffer, receivedNum);
        totalSize += receivedNum;
    }
    return totalSize;
}

/* getMessage
** Returns the string of the reply message received from the server
*/
std::string Ftp::getMessage() const
{
    return replyMessage;
}

/* getAscii
** Returns the string of the ASCII stream received from the server
*/
std::string Ftp::getAscii() const
{
    return asciiStream;
}

/* getReply
** Returns the number representation of the reply code extracted from the replyMessage message received from the server
*/
int Ftp::getReply() const
{
    if (replyMessage.length() > 0)
    {
        Parser token;
        std::vector<std::string> reply;
        reply = token.parseInput(replyMessage);
        return atoi(reply[0].c_str());
    }
    return -1;
}

/* getStream
** Returns the number representation of the reply code extracted from the ASCII stream received from the server
*/
int Ftp::getStream() const
{
    if (asciiStream.length() > 0)
    {
        Parser token;
        std::vector<std::string> reply;
        reply = token.parseInput(asciiStream);
        return atoi(reply[0].c_str());
    }
    return -1;
}

/* getMessage
** Returns the string of the server
*/
std::string Ftp::getServerName() const
{
    return serverName;
}

/* getClientSD
** Returns the value of the command connection socket
*/
int Ftp::getClientSD() const
{
    return clientSD;
}

/* getPassiveSD
** Returns the value of the (passive) data connection socket
*/
int Ftp::getPassiveSD() const
{
    return passivevSD;
}

/* isActive
** Returns if the FTP connection is opened or closed (i.e. isActive)
*/
bool Ftp::isActive() const
{
    return active;
}