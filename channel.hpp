#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>
#include "client.hpp"

class Client;

class Server;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;

    std::set<const Client*> _members;
    std::set<int> _operators; // mi salvo gli fd degli operator
    std::set<int> _invited; // mi salvo gli fd degli invitati

    bool _inviteOnly;
    bool _topicRestricted;
	int _userLimit;

public:
    Channel(std::string name);
    Channel(const Channel& other);
    Channel &operator=(const Channel &other);
    ~Channel();

    const std::string getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string topic);

    bool hasMember() const;
    bool isOperator(const Client &other) const;
	bool isInvited(const Client &other) const;

    void addMember(Client* client);
    void removeMember(const Client& client);

    void addOperator(const Client &client);
    void removeOperator(const Client &client);

    void inviteUser(const Client &client);
	void removeInvite(const Client &client);

	bool canJoin(const Client &other, const std::string &key) const;
	bool canChangeTopic(const Client& client) const;

    void setInviteOnly(const Client &client);
    void setTopicRestricted(const Client &client);
    void setKey(const Client &client, const std::string& key);
	void clearKey(const Client &client);

    void setUserLimit(const Client &client, int limit);
	void clearUserLimit(const Client &client);

	bool empty() const;


};

#endif