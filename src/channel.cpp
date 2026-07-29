#include "../channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1) {}

Channel::Channel(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _topic = other._topic;
        _key = other._key;
        _inviteOnly = other._inviteOnly;
        _topicRestricted = other._topicRestricted;
        _userLimit = other._userLimit;
        _members = other._members;
        _operators = other._operators;
        _invited = other._invited;
    }
}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _topic = other._topic;
        _key = other._key;
        _inviteOnly = other._inviteOnly;
        _topicRestricted = other._topicRestricted;
        _userLimit = other._userLimit;
        _members = other._members;
        _operators = other._operators;
        _invited = other._invited;
    }
    return *this;
}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

bool Channel::hasMember() const {
    return !_members.empty();
}

bool Channel::isOperator(const Client& client) const {
    return _operators.find(client.getFd()) != _operators.end();
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
    _operators.insert(client.getFd());
}

void Channel::removeOperator(const Client& client) {
    _operators.erase(client.getFd());
	if(_operators.empty() && !_members.empty() )
		addOperator(**(_members.begin()));
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
    if (!_topicRestricted)
        return true;
    return isOperator(client);
}

void Channel::setInviteOnly(const Client& client) {
    if (!isOperator(client))
        return;
    _inviteOnly = !_inviteOnly;
}

void Channel::setTopicRestricted(const Client& client) {
    if (!isOperator(client))
        return;
    _topicRestricted = !_topicRestricted;
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
    _userLimit = limit;
}

void Channel::clearUserLimit(const Client& client) {
    if (!isOperator(client))
        return;
    _userLimit = -1;
}

bool Channel::empty() const {
    return _members.empty();
}
bool Channel::isMember(const Client *client){
	if (_members.find(client) != _members.end())
		return true;
	return false;
}