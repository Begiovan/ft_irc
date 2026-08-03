#include "../channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1) {}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const Client& executor, const std::string& topic) {
	if (canChangeTopic(executor))
    	_topic = topic;
}

bool Channel::empty() const {
    return _members.empty();
}

bool Channel::isOperator(const Client& client) const {
    if (isMember(&client))
        return _operators.find(client.getFd()) != _operators.end();
    return false;
}

bool Channel::isInvited(const Client& client) const {
    return _invited.find(client.getFd()) != _invited.end();
}

void Channel::addMember(Client& client) {
    const bool first = _members.empty();

    _members.insert(&client);
    client.addChannel(this);

    if (first)
        _operators.insert(client.getFd());
}

void Channel::removeMember(Client& client) {
    client.removeChannel(this);
    _members.erase(&client);
    _operators.erase(client.getFd());
    _invited.erase(client.getFd());
}

void Channel::addOperator(const Client& client) {
	if (isMember(&client))
    	_operators.insert(client.getFd());
	else
		std::cout << "client is not member of this channel" << std::endl;
}

void Channel::removeOperator(const Client& client) {
    if (isMember(&client) && isOperator(client))
        _operators.erase(client.getFd());
}

void Channel::inviteUser(const Client& client) {
    _invited.insert(client.getFd());
}

void Channel::removeInvite(const Client& client) {
    _invited.erase(client.getFd());
}

bool Channel::canJoin(const Client& other, const std::string& key) const {
    if (_inviteOnly && _invited.find(other.getFd()) == _invited.end())
        return false;
    if (!_key.empty() && key != _key)
        return false;
    if (_userLimit >= 0 && static_cast<int>(_members.size()) >= _userLimit)
        return false;
    return true;
}

bool Channel::canChangeTopic(const Client& client) const {
    if (!_topicRestricted && isMember(&client))
        return true;
    return isOperator(client);
}

void Channel::setInviteOnly(const Client& client, bool enabled) {
    if (!isOperator(client))
        return;
    _inviteOnly = enabled;
}

void Channel::setTopicRestricted(const Client& client, bool enabled) {
    if (!isOperator(client))
        return;
    _topicRestricted = enabled;
}

void Channel::setKey(const Client& client, const std::string& key) {
    if (!isOperator(client))
        return;
    _key = key;
}

void Channel::clearKey(const Client& client) {
    if (!isOperator(client))
        return;
    _key = "";
}

void Channel::setUserLimit(const Client& client, int limit) {
    if (!isOperator(client))
        return;
    if (limit > 0)
        _userLimit = limit;
    else
        std::cout << "invalid User limit" << std::endl;
}

void Channel::clearUserLimit(const Client& client) {
    if (!isOperator(client))
        return;
    _userLimit = -1;
}

bool Channel::isMember(const Client *client) const{
	return _members.find(client) != _members.end();
}
