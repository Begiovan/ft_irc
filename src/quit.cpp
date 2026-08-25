#include "ACommand.hpp"

Quit::Quit(Server &server) : ACommand(server){}
Quit::~Quit(){}

void Quit::execute(Client *client, std::vector<std::string> params){
    
    std::string message;
    if (params.size() >= 1)
        message = params[0];
    else
        message = "Client Quit";
    std::set<const Channel*> channelsCopy = client->getChannels();
    for (std::set<const Channel*>::iterator chIt = channelsCopy.begin(); chIt != channelsCopy.end(); ++chIt)
    {
        _server->broadcast(**chIt, RPL_QUIT(client->getNickname(), message) + "\r\n");
    }

    _server->disconnectClient(client->getFd());

}