#pragma once

#include "irc_response.hpp"

#include <string>
#include <set>
#include <iostream>

class Channel;

class Client {

	private:
		int							_fd;
		int							_registrationStatus;
		std::string					_username;
		std::string					_nickname;
		std::string 				_buffer;		//recv()
		std::string 				_sendBuffer;	//send()
		std::set<Channel*>			_channels;
		bool						_isBot;


	public:
		enum RegistrationFlag
		{
			PASS_OK = 1,
			NICK_OK = 2,
			USER_OK = 4
		};

		Client(int fd);
		~Client();

		int		getFd() const;

		const std::string	&getUsername() const;
		void				setUsername(std::string username);

		const std::string	&getNickname() const;
		void				setNickname(std::string nickname);
		std::string		nickOrStar() const;
		std::string		prefix() const;
		void					addChannel(Channel *chan) ;
		void					removeChannel(Channel *chan) ;
		const std::set<Channel*>	&getChannels() const;
		bool					isInChannel(Channel *channel) const;
		bool					isBot() const;
		void					setBot(bool isBot);

		void		appendBuffer(std::string message);
		std::string	&getBuffer();

		void		appendSendBuffer(std::string message);
		std::string &getSendBuffer();

		int		getRegistrationStatus() const;
		void	setRegistrationStatus(RegistrationFlag flag);
		bool	isRegistered() const;

};
