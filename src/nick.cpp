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
            _server->sendToClient(*client, ERR_NICKNAMEINUSE("*") + "\r\n");
        else
        {
            client->setNickname(params[0]);
            client->setRegistrationStatus(Client::NICK_OK);
        }
    }
    else
{
    _server->sendToClient(*client, ERR_ERRONEUSNICKNAME("*", params[0]) + "\r\n");
    return;
}
}