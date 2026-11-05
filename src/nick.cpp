#include "ACommand.hpp"

Nick::Nick(Server &server) : ACommand(server, ANY){}
Nick::~Nick(){}

void Nick::execute(Client *client, std::vector<std::string> params)
{
    if (params.size() < 1)
    {
        _server->sendToClient(*client,
            ERR_NONICKNAMEGIVEN("*") + "\r\n");
        return;
    }

    if (!(client->getRegistrationStatus() & Client::PASS_OK))
    {
        _server->sendToClient(*client, ERR_PASSWDMISMATCH("*") + "\r\n");
        return;
    }

    if (!isValidNickname(params[0]))
    {
        _server->sendToClient(*client,
            ERR_ERRONEUSNICKNAME("*", params[0]) + "\r\n");
        return;
    }

    Client *other = _server->getClientByNick(params[0]);
    if (other && other != client)
    {
        std::string who = client->getNickname().empty() ? "*" : client->getNickname();
        _server->sendToClient(*client, ERR_NICKNAMEINUSE(who, params[0]) + "\r\n");
        return;
    }

    std::string oldNick = client->getNickname();
    bool wasRegistered = client->isRegistered();

    client->setNickname(params[0]);
    client->setRegistrationStatus(Client::NICK_OK);

    if (wasRegistered)
        _server->broadcastToNeighbours(*client,
            RPL_NICK(oldNick, client->getUsername(), client->getNickname()) + "\r\n", true);
    else
        _server->tryCompleteRegistration(*client);
}
