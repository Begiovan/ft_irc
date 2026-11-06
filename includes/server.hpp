#pragma once

#include <map>
#include <string.h>

#include "ft_irc.hpp"
#include "channel.hpp"
#include "client.hpp"
#include "irc_response.hpp"
#include "ACommand.hpp"

using std::string;

class ACommand;
class Client;
class Channel;

class Server {

	private:

	int								_serverSocket;
	int								_port;
	string							_password;
	std::vector<pollfd>				_fds; // _sockets; -> Name proposal
	std::map <int, Client*>			_clients;
	std::map <string, Channel*>		_channels;
	std::map <string, ACommand*> 	_commands;
	
	Client							*_bot;

	pollfd	makePollFd(int fd); // Can be static
	void	acceptClient();
	int		receiveClient(int fd);
	void	setupSocket(int port);
	void	flushClient(int fd);
	Client	*getClientByFd(int fd);
	string	buildChannelListMessage() const;
	void	createBot();
	bool	channelHasRealUsers(const Channel *channel) const;

	public:

	Server(int port, const string &password);
	~Server();
	
	const string &getPassword() const;
	
	void	disconnectClient(int fd, const string &reason = "Connection closed");
	void	run();
	Client	*getClientByNick(const string &value);
	Channel *findChannel(const string &name);
	Channel *createChannel(const string &name);
	void	removeEmptyChan(Channel &channel);
	Client	*getBot() const;
	void	botOnMessage(Client &from, const string &target, const string &message);

	// MESSAGGI
	void	sendToClient(Client &client, const string &message); // TO DO
	void	broadcast(const Channel &channel,
			const string &message, const Client *exclude = NULL);

	// COMANDI
	ACommand	*dispatch(string command);


	//modifiche 06/11/26

	void joinChannel(Client& client, Channel& chan);
	void partChannel(Client& client, Channel& chan);
	void broadcastToNeighbours(Client& client, const std::string& message, bool includeSelf);
	void sendTopic(Client& client, Channel& chan);
};
