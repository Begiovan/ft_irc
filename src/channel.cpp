#include "channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1) {}

Channel::~Channel() {}

void Channel::addMember(Client& client) {
    _members.insert(&client);
}

void Channel::removeMember(Client& client) {
    _members.erase(&client);
    _operators.erase(&client);
    _invited.erase(&client);
}

void Channel::inviteUser(Client& client) {
    _invited.insert(&client);
}

void Channel::removeInvite(Client& client) {
    _invited.erase(&client);
}

void Channel::addOperator(Client& client) {
    _operators.insert(&client);
}

void Channel::removeOperator(Client& client) {
    _operators.erase(&client);
}

bool Channel::empty() const {
    return _members.empty();
}

bool Channel::isMember(Client *client) const {
    return _members.find(client) != _members.end();
}

bool Channel::isOperator(Client& client) const {
    return _operators.find(&client) != _operators.end();
}

Channel::JoinResult Channel::canJoin(Client& other, const std::string& key) const {
    if (isMember(&other))
        return JOIN_ERR_ALREADY_IN;
    if (_inviteOnly && _invited.find(&other) == _invited.end())
        return JOIN_ERR_INVITE_ONLY;
    if (!_key.empty() && key != _key)
        return JOIN_ERR_BAD_KEY;
    if (_userLimit >= 0 && static_cast<int>(_members.size()) >= _userLimit)
        return JOIN_ERR_CHANNEL_FULL;
    return JOIN_OK;
}

bool Channel::canChangeTopic(Client& client) const {

	if (_topicRestricted)
		return isOperator(client);
	else
		return isMember(&client);
}

// Getters

const std::string& Channel::getName() const {
    return _name;
}

const std::set<Client*> &Channel::getMembers() const{
    return(_members);
}

const std::string& Channel::getTopic() const {
    return _topic;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

bool Channel::hasKey() const {
    return !_key.empty();
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

bool Channel::isInvited(Client& client) const {
    return _invited.find(&client) != _invited.end();
}

    // TOPIC 
void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

void Channel::setTopicRestricted(bool enabled) {
    _topicRestricted = enabled;
}

void Channel::clearUserLimit() {
    _userLimit = -1;
}

bool Channel::setUserLimit(int limit) {
    if (limit > 0)
    {
        _userLimit = limit;
        return true;
    }
        return false;
}

int Channel::getUserLimit() const{
    return _userLimit;
}

void Channel::setInviteOnly(bool enabled) {
    _inviteOnly = enabled;
}

void Channel::setKey(const std::string& key) {
    _key = key;
}

void Channel::clearKey() {
    _key = "";
}
