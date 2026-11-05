#include "ACommand.hpp"

Join::Join(Server &server) : ACommand(server){}
Join::~Join(){}

// syntax: JOIN channel key (optional)

void Join::execute(Client *client, std::vector<std::string> params){
    if (params.size() < 1){
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN")+ "\r\n");
		return;
	}
	Channel *chan = _server->findChannel(params[0]);
	bool newChannel = false;

	if(!chan){
		if (params[0][0] != '#' && params[0][0] != '&'){
			_server->sendToClient(*client, ERR_NOSUCHCHANNEL(client->getNickname(), params[0]) + "\r\n");
			return;
		}
		chan = _server->createChannel(params[0]);
		newChannel = true;
	}
	else
	{
		Channel::JoinResult res;
		if (params.size() == 1)
			res = chan->canJoin(*client, "");
		else
			res = chan->canJoin(*client, params[1]);
		if (res != Channel::JOIN_OK){
			std::string err;
			switch (res)
			{
			case Channel::JOIN_ERR_INVITE_ONLY:
				err = ERR_INVITEONLYCHAN(client->getNickname(), params[0]);
				break;
			case Channel::JOIN_ERR_BAD_KEY:
				err = ERR_BADCHANNELKEY(client->getNickname(), params[0]);
				break;
			case Channel::JOIN_ERR_CHANNEL_FULL:
				err = ERR_CHANNELISFULL(client->getNickname(), params[0]);
				break;
			case Channel::JOIN_ERR_ALREADY_IN:
				return;
			default:
				err = ERR_UNKNOWNCOMMAND(client->getNickname(), "JOIN");
				break;
			}
			_server->sendToClient(*client, err + "\r\n");
			return;
		}
	}

	_server->joinChannel(*client, *chan);
	if (newChannel)
		chan->addOperator(*client);
	_server->broadcast(*chan, RPL_JOIN(client->getNickname(), client->getUsername(), params[0]) + "\r\n");
	_server->sendTopic(*client, *chan);
	_server->sendNames(*client, *chan);
}