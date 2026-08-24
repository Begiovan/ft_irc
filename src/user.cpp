#include "ACommand.hpp"

User::User(Server &server) : ACommand(server){}
User::~User(){}

void User::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 1){
        _server->sendToClient(*client, ERR_NEEDMOREPARAMS("*", "USER") + "\r\n");
        return;
    }

    if (client->getRegistrationStatus() & Client::USER_OK)
    {
        _server->sendToClient(*client, ERR_ALREADYREGISTERED("*") + "\r\n");
        return;
    }
    else
    {
        client->setUsername(params[0]);
        client->setRegistrationStatus(Client::USER_OK);

        if (client->isRegistered())
            _server->sendToClient(*client, RPL_WELCOME(client->getNickname()) + "\r\n");
    }

}