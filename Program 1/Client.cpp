////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Program 1: Sockets                                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// This assignment is intended for two purposes:                                           \\
// (1) To exercise use of various socket-related system calls                              \\
// (2) To evaluate dominant overheads of point-to-point communication over 1Gbps networks. \\
// This program will use the client-server model where a client process establishes        \\
// a connection to a server, sends data or requests, and closes the connection while       \\
// the server sends back responses or acknowledgments to the client.                       \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>

// As specified by the assignment: The product of [nbufs] and [bufsize] must be 1500.
const int BUFF_SIZE = 1500;

// Arguments to argv are passed in as follows:
// [port] [repetitions] [nbufs] [bufsize] [server] [type]
// Port will be passed in as last 5 digits of student number
// PORT = 37438
int main(int argc, char *argv[])
{
    // Error check if enough arguments have been passed in
    if (argc < 7)
    {
        std::cout << "Not enough arguments passed in." << std::endl;
        std::cout << "Current argv size = " << argc << " " << "Size must be >=7" << std::endl;
        std::cout << "Use the format: [port] [repetitions] [nbufs] [bufsize] [server] [type]" << std::endl;
        return -1;
    }
    
    // Error check for port
    if ((atoi(argv[1]) < 1023) || (atoi(argv[1]) > 65536))
    {
        std::cout << "Error! PORT = " << argv[1] << std::endl;
        std::cout << " Allowable port range is 1024 to 65536." << std::endl;
        return -1;
    }
    
    // Error check for repetitions
    if(atoi(argv[2]) < 0)
    {
        std::cout << "Error! REPETITION = " << argv[2] << std::endl;
        std:: cout << " The number of repetitions must be a positive integer." << std::endl;
        return -1;
    }
    
    // Error check for nbufs and bufsize product
    if ((atoi(argv[3]) * atoi(argv[4])) != BUFF_SIZE)
    {
        std::cout << "Error! NBUFS = " << argv[3] << " BUFSIZE = " << argv[4] << std::endl;
        std::cout << " nbufs * bufsize must equal 1500." << std::endl;
        return -1;
    }
    
    // Error check for type of transfer
    if (atoi(argv[6]) > 3 || atoi(argv[6]) < 1)
    {
        std::cout << "Error! TYPE = " << argv[6] << std::endl;
        std::cout << " Allowable type is 1, 2, or 3." << std::endl;
        return -1;
    }
    
    int port = atoi(argv[1]);
    int repetitions = atoi(argv[2]);
    int nbufs = atoi(argv[3]);
    int bufsize = atoi(argv[4]);
    const char *server = argv[5];
    int type = atoi(argv[6]);
    
    // Get hostent structure corresponding to server IP/name passed in
    struct hostent *host = gethostbyname(server);
    if (host == NULL)
    {
        std::cout << "Error! HOSTNAME failed" << std::endl;
        return -1;
    }
    
    // Build the socket structure and address of the client for sending
    sockaddr_in sendSockAddr;
    bzero((char *)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa( *(struct in_addr*) (*host->h_addr_list)));
    sendSockAddr.sin_port = htons(port);
    
    // Open the TCP socket
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSd < 0)
    {
        std::cout << "Error! SOCKET failed." << std::endl;
        close(clientSd);
        return -1;
    }

    // Connect to the server with the opened TCP socket, and check if successful
    int returnCode = connect(clientSd, (sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
    if (returnCode < 0)
    {
        std::cout << "Error! CONNECT failed." << std::endl;
        close(clientSd);
        return -1;
    }
    
    // Data buffer for writing to server
    char databuf[nbufs][bufsize];
    
    // Variables for time stamps and calculations
    struct timeval start;
    struct timeval lap;
    struct timeval stop;
    long dataSendTime;
    long roundTripTime;
    
    // Record the start time
    gettimeofday(&start, NULL);
    
    // Data writing to server using specified type of transfer
    for (int count = 0; count < repetitions; count++)
    {
        // Multiple writes:
        // Invokes the write() system call for each data buffer,
        // thus resulting in calling as many write()s as the
        // number of data buffs
        if (type == 1)
        {
            for (int position = 0; position < nbufs; position++)
            {
                write(clientSd, databuf[position], bufsize);
            }
        }
        // writev:
        // Allocates an array of iovec data structures, each having
        // its *iov_base field point to a different data buffer as
        // well as storing the buffer size in its iov_len field; and
        // thereafter calls writev( ) to send all data buffers at once.
        else if (type == 2)
        {
            struct iovec vector[nbufs];
            for (int position = 0; position < nbufs; position++)
            {
                vector[position].iov_base = databuf[position];
                vector[position].iov_len = bufsize;
            }
            writev(clientSd, vector, nbufs);
        }
        // Single write:
        // Allocates an nbufs-sized array of data buffers, and thereafter
        // calls write( ) to send this array, (i.e., all data buffers) at once.
        else
        {
            write(clientSd, databuf, (nbufs * bufsize));
        }
    }
    
    // Record end of transfer time
    gettimeofday(&lap, NULL);
    
    // Reveive the acknowledgment abd quantity from the server for how many
    // time it called read()
    int numReads;
    read(clientSd, &numReads, sizeof(numReads));
    
    // Record end of total round trip time
    gettimeofday(&stop, NULL);

    // Calculate and print out time statistics for the transfer
    dataSendTime = ((lap.tv_sec - start.tv_sec) * 1000000) + (lap.tv_usec - start.tv_usec);
    roundTripTime = ((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec);
    
    // Print format: Test 1: data-sending time = xxx usec, round-trip time = yyy usec, #reads = zzz
    std::cout << "Test " << type << ": ";
    std::cout << "data-sending time = " << dataSendTime << " usec, ";
    std::cout << "round-trip time = " << roundTripTime << " usec, ";
    std::cout << "#reads = " << numReads << std::endl;
    
    // End session and exit
    close(clientSd);
    return 0;
}
