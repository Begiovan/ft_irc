#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <iostream>


class Channel;

class Client {

	private:


		int _fd;
		int _registrationStatus;
		std::string _username;
		std::string _nickname;
		std::string _buffer;
		std::set<const Channel*> _channels;


	public:
		enum RegistrationFlag
		{
			PASS_OK = 1,
			NICK_OK = 2,
			USER_OK = 4
		};

		Client(int fd);
		//Client(int fd, std::string username, std::string nickname);
		Client(const Client &other);
		Client &operator=(const Client &other);
		~Client();

		int getFd() const;
		void setFd(int fd);

		const std::string &getUsername() const;
		void setUsername(std::string username);

		std::string &getNickname() ;
		void setNickname(std::string nickname);

		void addChannel(const Channel *chan) ;
		void removeChannel(const Channel *chan) ;
		const std::set<const Channel*>& getChannels() const;

		void appendBuffer(std::string message);
		std::string &getBuffer();
		void clearBuffer();

		int getRegistrationStatus() const;
		void setRegistrationStatus(RegistrationFlag flag);
		bool isRegistered() const;

};

#endif