#include "ACommand.hpp"

Nick::Nick(Server &server) : ACommand(server){}
Nick::~Nick(){}

void Nick::execute(Client *client, std::vector<std::string> params)
{
    if (params.size() < 1)
    {
        _server->sendToClient(*client,
            ERR_NONICKNAMEGIVEN("*") + "\r\n");
        return;
    }

    if (!isValidNickname(params[0]))
    {
        _server->sendToClient(*client,
            ERR_ERRONEUSNICKNAME("*", params[0]) + "\r\n");
        return;
    }

    if (_server->findClient(params[0]))
    {
        _server->sendToClient(*client,
            ERR_NICKNAMEINUSE(client->getNickname(), params[0]) + "\r\n");
        return;
    }

    std::string oldNick = client->getNickname();
    bool wasRegistered = client->isRegistered();

    client->setNickname(params[0]);
    client->setRegistrationStatus(Client::NICK_OK);

    if (wasRegistered)
    {
        std::set<const Client *> notifyClients;

        const std::set<const Channel *> &channels = client->getChannels();

        for (std::set<const Channel *>::const_iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            const std::set<const Client *> &members = (*it)->getMembers();

            for (std::set<const Client *>::const_iterator member = members.begin();
                 member != members.end(); ++member)
            {
                notifyClients.insert(*member);
            }
        }

        notifyClients.insert(client);

        for (std::set<const Client *>::const_iterator it = notifyClients.begin();
             it != notifyClients.end(); ++it)
        {
            _server->sendToClient(
                const_cast<Client &>(**it),
                RPL_NICK(oldNick,
                         client->getUsername(),
                         client->getNickname()) + "\r\n"
            );
        }
    }
    else if (client->isRegistered())
    {
        _server->sendToClient(
            *client,
            RPL_WELCOME(client->getNickname()) + "\r\n"
        );
    }
}