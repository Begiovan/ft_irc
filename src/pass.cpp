#include "ACommand.hpp"

Pass::Pass(Server &server) : ACommand(server){}
Pass::~Pass(){}

void Pass::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 1){
        _server->sendToClient(*client, ERR_NEEDMOREPARAMS("*", "PASS") + "\r\n");
        return;
    }

    if (_server->getPassword() == params[0])
    {
        client->setRegistrationStatus(Client::PASS_OK);
    }
    else
    {
        _server->sendToClient(*client, ERR_PASSWDMISMATCH("*") + "\r\n");
        return;
    }
}