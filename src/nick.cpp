#include "ACommand.hpp"

Nick::Nick(Server &server) : ACommand(server, "NICK", 1, ANY){}
Nick::~Nick(){}

void Nick::execute(Client &client, const std::vector<std::string> &params)
{
    if (!hasEnoughParams_OrReply(client, params))
        return;

    if (!(client.getRegistrationStatus() & Client::PASS_OK))
    {
        _server->sendToClient(client, ERR_PASSWDMISMATCH("*") + "\r\n");
        return;
    }

    if (!isValidNickname(params[0]))
    {
        _server->sendToClient(client,
            ERR_ERRONEUSNICKNAME("*", params[0]) + "\r\n");
        return;
    }

    if (_server->getClientByNick(params[0]) != NULL)
    {
        _server->sendToClient(client,
            ERR_NICKNAMEINUSE(client.nickOrStar(), params[0]) + "\r\n");
        return;
    }

    std::string oldNick = client.getNickname();
    bool wasRegistered = client.isRegistered();

    client.setNickname(params[0]);
    client.setRegistrationStatus(Client::NICK_OK);

    if (wasRegistered)
    {
        const std::string message = RPL_NICK(oldNick,client.getUsername(), client.getNickname()) + "\r\n";
        _server->broadcastToNeighbours(client, message, false);
    }
    else if (client.isRegistered())
    {
        _server->sendToClient(
            client,
            RPL_WELCOME(client.getNickname()) + "\r\n"
        );
    }
}
