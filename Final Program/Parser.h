////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// String parser class for parsing inputs header file                                      \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#ifndef _Parser_H_
#define _Parser_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

class Parser
{
    public:
        Parser();       
        std::vector<std::string> parseInput(const char input[]);
        std::vector<std::string> parseInput(std::string input);

    private:
};

#endif