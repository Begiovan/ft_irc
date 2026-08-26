#include "ACommand.hpp"

Cap::Cap(Server &server) : ACommand(server){}
Cap::~Cap(){}

void Cap::execute(Client *client, std::vector<std::string> params)
{
    std::string nick = client->getNickname();
    if (nick.empty())
        nick = "*";

    if (params.empty() || params[0] == "LS")
        _server->sendToClient(*client, ":ircserv CAP " + nick + " LS :\r\n");
    else if (params[0] == "REQ")
    {
        std::string requested = params.size() > 1 ? params[1] : "";
        _server->sendToClient(*client, ":ircserv CAP " + nick + " NAK :" + requested + "\r\n");
    }
}
