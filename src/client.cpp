#include "../client.hpp"

Client::Client(int fd){
	_fd = fd;
	_registrationStatus = 0;
}

// Client::Client(int fd, std::string username, std::string nickname) : _fd(fd), _username(username), _nickname(nickname){
// 	_registrationStatus = USER_OK;
// }

Client::Client(const Client &other){
	if (this != &other)
	{
		_fd = other._fd;
		_username = other._username;
		_nickname = other._nickname;
		_channels = other._channels;
		_registrationStatus = other._registrationStatus;
		_buffer = other._buffer;

	}
}

Client &Client::operator=(const Client &other){
		if (this != &other)
	{
		_fd = other._fd;
		_username = other._username;
		_nickname = other._nickname;
		_channels = other._channels;
		_registrationStatus = other._registrationStatus;
		_buffer = other._buffer;

	}
	return (*this);
}

Client::~Client(){}

int Client::getFd() const{
	return(_fd);
}

void Client::setFd(int fd){
	_fd = fd;
}

const std::string &Client::getUsername() const{
	return _username;
}

void Client::setUsername(std::string username){
	_username = username;
}

std::string &Client::getNickname(){
	return _nickname;
}

void Client::setNickname(std::string nickname){
	_nickname = nickname;
}

void Client::addChannel(const Channel *chan){

	_channels.insert(chan);
}

void Client::removeChannel(const Channel *chan) {

	_channels.erase(chan);
}

const std::set<const Channel*>& Client::getChannels() const{
	return _channels;
}

void Client::appendBuffer(std::string message){
	_buffer += message;
}

std::string &Client::getBuffer() const{
	return _buffer;
}

void Client::clearBuffer(){
	_buffer.erase();
}

int Client::getRegistrationStatus() const{
	return _registrationStatus;
}

void Client::setRegistrationStatus(RegistrationFlag flag){
	_registrationStatus |= flag;
}

bool Client::isRegistered() const {
	    return (_registrationStatus
        & (PASS_OK | NICK_OK | USER_OK))
        == (PASS_OK | NICK_OK | USER_OK);
}