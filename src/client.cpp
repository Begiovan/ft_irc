#include "client.hpp"

Client::Client(int fd){
	_fd = fd;
	_registrationStatus = 0;
}

Client::~Client(){}

int Client::getFd() const{
	return(_fd);
}

void Client::setFd(int fd){
	_fd = fd;
}

const std::string &Client::getUsername(){
	return _username;
}

void Client::setUsername(std::string username){
	if (_registrationStatus & USER_OK)
		_username = username;
	else
		{
			std::cout << "error" << std::endl;
		}
}

const std::string &Client::getNickname(){
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

std::string &Client::getBuffer(){
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

bool Client::isInChannel(const Channel *channel){
	return (_channels.find(channel) != _channels.end());
}

void Client::appendSendBuffer(std::string message){
	_sendBuffer += message;
}
std::string &Client::getSendBuffer(){
	return _sendBuffer;
}
void Client::clearSendBuffer(){
	_sendBuffer.erase();
}