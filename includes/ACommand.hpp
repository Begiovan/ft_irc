#ifndef ACOMMAND_HPP
# define ACOMMAND_HPP
#include "ft_irc.hpp"
#include "client.hpp"
#include "server.hpp"

class Server;
class Client;
struct Command;

class ACommand {
protected:
    Server *_server;
    bool _isauth;

public:
    explicit ACommand(Server &server, bool isAuth) : _server(&server), _isauth(isAuth) {}
    virtual ~ACommand() {}

    virtual void execute(Client *client, std::vector<std::string> args) = 0;
};

class Kick : public ACommand {
    public:
        Kick(Server &server, bool isAuth);
        ~Kick();
        void execute(Client *client, std::vector<std::string> params);
};

class Join : public ACommand {

    public:
        Join(Server &server, bool isAuth);
        ~Join();
        void execute(Client *client, std::vector<std::string> params);
};

class Topic : public ACommand {
    public:
        Topic(Server &server, bool isAuth);
        ~Topic();
        void execute(Client *client, std::vector<std::string> params);
};

class Invite : public ACommand {

        public:
        Invite(Server &server, bool isAuth);
        ~Invite();
        void execute(Client *client, std::vector<std::string> params);

};

#endif