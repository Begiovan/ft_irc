#pragma once
#include "ft_irc.hpp"
#include "channel.hpp"
#include "client.hpp"
#include <map>
#include <string.h>
#include "irc_response.hpp"

class ACommand;
class Client;
class Channel;

class Server {
private:
    int _serverSocket;
    int _port;
    std::string _password;
    std::vector<pollfd> _fds;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;

    pollfd makePollFd(int fd);
    void acceptClient();
    int receiveClient(int i);
    void disconnectClient(int fd);

public:
    Server(int port, const std::string &password);
    ~Server();

    void setupSocket(int port);
    void run();
    bool findClient(std::string value);
    Client *returnClient(std::string value);
    Channel *findChannel(const std::string &name);

    // MESSAGGI
    void sendToClient(Client &client, const std::string &message); // TO DO
    void broadcast(const Channel &channel, const std::string &message);
    void flushClient(int fd);

    // COMANDI
    ACommand *dispatch(Command cmd);

};