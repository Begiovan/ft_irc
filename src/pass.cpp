#include "ACommand.hpp"

Pass::Pass(Server &server) : ACommand(server, "PASS", 1, UNREGISTERED){}
Pass::~Pass(){}

void Pass::execute(Client *client, std::vector<std::string> params){
    if (!client || !hasEnoughParams_OrReply(*client, params))
        return;

    if (_server->getPassword() == params[0])
    {
        client->setRegistrationStatus(Client::PASS_OK);
        if (client->isRegistered())
            _server->sendToClient(*client, RPL_WELCOME(client->getNickname()) + "\r\n");
    }
    else
    {
        _server->sendToClient(*client, ERR_PASSWDMISMATCH("*") + "\r\n");
        return;
    }
}
