#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include "channel.hpp"

class Channel;

// TODO buffer di output/input

class Client {

	private:
		enum RegistrationFlag
		{
			PASS_OK = 1,
			NICK_OK = 2,
			USER_OK = 4
		};

		int _fd;
		std::string _username;
		std::string _nickname;
		std::set<Channel*> _channels;


	public:

		int _isOnline;

		Client();
		Client(int fd, std::string username, std::string nickname);
		Client(const Client &other);
		Client &operator=(const Client &other);
		~Client();

		int getFd();
		void setFd();
		std::string getUsername();
		std::string getNickname();

};

#endif