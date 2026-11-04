#include "ACommand.hpp"

Notice::Notice(Server &server) : ACommand(server, "NOTICE", 2){}
Notice::~Notice(){}

void Notice::execute(Client *client, std::vector<std::string> params){
    if (!client || !hasEnoughParams_OrReply(*client, params))
        return;
    if (params[0][0] == '#')
    {
        Channel *chan = resolveChannel_OrReply(*client, params[0]);
        if (!chan || !findMember_OrReply(*client, *chan))
            return;
        _server->broadcast(*chan, RPL_NOTICE(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n", client);
    }
    else
    {
        Client *target = resolveNick_OrReply(*client, params[0]);
        if (!target)
            return;
        _server->sendToClient(*target, RPL_NOTICE(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n");
    }
}