#include "ACommand.hpp"

Ping::Ping(Server &server) : ACommand(server){}
Ping::~Ping(){}

void Ping::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 1){
        _server->sendToClient(*client, ERR_NEEDMOREPARAMS("*", "PING") + "\r\n");
        return;
    }

    _server->sendToClient(*client, RPL_PING("ircserv", params[0]) + "\r\n");
}