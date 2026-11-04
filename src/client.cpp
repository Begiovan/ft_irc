#include "client.hpp"

Client::Client(int fd){
	_fd = fd;
	_registrationStatus = 0;
	_isBot = false;
}

Client::~Client(){}

int Client::getFd() const{
	return(_fd);
}

const std::string &Client::getUsername() const{
	return _username;
}

void Client::setUsername(std::string username){
    _username = username;
}

const std::string &Client::getNickname() const{
	return _nickname;
}
std::string Client::nickOrStar() const {
	return _nickname.empty() ? "*" : _nickname;
}

std::string Client::prefix() const {
	return _nickname.empty() ? "*" : _nickname + "!" + _username + "@" SERVER_NAME;
}
void Client::setNickname(std::string nickname){
	_nickname = nickname;
}

void Client::addChannel(Channel *chan){

	_channels.insert(chan);
}

void Client::removeChannel(Channel *chan) {

	_channels.erase(chan);
}

const std::set<Channel*>& Client::getChannels() const{
	return _channels;
}

void Client::appendBuffer(std::string message){
	_buffer += message;
}

std::string &Client::getBuffer(){
	return _buffer;
}

int Client::getRegistrationStatus() const{
	return _registrationStatus;
}

void Client::setRegistrationStatus(RegistrationFlag flag){
	_registrationStatus |= flag;
}

bool Client::isRegistered() const {
	    return ( _registrationStatus & (PASS_OK | NICK_OK | USER_OK) )
			== ( PASS_OK | NICK_OK | USER_OK );
}

bool Client::isInChannel(const Channel *channel) const{
	return (_channels.find(const_cast<Channel *>(channel)) != _channels.end());
}

bool Client::isBot() const{
	return _isBot;
}

void Client::setBot(bool isBot){
	_isBot = isBot;
}

void Client::appendSendBuffer(std::string message){
	_sendBuffer += message;
}
std::string &Client::getSendBuffer(){
	return _sendBuffer;
}
