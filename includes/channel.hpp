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

    void addMember(Client& client);
    void removeMember(Client& client);

    void inviteUser(const Client& client);
    void removeInvite(const Client& client);

    // OPERATOR
    void addOperator(const Client& client);
    void removeOperator(const Client& client);
    
    // check user

    enum JoinResult {
        JOIN_OK,
        JOIN_ERR_INVITE_ONLY,
        JOIN_ERR_BAD_KEY,
        JOIN_ERR_CHANNEL_FULL,
        JOIN_ERR_ALREADY_IN
    };
    
    bool empty() const;
    bool isMember(const Client *client) const;
    bool isOperator(const Client& client) const;
    JoinResult canJoin(const Client& other, const std::string& key) const;
    bool canChangeTopic(const Client& client) const;

    // Getters

    const std::string& getName() const;
    const std::set<const Client*> &getMembers() const;

    // TOPIC 

    void setTopic(const std::string& topic); // topic
    void setTopicRestricted(bool enabled); // mode
    const std::string& getTopic() const;
    bool topicRes() const;   

    // USER LIMIT
    void clearUserLimit();
    void setUserLimit(int limit);
    int getUserLimit() const;

    // INVITE

    void setInviteOnly(bool enabled);
    bool isInvited(const Client& client) const;
    bool inviteRes() const;

    // KEY
    
    bool hasKey() const;
    const std::string &getKey() const;
    void setKey(const std::string& key);
    void clearKey();

};

#endif