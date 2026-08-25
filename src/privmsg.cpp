#include "ACommand.hpp"

Privmsg::Privmsg(Server &server) : ACommand(server){}
Privmsg::~Privmsg(){}

void Privmsg::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 2){
        _server->sendToClient(*client, ERR_NEEDMOREPARAMS("*", "PRIVMSG") + "\r\n");
        return;
    }

    if (params[0][0] == '#')
    {
        Channel *chan = _server->findChannel(params[0]);
        if (!chan)
        {
            _server->sendToClient(*client, ERR_NOSUCHCHANNEL("*", params[0]) + "\r\n");
            return;
        }
        if (!chan->isMember(client))
        {
            _server->sendToClient(*client, ERR_CANNOTSENDTOCHAN("*", params[0]) + "\r\n");
            return;
        }
        _server->broadcast(*chan, RPL_PRIVMSG(client->getNickname(), params[0], params[1]) + "\r\n", client);
    }
    else
    {
        Client *target= _server->returnClient(params[0]);

        if(!target)
        {
            _server->sendToClient(*client, ERR_NOSUCHNICK("*", params[0]) + "\r\n");
            return;
        }
        else
        {
            _server->sendToClient(*target, RPL_PRIVMSG(client->getNickname(), params[0], params[1]) + "\r\n");
        }
    }

}   