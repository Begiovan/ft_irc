#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include "client.hpp"

class Server;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;

    std::set<const Client*> _members;
    std::set<int> _operators;
    std::set<int> _invited;

    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;

public:
    explicit Channel(const std::string& name);
    ~Channel();

    // Membership
    // void join(Client& client, const std::string& key);
    // void part(const Client& client);

    // OP commands

    //void kick(const Client& executor, const Client& target);
    //void invite(const Client& executor, const Client& target);
    void setTopic(const Client& executor, const std::string& topic);    


    void addMember(Client& client);
    void removeMember(Client& client);

    void addOperator(const Client& client);
    void removeOperator(const Client& client);

    void inviteUser(const Client& client);
    void removeInvite(const Client& client);

    bool canJoin(const Client& other, const std::string& key) const;
    bool canChangeTopic(const Client& client) const;

    // Modes

    void setInviteOnly(const Client& client, bool enabled);
    void setTopicRestricted(const Client& client, bool enabled);
    void setKey(const Client& client, const std::string& key);
    void clearKey(const Client& client);

    void setUserLimit(const Client& client, int limit);
    void clearUserLimit(const Client& client);

    // check

    bool empty() const;
    bool isMember(const Client *client) const;
    bool isOperator(const Client& client) const;
    bool isInvited(const Client& client) const;

    // Getters
    const std::string& getName() const;
    const std::string& getTopic() const;


};

#endif