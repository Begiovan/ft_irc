#pragma once

#include <set>
#include <string>

#include "client.hpp"

class Server;

class Channel {
private:
	std::string	_name;
	std::string _topic;
	std::string _key;

	std::set<Client*>		_members;
	std::set<Client*>			_operators;
	std::set<Client*>			_invited;

	bool	_inviteOnly;
	bool	_topicRestricted;
	int		_userLimit;

public:
	explicit Channel(const std::string& name);
	~Channel();

	// Membership

	void addMember(Client &client);
	void removeMember(Client &client);

	void inviteUser(Client &client);
	void removeInvite(Client &client);

	// OPERATOR
	void addOperator(Client &client);
	void removeOperator(Client &client);
	
	// check user

	enum JoinResult {
		JOIN_OK,
		JOIN_ERR_INVITE_ONLY,
		JOIN_ERR_BAD_KEY,
		JOIN_ERR_CHANNEL_FULL,
		JOIN_ERR_ALREADY_IN
	};
	
	bool		empty() const;
	bool		isMember(const Client &client) const;
	bool		isOperator(Client &client) const;
	JoinResult	canJoin(Client &other, const std::string& key) const;
	bool		canChangeTopic(Client &client) const;

	// Getters

	const std::string& getName() const;
	const std::set<Client*> &getMembers() const;

	// TOPIC 

	void				setTopic(const std::string& topic); // topic
	void				setTopicRestricted(bool enabled); // mode
	const std::string&	getTopic() const;
	bool				isTopicRestricted() const;	

	// USER LIMIT
	void	clearUserLimit();
	bool	setUserLimit(int limit);
	int		getUserLimit() const;

	// INVITE

	void	setInviteOnly(bool enabled);
	bool	isInvited(Client &client) const;
	bool	isInviteOnly() const;

	// KEY
	
	bool				hasKey() const;
	const std::string	&getKey() const;
	void				setKey(const std::string& key);
	void				clearKey();

};
