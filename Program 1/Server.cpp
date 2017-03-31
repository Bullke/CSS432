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
#include <fcntl.h>
#include <signal.h>

// As specified by the assignment: The product of [nbufs] and [bufsize] must be 1500.
const int BUFF_SIZE = 1500;
// Maximum number of connections server is listening to
const int MAX_CONNECTIONS = 5;

// Initialization of variables global variables
int port;
int repetitions;
int serverSd;
int newSd;

// Interrupt handler:
//  It will be called upon an I/O interrupt (a.k.a. a signal in Unix).
// This functioin receives just an integer argument and returns void.
// The argument is the signal identifier, (i.e., SIGIO) which you
// don't have to take care of in the function.
void interruptHandler (int signal)
{
    // Data buffer for receiving data from client
    char dataBuff[BUFF_SIZE];
    
    // Variables for time stamps and calculations
    struct timeval start;
    struct timeval stop;
    long dataReceivingTime;
    
    // Record the start time
    gettimeofday(&start, NULL);
    
    // Repeating the read() function for reading from the client and
    // keeping track of the number or total reads.
    int count = 0;
    for (int i = 0; i < repetitions; i++)
    {
        for (int nRead = 0; ( nRead += read( newSd, dataBuff, BUFF_SIZE - nRead ) ) < BUFF_SIZE ; ++count );
         
        /*
        int nRead = 0;
        while (nRead < BUFF_SIZE)
        {
            nRead += read(newSd, dataBuff, BUFF_SIZE - nRead);
            count++;
            
        }
         */
    }
    
    // Record end of total data receiving time
    gettimeofday(&stop, NULL);
    
    // Send the number of read( ) calls made, (i.e., count in the above) as an acknowledgment.
    write(newSd, &count, sizeof(count));
    
    // Calculate and print out time statistics for the transfer
    // Print format: data-receiving time = xxx usec
    dataReceivingTime = ((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec);
    std::cout << "data-receiving time = " << dataReceivingTime << " usec" << std::endl;
    
    // End session and exit
    close(newSd);
    close(serverSd);
    exit(0);
}

// Arguments to argv are passed in as follows:
// [port] [repetitions]
// Port will be passed in as last 5 digits of student number
// PORT = 37438
int main(int argc, char *argv[])
{
    // Error check if enough arguments have been passed in
    if (argc < 3)
    {
        std::cout << "Not enough arguments passed in." << std::endl;
        std::cout << "Current argv size = " << argc << " " << "Size must be >=7" << std::endl;
        std::cout << "Use the format: [port] [repetition]" << std::endl;
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
    
    port = atoi(argv[1]);
    repetitions = atoi(argv[2]);
    
    // Build the receiving socket structure and address of the server for receiving
    sockaddr_in acceptSockAddr;
    bzero((char *)&acceptSockAddr, sizeof(acceptSockAddr));
    acceptSockAddr.sin_family = AF_INET;
    acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSockAddr.sin_port = htons(port);
    
    // Open the TCP socket
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    
    // Bind the TCP to the its local address
    int returnCode = bind(serverSd, (sockaddr *)&acceptSockAddr, sizeof(acceptSockAddr));
    if (returnCode < 0)
    {
        std::cout << "Error! BIND failed." << std::endl;
        close(serverSd);
        return -1;
    }
    
    // Listen for specified number of connections
    listen(serverSd, MAX_CONNECTIONS);
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    
    // Wait to receive a request from client and get new socket
    newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    
    // Wait for the I/O interrupt signal and change the new socket into an asynchronous connection
    signal(SIGIO, interruptHandler);
    fcntl(newSd, F_SETOWN, getpid());
    fcntl(newSd, F_SETFL, FASYNC);
    
    // Put the server to sleep forever
    while (true)
    {
        sleep(1000);
    }
    
    return 0;
}
