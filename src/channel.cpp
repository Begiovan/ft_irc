#include "channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic(""), _key(""), _inviteOnly(false), _topicRestricted(false), _userLimit(-1) {}

Channel::~Channel() {}

void Channel::addMember(Client& client) {
    _members.insert(&client);
}

void Channel::removeMember(Client& client) {
    _members.erase(&client);
    _operators.erase(client.getFd());
    _invited.erase(client.getFd());
}

void Channel::inviteUser(const Client& client) {
    _invited.insert(client.getFd());
}

void Channel::removeInvite(const Client& client) {
    _invited.erase(client.getFd());
}

void Channel::addOperator(const Client& client) {
    _operators.insert(client.getFd());
}

void Channel::removeOperator(const Client& client) {
    _operators.erase(client.getFd());
}

bool Channel::empty() const {
    return _members.empty();
}

bool Channel::isMember(const Client *client) const {
    return _members.find(client) != _members.end();
}

bool Channel::isOperator(const Client& client) const {
    return _operators.find(client.getFd()) != _operators.end();
}

Channel::JoinResult Channel::canJoin(const Client& other, const std::string& key) const {
    if (isMember(&other))
        return JOIN_ERR_ALREADY_IN;
    if (_inviteOnly && _invited.find(other.getFd()) == _invited.end())
        return JOIN_ERR_INVITE_ONLY;
    if (!_key.empty() && key != _key)
        return JOIN_ERR_BAD_KEY;
    if (_userLimit >= 0 && static_cast<int>(_members.size()) >= _userLimit)
        return JOIN_ERR_CHANNEL_FULL;
    return JOIN_OK;
}

bool Channel::canChangeTopic(const Client& client) const {
    if (!_topicRestricted && isMember(&client))
        return true;
    return isOperator(client);
}

// Getters

const std::string& Channel::getName() const {
    return _name;
}

const std::set<const Client*> &Channel::getMembers() const{
    return(_members);
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

void Channel::setUserLimit(int limit) {
    if (limit > 0)
        _userLimit = limit;
    else
        std::cout << "invalid User limit" << std::endl;
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
