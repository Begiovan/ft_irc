#include "ACommand.hpp"

User::User(Server &server) : ACommand(server, "USER", 1, UNREGISTERED){}
User::~User(){}

void User::execute(Client &client, const std::vector<std::string> &params){
    if (!hasEnoughParams_OrReply(client, params))
        return;

    if (client.getRegistrationStatus() & Client::USER_OK)
    {
        _server->sendToClient(client, ERR_ALREADYREGISTERED("*") + "\r\n");
        return;
    }

    if (client.getRegistrationStatus() & Client::PASS_OK)
    {
        client.setUsername(params[0]);
        client.setRegistrationStatus(Client::USER_OK);

        if (client.isRegistered())
            _server->sendToClient(client, RPL_WELCOME(client.getNickname()) + "\r\n");
    }
    else
    {
        _server->sendToClient(client, ERR_PASSWDMISMATCH("*") + "\r\n");
        return;
    }
}
