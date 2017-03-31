////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// The purpose of thi assignment is to design and code a limited client FTP program based  \\
// on the RFC 959 document.                                                                \\
// The limitation of this program is the limited function/command option in comparison to  \\
// the full FTP program specified by the RFC 959 document. This implementation is limited  \\
// to following commands/functions:                                                        \\
// open [IP] [port #] - Establish a TCP connection to the specified IP and port            \\
// name: [account ID] - Send user ID to server                                             \\
// password: [user password] - Send user password to server                                \\
// ls - Request from the server a full list of the contents of currect working directory   \\
// cd [dir pathname] - Change the current working directory to the pathname specified one  \\
// get [file] - Request a file from the server in the current working directory            \\
// put [file] - Store a file on the server in the current working directory                \\
// help - Requests help from the server                                                    \\
// close - Closes the connection to the server, but does not terminate the FTP program     \\
// quit - Closes the connection to the server and terminate the FTP program                \\
//\\                                                                                       \\
//\\                                                                                       \\
// The program can be run with the following commands:                                     \\
// ./ftp     *Only runs the program without the connection                                 \\
// ./ftp [domain ID/IP]    *Runs the program and connects to given IP with default port 21 \\
// .ftp [domain ID/IP] [port#]    *Runs the program and connects to given IP and port #    \\
//\\                                                                                       \\
//\\                                                                                       \\
// The driver contains the main and is responsible for taking the user input while         \\
// while displaying 'ftp>', parsing the commands and passing them to the commander.        \\
// The main loops until the user uses the quit command.                                    \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include "Commander.h"
#include "Parser.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>

int main(int argc, char *argv[1])
{
    // Variable initializers
    char *serverName = NULL;
    int portNum = DEFAULT_PORT;
    char input[512];
    Commander commands;
    Parser stringCommander;
    std::vector<std::string> instruction;
    // Check starting arguments for domain ID/IP and port #
    if((argc >= 2) && (argc <= 3))
    {
        serverName = argv[1];
        if(argc == 2)
        {
            commands.open(serverName, portNum);
        }
        else
        {
            portNum = atoi(argv[2]);
            if(portNum <= 0)
            {
                fprintf(stderr, "host-name [port]\n");
            }
            commands.open(serverName, portNum);
        }
    }
    else
    {
        fprintf(stderr, "host-name [port]\n");
    }
    // Keep looping and taking in user commands
    do
    {
        printf("ftp> ");
        std::cin.getline(input,512);
        instruction = stringCommander.parseInput(input);
        commands.parseCommand(instruction);
        
    }
    while (commands.isActive());
    return 0;
}