#include "ACommand.hpp"

Kick::Kick(Server &server, bool isAuth) : ACommand(server, isAuth){}
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
	if (!(chan->isMember(target)))
	{
		_server->sendToClient(*client, ERR_USERNOTINCHANNEL(client->getNickname(), target->getNickname(), params[0])+ "\r\n");
		return;
	}
	std::string reason = "No Reason Given.\r\n";
	if (params.size() == 3)
		reason = params[2] + "\r\n";
	_server->broadcast(*chan, RPL_KICK(client->getNickname(), params[0], params[1], reason));
	chan->removeMember(*target);
	target->removeChannel(chan);
}