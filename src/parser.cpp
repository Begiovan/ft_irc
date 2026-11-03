#include "parser.hpp"

#include <cctype>

Command parseCommand(const std::string &line){

    Command cmd;
    size_t pos = line.find(' ');
    
    if(pos == std::string::npos)
    {
        cmd.command = line;
    }
    else
    {
        std::string nline = line.substr(pos + 1);
        cmd.command = line.substr(0, pos);

        while(true)
        {
            
            if (nline[0] == ':')
            {
                std::string param = nline.substr(1);
                cmd.params.push_back(param);
                break ;
            }
            else
            {
                size_t spacePos = nline.find(' ');
                if(spacePos == std::string::npos)
                {
                    cmd.params.push_back(nline);
                    break ;
                }
                else
                {
                    std::string param = nline.substr(0, spacePos);
                    cmd.params.push_back(param);
                    nline.erase(0, spacePos + 1);
                }
            }
        }
    }

    for (size_t i = 0; i < cmd.command.length(); ++i)
        cmd.command[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd.command[i])));
    
    return cmd;
}