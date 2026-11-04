#include "ACommand.hpp"

Topic::Topic(Server &server) : ACommand(server, "TOPIC", 1){}
Topic::~Topic(){}

// syntax: TOPIC channel -> shows the current topic
// topic channel :topic -> sets a new topic
// topic channel : -> clears the topic by setting it to an empty string

void Topic::execute(Client *client, std::vector<std::string> params){
	if (!client || !hasEnoughParams_OrReply(*client, params))
		return;

	Channel *chan = resolveChannel_OrReply(*client, params[0]);
	if (!chan || !findMember_OrReply(*client, *chan))
		return;

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
	_server->broadcast(*chan, RPL_TOPICCHANGE(client->getNickname(), client->getUsername(), params[0], chan->getTopic()) + "\r\n");
}
