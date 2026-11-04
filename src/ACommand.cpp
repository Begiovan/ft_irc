#include "ACommand.hpp"

AuthRequirement	ACommand::authRequirement()
{
	return _authRequirement;
}

bool	ACommand::hasEnoughParams_OrReply(Client &c, const std::vector<std::string> &params)
{
	if (params.size() < _requiredParams)
	{
		_server->sendToClient(c, ERR_NEEDMOREPARAMS( c.getNickname(), this->_name));
		return false;
	}
	return true;
}

Channel*	ACommand::resolveChannel_OrReply(Client &c, const std::string &chName)
{
	Channel *chan = _server->findChannel(chName);
	if (!chan)
	{
		_server->sendToClient(c, ERR_NOSUCHCHANNEL(c.getNickname(), chName));
		return NULL;
	}
	return chan;
}

bool	ACommand::findMember_OrReply(Client &c, Channel &ch)
{
	if (!ch.isMember(&c))
	{
		_server->sendToClient(c, ERR_NOTONCHANNEL(c.getNickname(), ch.getName()));
		return false;
	}
	return true;
}

bool	ACommand::findOperator_OrReply(Client &c, Channel &ch)
{
	if (!ch.isOperator(c))
	{
		_server->sendToClient(c, ERR_CHANOPRIVSNEEDED(c.getNickname(), ch.getName()));
		return false;
	}
	return true;
}

Client *ACommand::resolveNick_OrReply(Client &c, const std::string &nick)
{
    Client *target = _server->getClientByNick(nick);

    if (!target)
    {
        _server->sendToClient(c, ERR_NOSUCHNICK(c.getNickname(), nick));
        return NULL;
    }
    return target;
}
