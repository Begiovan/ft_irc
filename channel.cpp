#include "channel.hpp"

Channel::Channel(std::string name){
	_name = name;
	_topic = "";
	_key = "";

	_inviteOnly = false;
	_topicRestricted = false;
	_userLimit = -1;

}

Channel::Channel ( const Channel &other){
	if ( this != &other )
{		_name = other._name;
		_topic = other._topic;
		_key = other._topic;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_userLimit = other._userLimit;}
}

Channel & Channel::operator=(const Channel &other){
		if ( this != &other )
{		_name = other._name;
		_topic = other._topic;
		_key = other._topic;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_userLimit = other._userLimit;}
	return (*this);
}

Channel::~Channel(){}

const std::string Channel::getName() const{
	return _name;
}

const std::string& Channel::getTopic() const{
	return _topic;
}

void Channel::setTopic(std::string topic){
	_topic = topic;
}

bool Channel::hasMember() const{
	if (_members.size() > 0 )
		return true;
	return false;
}

bool Channel::isOperator(const Client &client) const{
	return _operators.find(client.getFd()) != _operators.end();
}

bool Channel::isInvited(const Client &client) const{
	return _invited.find(client.getFd()) != _invited.end();
}

// controlla pointer e se è un primo membro lo rende automaticamente OP
void Channel::addMember(Client* client){
	bool first;

	if (client == NULL)
		return;
	if (hasMember() == 0)
		first = true;
	_members.insert(client);
	if (first)
		_operators.insert(client);
}

void Channel::removeMember(const Client &client){
	_members.erase(client);
	_operators.erase(client);
	_invited.erase(client);
}

void Channel::addOperator(const Client &client){
	_operators.insert(client);
}

void Channel::removeOperator(const Client &client){
	_operators.erase(client);
}

void Channel::inviteUser(const Client &client){
	_invited.insert(client);
}

void Channel::removeInvite(const Client &client){
	_invited.erase(client);
}

bool Channel::canJoin(const Client &other, const std::string &key) const{
ì // controlla invite, key e userlimit

}

bool Channel::canChangeTopic(const Client &client) const{
	if (!_topicRestricted)
		return true;
	else if (_topicRestricted && isOperator(client))
		return true;
	else
		return false;
}

    void Channel::setInviteOnly(){
		_inviteOnly = !_inviteOnly;
	}
    void Channel::setTopicRestricted(){
		_topicRestricted = !_topicRestricted;
	}
    void Channel::setKey(const std::string& key){
		_key = key;
	}
	void Channel::clearKey(){
		_key = "";
	}

	void Channel::setUserLimit(int limit){
		_userLimit = limit;

	}
	void Channel::clearUserLimit(){
		_userLimit = -1;
	}

	bool Channel::empty() const{
		for (size_t i = 0; i < _members.size(); i++ )
			
	}