////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Final Project: Network Application                                            \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// The Commander class handles the user commands and client logic. Specific commands are   \\
// mapped to specific functions that perform instructions depending on the input code.     \\
// For certain functions/commands additional user inputs and string parsing is required.   \\
// Current available commands are:                                                         \\
// OPEN                                                                                    \\
// CD                                                                                      \\
// PWD                                                                                     \\
// LS                                                                                      \\
// SYST                                                                                    \\
// HELP                                                                                    \\
// GET                                                                                     \\
// PUT                                                                                     \\
// DEL                                                                                     \\
// CLOSE                                                                                   \\
// QUIT                                                                                    \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include "Commander.h"

/*
** Constructor initializers and map filling
*/
Commander::Commander()
{
    commandMap["open"] = OPEN;
    commandMap["cd"] = CD;
    commandMap["get"] = GET;
    commandMap["put"] = PUT;
    commandMap["del"] = DEL;
    commandMap["pwd"] = PWD;
    commandMap["syst"] = SYST;
    commandMap["ls"] = LS;
    commandMap["dir"] = LS;
    commandMap["help"] = HELP;
    commandMap["close"] = CLOSE;
    commandMap["quit"] = QUIT;
    active = true;
}

/* parseCommand
** Choses the appropriate command depending on passed in input
*/
bool Commander::parseCommand(std::vector<std::string> commands)
{
    if (commands.size() == 0)
    {
        return false;
    }
    std::string command = commands[0];
    transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    switch (commandMap[command])
    {
        case OPEN:
            return open(commands);
        case CD:
            return cd(commands);
        case PWD:
            return pwd();
        case LS:
            return ls();
        case SYST:
            return syst();
        case GET:
            return get(commands);
        case PUT:
            return put(commands);
        case DEL:
            return del(commands);
        case CLOSE:
            return close();
        case QUIT:
            return quit();
        case HELP:
            return help();
        default:
            fprintf(stderr, "?Invalid command\n");
            return false;
    }
}

/* open
** Opens a connection with manually inputted domain ID and port
*/
bool Commander::open(std::vector<std::string> commands)
{
    if (ftp.isActive())
    {
        fprintf(stderr, "Already connected to %s, close connection first.\n", ftp.getServerName().c_str());
        return false;
    }
    if (commands.size() == 1)
    {
        printf("(to) ");
        commands.clear();
        char userInput[512];
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
    }
    else
    {
        commands.erase(commands.begin());
    }
    
    if (commands.size() != 0)
    {
        const char *serverName = commands[0].c_str();

        int portNum = 21;
        if (commands.size() == 2)
        {
            portNum = atoi(commands[1].c_str());
        }

        if (commands.size() < 3)
        {
            return open(serverName, portNum) > 0;
        }
    }
    fprintf(stderr, "open host-name [port]\n");
    return false;
}

/* open
** Opens a connection to the specified domain ID and port
*/
bool Commander::open(const char *serverName, int portNum)
{
    if (ftp.isActive())
    {
        fprintf(stderr, "Already connected to %s, close connection first.\n", ftp.getServerName().c_str());
        return false;
    }
    int clientSD = ftp.openSocket(serverName, portNum);
    if (clientSD > 0)
    {
        printf("Connected to %s.\n", serverName);
    }
    int repCode = 0;
    if (ftp.receiveMessage() > 0)
    {
        printf("%s",ftp.getMessage().c_str());
        repCode = ftp.getReply();
    }
    if (repCode == 220)
    {
        Parser stringCommander;
        std::vector<std::string> commands;
        char userInput[512];
        std::string userString = getlogin();
        std::string userName = "USER ";
        std::string password = "PASS ";
        printf("Name (%s:%s): ", ftp.getServerName().c_str(), userString.c_str());
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
        if (commands.size() != 0)
        {
            userString = commands[0];
        }
        int repCode = 0;
         userName += userString;
        if (ftp.sendCommand(userName) > 0)
        {
            if (ftp.receiveMessage() > 0)
            {
                printf("%s",ftp.getMessage().c_str());
                repCode = ftp.getReply();
            }
        }
        if (repCode == 331)
        {
            printf("Password:");
            std::cin.getline(userInput, 512);
            commands = stringParser.parseInput(userInput);
            if (commands.size() != 0)
            {
                password += commands[0];
            }
            
            if (ftp.sendCommand(password) > 0)
            {
                if (ftp.receiveMessage() > 0)
                {
                    printf("%s",ftp.getMessage().c_str());
                    ftp.receiveStream(ftp.getClientSD());
                    printf("%s",ftp.getAscii().c_str());
                    repCode = ftp.getStream();
                }
            }
        }
        if(repCode == 230)
        {
            ftp.receiveStream(ftp.getClientSD());
            printf("%s",ftp.getAscii().c_str());
            syst();
            return ftp.isActive();
        }
        if (repCode == 530)
        {
            fprintf(stderr, "Login failed!\n");
        }
    }
    ftp.closeSocket(clientSD);
    return false;
}

/* cd
** Changes the current working directory to the one specified by the pathname
*/
bool Commander::cd(std::vector<std::string> commands)
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    int repCode = 0;
    std::string sendCommand = "CWD ";
    char userInput[512];
    if (commands.size() == 1)
    {
        printf("(remote-directory) ");
        commands.clear();
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
    }
    else
    {
        commands.erase(commands.begin());
    }
    if (commands.size() != 0)
    {
        sendCommand += commands[0];
        if (ftp.sendCommand(sendCommand) > 0)
        {
            if (ftp.receiveMessage() > 0)
            {
                printf("%s", ftp.getMessage().c_str());
                repCode = ftp.getReply();
            }
        }
        return (repCode == 250);
    }
    fprintf(stderr, "cd [remote-directory]\n");
    return false;
}

/* get
** Gets/downloads the file from the server's remote location and stores into client's local location
*/
bool Commander::get(std::vector<std::string> commands)
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    std::string remoteLocation;
    std::string localLocation;
    int repCode = 0;
    char userInput[512];
    int newPassSD = 0;
    int status;
    int forkOK;
    std::string sendCommand = "RETR ";
    // User input does not specify remote file name, get info from user
    if (commands.size() == 1)
    {
        printf("(remote-file) ");
        commands.clear();
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
        if (commands.size() == 1)
        {
            remoteLocation = commands[0];
            printf("(local-file) ");
            commands.clear();
            std::cin.getline(userInput, 512);
            commands = stringParser.parseInput(userInput);
        }
        if (commands.size() == 1)
        {
            localLocation = commands[0];
        }  
    }
    // User input contains only remote file name (local file name = remote)
    else if (commands.size() == 2)
    {
        remoteLocation = commands[1];
        localLocation = commands[1];
    }
    // User input contains remote and local file name
    else if (commands.size() == 3)
    {
        remoteLocation = commands[1];
        localLocation = commands[2];
    }
    if ((localLocation.size() == 0) || (remoteLocation.size() == 0))
    {
        fprintf(stderr, "get [remote-file] [local-file]");
        return false;
    }
    if (!passiveConnection())
    {
        return false;
    }
    if (ftp.sendCommand("TYPE I") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
    }
    if (repCode != 200)
    {
        return false;
    }
    newPassSD = ftp.getPassiveSD();
    sendCommand += remoteLocation;
    if (ftp.sendCommand(sendCommand) > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
        struct timeval startTime;
        struct timeval endTime;
        startTime.tv_sec = 0;
        startTime.tv_usec = 0;
        endTime.tv_sec = 0;
        endTime.tv_usec = 0;
        double totalTime = 0;
        double seconds;
        long fileSize = 0;
        if (repCode == 150)
        {
            if (ftp.makeFile(localLocation.c_str()) < 0)
            {
                fprintf(stderr, "local: %s: Permission denied\n", localLocation.c_str());
                ftp.closeSocket(newPassSD);
                ftp.closeFile();
                return false;
            }
            forkOK = fork();
            if (forkOK < 0)
            {
                fprintf(stderr, "Fork failed!\n");
                exit(EXIT_FAILURE);
            }
            else if (forkOK == 0)
            {
                // Struct and variables for measuring time
                gettimeofday(&startTime, NULL);
                fileSize = ftp.receiveFile();
                gettimeofday(&endTime, NULL);
                totalTime = (((endTime.tv_sec - startTime.tv_sec) * 1000000) + (endTime.tv_usec - startTime.tv_usec));
                ftp.closeSocket(newPassSD);
                ftp.closeFile();
                if (ftp.receiveMessage() > 0)
                {
                    printf("%s", ftp.getMessage().c_str());
                    repCode = ftp.getReply();
                }
                if (repCode == 226)
                {
                    seconds = (totalTime / 1000000);
                    printf("%ld bytes received in %.2f sec (%.4f kB/s)\n", fileSize, seconds, (fileSize / seconds / 1024));
                }
                exit(EXIT_SUCCESS);
            }
            else
            {
                wait (&status);
            }
        }
    }
    ftp.closeSocket(newPassSD);
    return (ftp.getReply() == 226);
}

/* put
** Stores/uploads the file from the client's local location onto server's remote location
*/
bool Commander::put(std::vector<std::string> commands)
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    
    std::string remoteLocation;
    std::string localLocation;
    int repCode = 0;
    char userInput[512];
    int newPassSD = 0;
    int status;
    int forkOK;
    std::string sendCommand = "STOR ";
    // User input does not specify local file name, get info from user
    if (commands.size() == 1)
    {
        printf("(local-file) ");
        commands.clear();
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
        if (commands.size() == 1)
        {
            localLocation = commands[0];
            printf("(remote-file) ");
            commands.clear();
            std::cin.getline(userInput, 512);
            commands = stringParser.parseInput(userInput);
        }
        if (commands.size() == 1)
        {
            remoteLocation = commands[0];
        }
    }
    // User input contains only local file name (remote file name = local)
    else if (commands.size() == 2)
    {
        localLocation = commands[1];
        remoteLocation = commands[1];
    }
    // User input contains local and remote file name
    else if (commands.size() == 3)
    {
        localLocation = commands[1];
        remoteLocation = commands[2];
    }
    if ((localLocation.size() == 0) || (remoteLocation.size() == 0))
    {
        fprintf(stderr, "get [remote-file] [local-file]");
        return false;
    }
    if (ftp.readyFile(localLocation.c_str()) < 0)
    {
        fprintf(stderr, "local: %s: No such file or directory\n", localLocation.c_str());
        return false;
    }
    if (!passiveConnection())
    {
        return false;
    }
    if (ftp.sendCommand("TYPE I") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
    }
    if (repCode != 200)
    {
        return false;
    }
    newPassSD = ftp.getPassiveSD();
    sendCommand += remoteLocation;
    if (ftp.sendCommand(sendCommand) > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
        // Struct and variables for measuring time
        struct timeval startTime;
        struct timeval endTime;
        startTime.tv_sec = 0;
        startTime.tv_usec = 0;
        endTime.tv_sec = 0;
        endTime.tv_usec = 0;
        double totalTime = 0;
        double seconds;
        long fileSize = 0;
        if (repCode == 150)
        {
            forkOK = fork();
            if (forkOK < 0)
            {
                fprintf(stderr, "Fork failed!\n");
                exit(EXIT_FAILURE);
            }
            else if (forkOK == 0)
            {
                gettimeofday(&startTime, NULL);
                fileSize = ftp.sendFile();
                gettimeofday(&endTime, NULL);
                totalTime = (((endTime.tv_sec - startTime.tv_sec) * 1000000) + (endTime.tv_usec - startTime.tv_usec));
                ftp.closeSocket(newPassSD);
                ftp.closeFile();
                if (ftp.receiveMessage() > 0)
                {
                    printf("%s", ftp.getMessage().c_str());
                    repCode = ftp.getReply();
                }
                if (repCode == 226)
                {
                    seconds = (totalTime / 1000000);
                    printf("%ld bytes sent in %.2f sec (%.4f kB/s)\n", fileSize, seconds, (fileSize / seconds / 1024));
                }
                exit(EXIT_SUCCESS);
            }
            else
            {
                wait(&status);
            }
        }
    }
    ftp.closeSocket(newPassSD);
    return (ftp.getReply() == 226);
}

/* del
** Deletes the file at the server's remote location
*/
bool Commander::del(std::vector<std::string> commands)
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    char userInput[512];
    std::string sendCommand = "DELE ";
    // User input does not specify remote file name, get info from user
    if (commands.size() == 1)
    {
        printf("(remote-file) ");
        commands.clear();
        std::cin.getline(userInput, 512);
        commands = stringParser.parseInput(userInput);
    }
    else
    {
        commands.erase(commands.begin());
    }
    
    if (commands.size() != 0)
    {
        int replyCode = 0;
        sendCommand += commands[0];
        if (ftp.sendCommand(sendCommand) > 0)
        {
            if (ftp.receiveMessage() > 0)
            {
                printf("%s", ftp.getMessage().c_str());
                replyCode = ftp.getReply();
            }
        }
        return replyCode == 250;
    }
    fprintf(stderr, "del [remote-file]\n");
    return false;
}

/* pwd
 ** Displays the current working directory
 */
bool Commander::pwd()
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    int repCode = 0;
    if (ftp.sendCommand("PWD") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
    }
    return (repCode == 257);
}

/* syst
 ** Displays the type of the server the client is connected to
 */
bool Commander::syst()
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    int repCode = 0;
    if (ftp.sendCommand("SYST") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
    }
    return (repCode == 215);
}

/* ls
** Gets a list (in ASCII stream) of contents of the current remote working directory
*/
bool Commander::ls()
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    if (!passiveConnection())
    {
        return false;
    }
    int newPassSD = ftp.getPassiveSD();
    int status;
    int forkOK = fork();
    if (forkOK < 0)
    {
        fprintf(stderr, "Fork failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (forkOK == 0)
    {
        if (ftp.receiveStream(newPassSD) > 0)
        {
            printf("%s", ftp.getAscii().c_str());
        }
        ftp.closeSocket(newPassSD);
        exit(EXIT_SUCCESS);
    }
    else
    {
        if (ftp.sendCommand("LIST") > 0)
        {
            if (ftp.receiveMessage() > 0)
            {
                printf("%s", ftp.getMessage().c_str());
            }
            wait(&status);
            if (ftp.receiveMessage() > 0)
            {
                printf("%s", ftp.getMessage().c_str());
            }
        }
    }
    return true;
}

/* passiveConnection
** Establish a new (passive) data connection with the server
*/
bool Commander::passiveConnection()
{
    int repCode = 0, newPassSD = -1, ipOffset = 0, ipMultiplier = 0, newPort = 0;
    if (ftp.sendCommand("PASV") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            repCode = ftp.getReply();
        }
    }
    if (repCode == 227)
    {
        std::vector<std::string> userCommands;
        std::stringstream ss(ftp.getMessage());
        std::string userInput;
        while (getline(ss, userInput, ','))
        {
            userCommands.push_back(userInput);
        }
        userInput = userCommands.back();
        userCommands.pop_back();
        for (int position = userInput.size(); position <= 0; position--)
        {
            if (!isdigit(userInput[position]) && (userInput.size() != 0))
            {
                userInput.erase(userInput.begin() + position);
            }
        }
        ipOffset = (userInput.size() > 0) ? (atoi(userInput.c_str())) : (-1);
        userInput = userCommands.back();
        userCommands.pop_back();
        for (int position = userInput.size(); position <= 0; position--)
        {
            if (!isdigit(userInput[position]) && (userInput.size() != 0))
            {
                userInput.erase(userInput.begin() + position);
            }
        }
        ipMultiplier = (userInput.size() > 0) ? (atoi(userInput.c_str())) : (-1);
        newPort = (ipMultiplier * 256 + ipOffset);
        newPassSD = ftp.openPassive(newPort);
        printf("%s", ftp.getMessage().c_str());
        if (newPassSD <= 0)
        {
            fprintf(stderr, "Passive mode refused!\n");
        }
    }
    return (newPassSD > 0);
}

/* close
** Closes the command connection channel with the server (does not terminate FTP program)
*/
bool Commander::close()
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    if (ftp.sendCommand("QUIT") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
        }
    }
    ftp.closeSocket(ftp.getClientSD());
    return true;
}

/* close
** Closes the command connection channel with the server, and terminates FTP program
*/
bool Commander::quit()
{
    if (ftp.isActive())
    {
        close();
    }
    active = false;
    return (ftp.isActive()) ? close() : true;
}

bool Commander::help()
{
    if (!ftp.isActive())
    {
        fprintf(stderr, "Not connected, connect first!\n");
        return false;
    }
    int repCode = 0;
    if (ftp.sendCommand("HELP") > 0)
    {
        if (ftp.receiveMessage() > 0)
        {
            printf("%s", ftp.getMessage().c_str());
            repCode = ftp.getReply();
        }
    }
    return (repCode == 214);
}

/* isActive
** Returns if the Commander connection to the server is opened or closed (i.e. isActive)
*/
bool Commander::isActive() const
{
    return active;
}