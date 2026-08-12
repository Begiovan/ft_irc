#include "ACommand.hpp"

Topic::Topic(Server &server, bool isAuth) : ACommand(server, isAuth){}
Topic::~Topic(){}

// sintax : TOPIC channel -> mostra il topic
// topic channel :topic -> imposta nuovo topic
// topic channe : -> rimuove il topic impostandolo come stringa vuota

void Topic::execute(Client *client, std::vector<std::string> params){

	if (params.size() <= 1){
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "KICK" )+ "\r\n");
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
	if (params.size() == 2) // view topic
	{
		if (chan->getTopic().empty()) // topic not set
		{
			_server->sendToClient(*client, RPL_NOTOPIC(client->getNickname(), params[0])+ "\r\n");
			return;
		}
		else // topic set
		{
			_server->sendToClient(*client, RPL_TOPIC(client->getNickname(), params[0], chan->getTopic())+ "\r\n");
			return;
		}
	}
	else {
		if (!chan->canChangeTopic(*client))
		{
			_server->sendToClient(*client, ERR_CHANOPRIVSNEEDED(client->getNickname(), params[0])+ "\r\n");
            return;
		}
		else
		{
			chan->setTopic(*client, params[1]);
            std::string message = ":" + client->getNickname() + " TOPIC " + params[0] + " :" + chan->getTopic() + "\r\n";
            _server->broadcast(*chan, message);
		}
	}
}