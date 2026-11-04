#pragma once

#include "ft_irc.hpp"
#include "client.hpp"
#include "server.hpp"

class Server;
class Client;
struct Command;

typedef enum {
		REGISTERED,
		UNREGISTERED,
		ANY
} AuthRequirement;

class ACommand {
protected:

    Server					*_server;
	const std::string		_name;
    const unsigned int      _requiredParams;
    const AuthRequirement   _authRequirement;

	bool	hasEnoughParams_OrReply(Client &c, const std::vector<std::string> &params);
	Channel	*resolveChannel_OrReply(Client &c, const std::string &chName);
	bool	findMember_OrReply(Client &c, Channel &ch);
	bool	findOperator_OrReply(Client &c, Channel &ch);
	Client	*resolveNick_OrReply(Client &c, const std::string &nick);

public:


    explicit ACommand(Server &server, std::string name, unsigned int requiredParams, AuthRequirement authRequirement = REGISTERED) :
		_server(&server), _name(name), _requiredParams(requiredParams), _authRequirement(authRequirement) {}

    virtual ~ACommand() {}

    AuthRequirement	authRequirement();

    virtual void execute(Client &client, const std::vector<std::string> &args) = 0;
};

/******** COMMANDS ***********/

class Ping : public ACommand {
    public:
        Ping(Server &server);
        ~Ping();
        void execute(Client &client, const std::vector<std::string> &params);
};


class Quit : public ACommand {
    public:
    Quit(Server &server);
    ~Quit();
    void execute(Client &client, const std::vector<std::string> &params);
};

class Pass : public ACommand {
    public:
    Pass(Server &server);
    ~Pass();
    void execute(Client &client, const std::vector<std::string> &params);
};

class Nick : public ACommand {
    public:
    Nick(Server &server);
    ~Nick();
    void execute(Client &client, const std::vector<std::string> &params);
};

class User : public ACommand {
    public:
    User(Server &server);
    ~User();
    void execute(Client &client, const std::vector<std::string> &params);
};

class Privmsg : public ACommand {
    public:
        Privmsg(Server &server);
        ~Privmsg();
        void execute(Client &client, const std::vector<std::string> &params);
};

class Notice : public ACommand {
    public:
        Notice(Server &server);
        ~Notice();
        void execute(Client &client, const std::vector<std::string> &params);
};

class Kick : public ACommand {
    public:
        Kick(Server &server);
        ~Kick();
        void execute(Client &client, const std::vector<std::string> &params);
};

class Join : public ACommand {

    public:
        Join(Server &server);
        ~Join();
        void execute(Client &client, const std::vector<std::string> &params);
};

class Topic : public ACommand {
    public:
        Topic(Server &server);
        ~Topic();
        void execute(Client &client, const std::vector<std::string> &params);
};

class Invite : public ACommand {

        public:
        Invite(Server &server);
        ~Invite();
        void execute(Client &client, const std::vector<std::string> &params);

};

class Mode : public ACommand {

    public:
    Mode(Server &server);
    ~Mode();
    void execute(Client &client, const std::vector<std::string> &params);
};
