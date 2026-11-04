#include "ACommand.hpp"

Privmsg::Privmsg(Server &server) : ACommand(server, "PRIVMSG", 2){}
Privmsg::~Privmsg(){}

void Privmsg::execute(Client &client, const std::vector<std::string> &params){
    if (!hasEnoughParams_OrReply(client, params))
        return;

    if (params[0][0] == '#' || params[0][0] == '&')
    {
        Channel *chan = resolveChannel_OrReply(client, params[0]);
        if (!chan || !findMember_OrReply(client, *chan))
            return;
        _server->broadcast(*chan, RPL_PRIVMSG(client.getNickname(), client.getUsername(), params[0], params[1]) + "\r\n", &client);
        Client *bot = _server->getBot();
        if (bot && chan->isMember(*bot))
            _server->botOnMessage(client, params[0], params[1]);
    }
    else
    {
        Client *target = resolveNick_OrReply(client, params[0]);

        if(!target)
            return;
        else
        {
            if (target->isBot())
                _server->botOnMessage(client, client.getNickname(), params[1]);
            else
                _server->sendToClient(*target, RPL_PRIVMSG(client.getNickname(), client.getUsername(), params[0], params[1]) + "\r\n");
        }
    }

}   