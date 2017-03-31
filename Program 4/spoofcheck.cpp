////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Program 4: Domain Name Service                                                \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// This assignment is to design and code a program that enables a server to check the      \\
// integrity of a client connection by looking for IP address spoofing. Through this       \\
// assignment, we are suppose to to learn how to use functions for accessing the           \\
// Domain Name System (DNS), such as getpeername, gethostbyaddr, and for converting        \\
// addresses, such as inet_ntoa, ntohs, and inet_addr.                                     \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include "Socket.h"

// Global variable
Socket *sock;
int serverPort;

int main(int argc, const char * argv[])
{
    // Check if console input is correct format and port value is in the allowed range
    if((argc == 2) && (atoi(argv[1]) > 1024 || atoi(argv[1]) < 65536))
    {
        // Get server port from input and build new TCP socket
        serverPort = atoi(argv[1]);
        sock = new Socket(serverPort);
    }
    else
    {
        fprintf(stderr, "Number of arguments must be 1! Current number: %d\n", argc );
        return -1;
    }
    
    // Keep accepting new connections
    while(true)
    {
        int clientSD;
        if((clientSD = sock->getServerSocket()) != -1)
        {
            // Create a child(copy) of the connection in the process thread
            if(fork() == 0)
            {
                int clientPort;
                char *clientIP, *clientName;
                
                sockaddr_in newClientSockAddr;
                socklen_t newClientSockAddrSize = sizeof(newClientSockAddr);
                
                // Assign new client's sockaddr to newCLientSockAddr from given clientSD
                getpeername(clientSD, (sockaddr *)&newClientSockAddr, &newClientSockAddrSize);
                
                // Retrive new client's address and port and convert to a readable string/number
                clientIP = inet_ntoa(newClientSockAddr.sin_addr);
                clientPort = ntohs(newClientSockAddr.sin_port);
                
                printf("client addr =  %s port = %d\n", clientIP, clientPort);
                
                struct hostent *newHost;
                // Assign new client's hostent structure to newHost from given client's IP address
                newHost = gethostbyaddr((const void *)&newClientSockAddr.sin_addr, sizeof(unsigned int), AF_INET);
                
                if(!newHost)
                {
                    printf("gethostbyaddr error for the client( %s): 1\na spoofing client\n\n", clientIP);
                }
                else
                {
                    // Print name of the host
                    clientName = newHost->h_name;
                    printf("official hostname: %s\n", clientName);
                    int i;
                    char **clientAlias;
                    // Print all host aliases
                    for(i = 0, clientAlias = newHost->h_aliases; *clientAlias != 0; i++, clientAlias++)
                    {
                        printf("alias: %s\n", *clientAlias);
                    }
                    if(i == 0)
                    {
                        printf("alias: none\n");
                    }
                    // If known address family
                    if(newHost->h_addrtype == AF_INET)
                    {
                        int addressPosition = 0;
                        bool isSpoofing = true;
                        char *retIP;
                        // Assign the initial IP address from the address list
                        in_addr *addressList = (in_addr *)newHost->h_addr_list[addressPosition];
                        // Check all IP addresses in the list
                        while(addressList != NULL)
                        {
                            // Convert IP address to readable number
                            retIP = inet_ntoa(*addressList);
                            printf("ip address: %s ... hit!\n", retIP);
                            // Check if client's IP matches and break
                            if(retIP == clientIP)
                            {
                                printf("an honest client\n\n");
                                isSpoofing = false;
                                break;
                            }
                            addressPosition++;
                        }
                        // Check for spoofing client
                        if(isSpoofing)
                        {
                            printf("a spoofing client\n");
                        }
                    }
                    else
                    {
                        printf("Address type not AF_INET\n");
                    }
                }
                close(clientSD);
            }
            else
            {
                close(clientSD);
            }
        }
    }
    close(serverPort);
    return 0;
}
