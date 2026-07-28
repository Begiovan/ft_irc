#pragma once
#include "ft_irc.hpp"
#include "channel.hpp"
#include "client.hpp"
#include <map>

class Client;

class Channel;

class Server {
private:
    int _serverSocket;
    std::string _password;
    std::vector<pollfd> _fds;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;

    pollfd makePollFd(int fd);
    void acceptClient();
    int reciveClient(int i);
    void disconnectClient(int fd);

public:
    Server(int port, const std::string &password);
    ~Server();

    void setupSocket(int port);
    void run();
};