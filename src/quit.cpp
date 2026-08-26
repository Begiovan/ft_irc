#include "ACommand.hpp"

Quit::Quit(Server &server) : ACommand(server){}
Quit::~Quit(){}

void Quit::execute(Client *client, std::vector<std::string> params){
    std::string message;
    if (params.size() >= 1)
        message = params[0];
    else
        message = "Client Quit";

    _server->disconnectClient(client->getFd(), message);
}