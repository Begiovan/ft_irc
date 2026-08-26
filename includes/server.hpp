#pragma once
#include "ft_irc.hpp"
#include "channel.hpp"
#include "client.hpp"
#include <map>
#include <string.h>
#include "irc_response.hpp"
#include "ACommand.hpp"

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
    
    public:
    Server(int port, const std::string &password);
    ~Server();
    
    const std::string &getPassword() const;
    
    void disconnectClient(int fd, const std::string &reason = "Connection closed");
    void setupSocket(int port);
    void run();
    bool findClient(std::string value);
    Client *returnClient(std::string value);
    Channel *findChannel(const std::string &name);
    Channel *createChannel(const std::string &name);
    void    removeEmptyChan(Channel *channel);

    // MESSAGGI
    void sendToClient(Client &client, const std::string &message); // TO DO
    void broadcast(const Channel &channel, const std::string &message, const Client *exclude = NULL);
    void flushClient(int fd);

    // COMANDI
    ACommand *dispatch(Command cmd, bool isAuth, Client *client);

};