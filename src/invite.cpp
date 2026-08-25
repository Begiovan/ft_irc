#include "ACommand.hpp"

Invite::Invite(Server &server) : ACommand(server){}
Invite::~Invite(){}

// sintax INVITE nickname channel

void Invite::execute(Client *client, std::vector<std::string> params){
	if (params.size() < 2)
	{
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE")+ "\r\n");
		return;
	}
	Client *target = _server->returnClient(params[0]);
	if (target == NULL)
	{
		_server->sendToClient(*client, ERR_NOSUCHNICK(client->getNickname(), params[0])+ "\r\n");
		return;
	}
	Channel *chan = _server->findChannel(params[1]);
	if (!chan)
	{
		_server->sendToClient(*client, ERR_NOSUCHCHANNEL(client->getNickname(), params[1])+ "\r\n");
		return;
	}
	if (!(chan->isMember(client)))
	{
		_server->sendToClient(*client, ERR_NOTONCHANNEL(client->getNickname(), params[1])+ "\r\n");
		return;
	}
	if (!chan->isOperator(*client))
	{
		_server->sendToClient(*client, ERR_CHANOPRIVSNEEDED(client->getNickname(), params[1]) + "\r\n");
		return;
	}
	if (chan->isMember(target))
	{
		_server->sendToClient(*client, ERR_USERONCHANNEL(client->getNickname(), target->getNickname(), params[1]));
		return;
	}
	chan->inviteUser(*target);
	_server->sendToClient(*client, RPL_INVITING(client->getNickname(), target->getNickname(), params[1]) + "\r\n");
	_server->sendToClient(*target, RPL_INVITE(client->getNickname(), client->getUsername(), target->getNickname(), params[1]) + "\r\n");
	return;
}