#include "ACommand.hpp"

Kick::Kick(Server &server) : ACommand(server){}
Kick::~Kick(){}

// sintax: KICK channel target: message (optional)

void Kick::execute(Client *client, std::vector<std::string> params){
	
	if (params.size() < 2 )
	{
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "KICK")+ "\r\n");
		return;
	}
	Channel *chan = _server->findChannel(params[0]);
	if (!chan)
	{
		_server->sendToClient(*client, ERR_NOSUCHCHANNEL(client->getNickname(), params[0])+ "\r\n");
		return;
	}
	if (!(chan->isMember(client)))
	{
		_server->sendToClient(*client, ERR_NOTONCHANNEL(client->getNickname(), params[0])+ "\r\n");
		return;
	}
	if (!(chan->isOperator(*client)))
	{
		_server->sendToClient(*client, ERR_CHANOPRIVSNEEDED(client->getNickname(), params[0])+ "\r\n");
		return;
	}
	Client *target = _server->returnClient(params[1]);
		if (target == NULL)
	{
		_server->sendToClient(*client, ERR_NOSUCHNICK(client->getNickname(), params[1])+ "\r\n");
		return;
	}
	if (!(chan->isMember(target)))
	{
		_server->sendToClient(*client, ERR_USERNOTINCHANNEL(client->getNickname(), target->getNickname(), params[0])+ "\r\n");
		return;
	}
	std::string reason = "No Reason Given.";
	if (params.size() == 3)
		reason = params[2];
	_server->broadcast(*chan, RPL_KICK(client->getNickname(), client->getUsername(), params[0], params[1], reason)  + "\r\n");
	chan->removeOperator(*target);
	chan->removeInvite(*target);
	chan->removeMember(*target);
	target->removeChannel(chan);
	_server->removeEmptyChan(chan);
}