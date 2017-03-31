////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// The Parser class has the responsibility to parse user inputs by taking in a line of     \\
// characters, i.e. a character array, isolating words from it separated by space,         \\
// then pushing them into a vector of strings which is then passed to Commander class      \\
// that deals with the user commands and input and by correctly communicating with the FTP \\
// server. Additionally, the Parser class can also isolate FTP server's RFC 959 reply codes\\
// in order to help Commander and FTP class the status of the connection/communication.    \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include "Parser.h"

Parser::Parser()
{}

/* parseInput
** Parses the user input into a vector of strings representing commands and input to commands
** Returns a vector of commands/input
*/
std::vector<std::string> Parser::parseInput(const char input[])
{
    std::vector<std::string> userCommands;
    std::stringstream ss(input);
    std::string userInput;

    // Isolates the individual words separated by space
    // Assumption is the words are separated by space, so there is no defense against
    // accidental concatenation
    while (getline(ss, userInput, ' '))
    {
        userCommands.push_back(userInput);
    }
    return userCommands;
}
/* @Overload
** parseInput
** Overloaded parse input function for dealing with a string
** Passes the char array of the input to the parseInput() function
** Returns the return of parseInput() which is a vector of commands/input
*/
std::vector<std::string> Parser::parseInput(std::string input)
{
    return parseInput((const char *)input.c_str());
}