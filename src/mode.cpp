#include "ACommand.hpp"
#include <sstream>
#include <cstdlib>

Mode::Mode(Server &server) : ACommand(server, "MODE", 1){}
Mode::~Mode(){}

// syntax: MODE channel -> shows the active modes on the channel
// MODE channel +/- letter value -> sets or clears a mode value
// MODE channel +letters -> configures multiple mode flags

static std::string getModes(Channel &channel){
	std::string mode;
	std::string args;

	if (channel.isInviteOnly())
		mode += "i";
	if (channel.isTopicRestricted())
		mode += "t";
	if (channel.hasKey())
		mode += "k";
	int limit = channel.getUserLimit();
	if (limit > -1)
	{
		mode += "l";
		std::stringstream ss;
		ss << limit;
		args = ss.str();
	}
	if (mode.empty())
		return "";
	return "+" + mode + (args.empty() ? "" : " " + args);
}

void Mode::execute(Client &client, const std::vector<std::string> &params){
	if (!hasEnoughParams_OrReply(client, params))
		return;

	Channel *chan = resolveChannel_OrReply(client, params[0]);
	if (!chan || !findMember_OrReply(client, *chan))
		return;
	if (params.size() == 1)
	{
		std::string modes = getModes(*chan);
		_server->sendToClient(client, RPL_MODE(client.getNickname(), client.getUsername(), params[0], modes, "") + "\r\n");
		return;
	}
	if (!findOperator_OrReply(client, *chan))
		return;

	bool		changed = false;
	bool		enable = true;
	size_t		numArgs = 2;
	std::string	mode = params[1];

	// TODO In this way we accept only one mode at time
	for (size_t i = 0; i < mode.size(); i++)
	{
		switch (mode[i])
		{
			case '+':
				enable = true;
				continue;
			case '-':
				enable = false;
				continue;
			case 'i':
				chan->setInviteOnly(enable);
				changed = true;
				break;
			case 't':
				chan->setTopicRestricted(enable);
				changed = true;
				break;
			case 'o':
				if (params.size() <= numArgs)
				{
					_server->sendToClient(client, ERR_NEEDMOREPARAMS(client.getNickname(), "MODE") + "\r\n");
					return;
				}
				{
					Client *target = resolveNick_OrReply(client, params[numArgs]);
					if (!target)
						return;
					if (!chan->isMember(*target))
					{
						_server->sendToClient(client, ERR_USERNOTINCHANNEL(client.getNickname(), target->getNickname(), params[0]) + "\r\n");
						return;
					}
					if (enable)
						chan->addOperator(*target);
					else
						chan->removeOperator(*target);
					_server->broadcast(*chan, RPL_MODE(client.getNickname(), client.getUsername(), params[0], params[1], target->getNickname()) + "\r\n");
				}
				numArgs++;
				break;
			case 'k':
				if (enable)
				{
					if (params.size() <= numArgs)
					{
						_server->sendToClient(client, ERR_NEEDMOREPARAMS(client.getNickname(), "MODE") + "\r\n");
						return;
					}
					chan->setKey(params[numArgs]);
					numArgs++;
				}
				else
					chan->clearKey();
				changed = true;
				break;
			case 'l':
				if (enable)
				{
					if (params.size() <= numArgs)
					{
						_server->sendToClient(client, ERR_NEEDMOREPARAMS(client.getNickname(), "MODE") + "\r\n");
						return;
					}
					if (!isPositiveInteger(params[numArgs]))
					{
						_server->sendToClient(client, ERR_NEEDMOREPARAMS(client.getNickname(), "MODE") + "\r\n");
						return;
					}
					int limit = atoi(params[numArgs].c_str());
					if(!chan->setUserLimit(limit))
					{
						_server->sendToClient(client, ERR_NEEDMOREPARAMS(client.getNickname(), "MODE") + "\r\n");
						return;
					}
					numArgs++;
				}
				else
					chan->clearUserLimit();
				changed = true;
				break;
			default:
				_server->sendToClient(client, ERR_UNKNOWNMODE(client.getNickname(), mode[i], params[0]) + "\r\n");
				return;
		}
	}
	if (changed)
		_server->broadcast(*chan, RPL_MODE(client.getNickname(), client.getUsername(), params[0], params[1], "") + "\r\n");
}
