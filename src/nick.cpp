#include "ACommand.hpp"

Nick::Nick(Server &server) : ACommand(server){}
Nick::~Nick(){}

void Nick::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 1){
        _server->sendToClient(*client, ERR_NONICKNAMEGIVEN("*") + "\r\n");
        return;
    }

    if (isValidNickname(params[0]))
    {
        if(_server->findClient(params[0]))
            _server->sendToClient(*client, ERR_NICKNAMEINUSE(client->getNickname(), params[0]) + "\r\n");
        else
        {
            std::string oldNick = client->getNickname();
            client->setNickname(params[0]);
            client->setRegistrationStatus(Client::NICK_OK);
            if (client->isRegistered())
            {
                if(!client->getChannels().empty())
                {
                    for(std::set<const Channel *>::const_iterator it = client->getChannels(); it != client->getChannels.end(); ++it)
                    {
                        for(std::set<const Client *>::const_iterator notify = members.begin(); notify != members.end(); ++notify)
                            _server->sendToClient(*notify, RPL_NICK(oldNick, client->getUsername(), client->getNickname()));
                    }
                }
                _server->sendToClient(*client, RPL_WELCOME(client->getNickname()) + "\r\n");
            }
        }
    }
    else
    {
        _server->sendToClient(*client, ERR_ERRONEUSNICKNAME("*", params[0]) + "\r\n");
        return;
    }
}