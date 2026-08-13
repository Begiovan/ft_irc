#include "ACommand.hpp"
#include <sstream>

Mode::Mode(Server &server) : ACommand(server){}
Mode::~Mode(){}

// sintax MODE channel -> restituisce i mode attivi sul canale
// MODE channel +/- lettera valore -> setup su quella lettera del valore
// MODE channel piu lettere ->setup diverse lettere

static std::string getModes(Channel &channel){
	std::string mode;
	std::string args;

	if (channel.inviteRes())
		mode += "i";
	if (channel.topicRes())
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

void Mode::execute(Client *client, std::vector<std::string> params){
	if (params.empty())
	{
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE") + "\r\n");
		return;
	}

	Channel *chan = _server->findChannel(params[0]);
	if (!chan)
	{
		_server->sendToClient(*client, ERR_NOSUCHCHANNEL(client->getNickname(), params[0]) + "\r\n");
		return;
	}
	if (!chan->isMember(client)){
		_server->sendToClient(*client, ERR_NOTONCHANNEL(client->getNickname(), params[0]) + "\r\n");
		return;
	}
	if (params.size() == 1)
	{
		std::string modes = getModes(*chan);
		_server->sendToClient(*client, RPL_MODE(client->getNickname(), params[0], modes, "") + "\r\n");
		return;
	}
	if (!chan->isOperator(*client))
	{
		_server->sendToClient(*client, ERR_CHANOPRIVSNEEDED(client->getNickname(), params[0]) + "\r\n");
		return;
	}
	bool enable = true;
	size_t numArgs = 2;
	std::string mode = params[1];
	for (size_t i = 0; i < mode.size(); i++){
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
				break;
			case 't':
				chan->setTopicRestricted(enable);
				break;
			case 'o':
				if (params.size() <= numArgs)
				{
					_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE") + "\r\n");
					return;
				}
				{
					Client *target = _server->returnClient(params[numArgs]);
					if (!target)
					{
						_server->sendToClient(*client, ERR_NOSUCHNICK(client->getNickname(), params[numArgs]) + "\r\n");
						return;
					}
					if (!chan->isMember(target))
					{
						_server->sendToClient(*client, ERR_USERNOTINCHANNEL(client->getNickname(), target->getNickname(), params[0]) + "\r\n");
						return;
					}
					if (enable)
						chan->addOperator(*target);
					else
						chan->removeOperator(*target);
				}
				numArgs++;
				break;
			case 'k':
				if (enable)
				{
					if (params.size() <= numArgs)
					{
						_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE") + "\r\n");
						return;
					}
					chan->setKey(params[numArgs]);
					numArgs++;
				}
				else
					chan->clearKey();
				break;
			case 'l':
				if (enable)
				{
					if (params.size() <= numArgs)
					{
						_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE") + "\r\n");
						return;
					}
					if (!isPositiveInteger(params[numArgs]))
					{
						_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE") + "\r\n");
						return;
					}
					int limit = std::atoi(params[numArgs].c_str());
					chan->setUserLimit(limit);
					numArgs++;
				}
				else
					chan->clearUserLimit();
				break;
			default:
				_server->sendToClient(*client, ERR_UNKNOWNCOMMAND(client->getNickname(), "MODE") + "\r\n");
				return;
		}
	}
}