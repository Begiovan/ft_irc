#include "ACommand.hpp"

Ping::Ping(Server &server) : ACommand(server, "PING", 1){}
Ping::~Ping(){}

void Ping::execute(Client &client, const std::vector<std::string> &params){
    if (!hasEnoughParams_OrReply(client, params))
        return;

    _server->sendToClient(client, RPL_PING("ircserv", params[0]) + "\r\n");
}