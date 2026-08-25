#include "ACommand.hpp"

Notice::Notice(Server &server) : ACommand(server){}
Notice::~Notice(){}

void Notice::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 2)
        return;

    if (params[0][0] == '#')
    {
        Channel *chan = _server->findChannel(params[0]);
        if (!chan)
            return;
        if (!chan->isMember(client))
            return;
        _server->broadcast(*chan, RPL_NOTICE(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n", client);
    }
    else
    {
        Client *target = _server->returnClient(params[0]);
        if (!target)
            return;
        _server->sendToClient(*target, RPL_NOTICE(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n");
    }
}