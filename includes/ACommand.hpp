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

public:
    explicit ACommand(Server &server) : _server(&server) {}
    virtual ~ACommand() {}

    virtual void execute(Client *client, std::vector<std::string> args) = 0;
};

class Kick : public ACommand {
    public:
        Kick(Server &server);
        ~Kick();
        void execute(Client *client, std::vector<std::string> params);
};

class Join : public ACommand {

    public:
        Join(Server &server);
        ~Join();
        void execute(Client *client, std::vector<std::string> params);
};

class Topic : public ACommand {
    public:
        Topic(Server &server);
        ~Topic();
        void execute(Client *client, std::vector<std::string> params);
};

class Invite : public ACommand {

        public:
        Invite(Server &server);
        ~Invite();
        void execute(Client *client, std::vector<std::string> params);

};

class Mode : public ACommand {

    public:
    Mode(Server &server);
    ~Mode();
    void execute(Client *client, std::vector<std::string> params);
};

#endif