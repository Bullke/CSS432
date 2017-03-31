////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Commander class for handling user input and commands                                    \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#ifndef _Commander_H_
#define _Commander_H_

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iomanip>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "Ftp.h"
#include "Parser.h"
#include <sstream>
#include <sys/time.h>

#define DEFAULT_PORT 21

// Different commands
enum typeCommand
{
    UNDEFINED,
    OPEN,
    CD,
    PWD,
    SYST,
    LS,
    GET,
    PUT,
    DEL,
    CLOSE,
    QUIT,
    HELP
};

class Commander
{
    public:
        Commander();
    
        bool parseCommand(std::vector<std::string> commands);
        bool open(std::vector<std::string> commands);
        bool open(const char *hostName, int hostPort);
        bool cd(std::vector<std::string> commands);
        bool get(std::vector<std::string> commands);
        bool put(std::vector<std::string> commands);
        bool del(std::vector<std::string> commands);
        bool pwd();
        bool syst();
        bool ls();
        bool help();
        bool close();
        bool quit();
        bool isActive() const;

    private:
        std::map<std::string, typeCommand> commandMap;
        Ftp ftp;
        Parser stringParser;
        bool active;
        bool passiveConnection();
};

#endif