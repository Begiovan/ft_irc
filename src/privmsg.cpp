#include "ACommand.hpp"

Privmsg::Privmsg(Server &server) : ACommand(server){}
Privmsg::~Privmsg(){}

void Privmsg::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 2){
        _server->sendToClient(*client, ERR_NEEDMOREPARAMS("*", "PRIVMSG") + "\r\n");
        return;
    }

    if (params[0][0] == '#' || params[0][0] == '&')
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
        _server->broadcast(*chan, RPL_PRIVMSG(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n", client);
        Client *bot = _server->getBot();
        if (bot && chan->isMember(bot))
            _server->botOnMessage(client, params[0], params[1]);
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
            if (target->isBot())
                _server->botOnMessage(client, client->getNickname(), params[1]);
            else
                _server->sendToClient(*target, RPL_PRIVMSG(client->getNickname(), client->getUsername(), params[0], params[1]) + "\r\n");
        }
    }

}   