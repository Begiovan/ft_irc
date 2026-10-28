#include "ACommand.hpp"

Topic::Topic(Server &server) : ACommand(server){}
Topic::~Topic(){}

// syntax: TOPIC channel -> shows the current topic
// topic channel :topic -> sets a new topic
// topic channel : -> clears the topic by setting it to an empty string

void Topic::execute(Client *client, std::vector<std::string> params){
	if (params.size() < 1){
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "TOPIC") + "\r\n");
		return;
	}

	Channel *chan = _server->findChannel(params[0]);
	if (!chan)
	{
		_server->sendToClient(*client, ERR_NOSUCHCHANNEL(client->getNickname(), params[0]) + "\r\n");
		return;
	}
	if (!chan->isMember(client))
	{
		_server->sendToClient(*client, ERR_NOTONCHANNEL(client->getNickname(), params[0]) + "\r\n");
		return;
	}

	if (params.size() == 1)
	{
		if (chan->getTopic().empty())
			_server->sendToClient(*client, RPL_NOTOPIC(client->getNickname(), params[0]) + "\r\n");
		else
			_server->sendToClient(*client, RPL_TOPIC(client->getNickname(), params[0], chan->getTopic()) + "\r\n");
		return;
	}

	if (!chan->canChangeTopic(*client))
	{
		_server->sendToClient(*client, ERR_CHANOPRIVSNEEDED(client->getNickname(), params[0]) + "\r\n");
		return;
	}

	chan->setTopic(params[1]);
	std::string message = ":" + client->getNickname() + " TOPIC " + params[0] + " :" + chan->getTopic() + "\r\n";
	_server->broadcast(*chan, message);
}
