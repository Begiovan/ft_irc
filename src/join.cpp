#include "ACommand.hpp"

Join::Join(Server &server, bool isAuth) : ACommand(server, isAuth){}
Join::~Join(){}

// sintax: JOIN channel key (optional)

void Join::execute(Client *client, std::vector<std::string> params){

    	if (params.size() == 1){
		_server->sendToClient(*client, ERR_NEEDMOREPARAMS(client->getNickname(), "KICK")+ "\r\n");
		return;
	}
	Channel *chan = _server->findChannel(params[0]);
	if(!chan){
		chan = new Channel(params[0]);
		_server->_channels.insert(std::make_pair(params[0], chan));
		chan->addMember(*client);
		chan->addOperator(*client);
	}
	Channel::JoinResult res = chan->canJoin(*client, params[1]);
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
            err = ERR_USERONCHANNEL(client->getNickname(), params[0]);
            break;
        default:
            err = ERR_UNKNOWNCOMMAND(client->getNickname(), "JOIN");
            break;
        }
		_server->sendToClient(*client, err + "\r\n");
		return;
	}
	chan->addMember(*client);
	_server->broadcast(*chan,  RPL_JOIN(client->getNickname(), params[0]) + "\r\n" );
	client->addChannel(chan);
	    if (chan->getTopic().empty())
        _server->sendToClient(*client, RPL_NOTOPIC(client->getNickname(), params[0]) + "\r\n");
    else
        _server->sendToClient(*client, RPL_TOPIC(client->getNickname(), params[0], chan->getTopic()) + "\r\n");

    std::string users;
    const std::set<const Client *> &members = chan->getMembers();
    for (std::set<const Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (!users.empty())
            users += ' ';
        if (chan->isOperator(**it))
            users += '@';
        users += const_cast<Client *>(*it)->getNickname();
    }
    _server->sendToClient(*client, RPL_NAMREPLY(client->getNickname(), params[0], users) + "\r\n");
    _server->sendToClient(*client, RPL_ENDOFNAMES(client->getNickname(), params[0]) + "\r\n");
	
}