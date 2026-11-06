#include "ACommand.hpp"

Kick::Kick(Server &server) : ACommand(server, "KICK", 2){}
Kick::~Kick(){}

// syntax: KICK channel target: message (optional)

void Kick::execute(Client &client, const std::vector<std::string> &params){
	if (!hasEnoughParams_OrReply(client, params))
		return;
	Channel *chan = resolveChannel_OrReply(client, params[0]);
	if (!chan || !findMember_OrReply(client, *chan) || !findOperator_OrReply(client, *chan))
		return;
	Client *target = resolveNick_OrReply(client, params[1]);
	if (!target || !chan->isMember(*target))
	{
		if (target)
			_server->sendToClient(client, ERR_USERNOTINCHANNEL(client.getNickname(), target->getNickname(), params[0]) + "\r\n");
		return;
	}
	std::string reason = "No Reason Given.";
	if (params.size() == 3)
		reason = params[2];
	_server->broadcast(*chan, RPL_KICK(client.getNickname(), client.getUsername(), params[0], params[1], reason)  + "\r\n");

	_server->partChannel(*target, *chan);
}