#include "ACommand.hpp"

Invite::Invite(Server &server) : ACommand(server, "INVITE", 2){}
Invite::~Invite(){}

// syntax: INVITE nickname channel

void Invite::execute(Client *client, std::vector<std::string> params){
	if (!client || !hasEnoughParams_OrReply(*client, params))
		return;
	Client *target = resolveNick_OrReply(*client, params[0]);
	if (!target)
		return;
	Channel *chan = resolveChannel_OrReply(*client, params[1]);
	if (!chan || !findMember_OrReply(*client, *chan) || !findOperator_OrReply(*client, *chan))
		return;
	if (chan->isMember(target))
	{
		_server->sendToClient(*client, ERR_USERONCHANNEL(client->getNickname(), target->getNickname(), params[1]) + "\r\n");
		return;
	}
	chan->inviteUser(*target);
	_server->sendToClient(*client, RPL_INVITING(client->getNickname(), target->getNickname(), params[1]) + "\r\n");
	_server->sendToClient(*target, RPL_INVITE(client->getNickname(), client->getUsername(), target->getNickname(), params[1]) + "\r\n");
	return;
}