#include "../server.hpp"

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
    std::cout << "server creato" << std::endl
              << "porta usata: " << _port << std::endl;
    std::cout << "the password is: " << password << std::endl;
    Server::setupSocket(_port);
    _fds.push_back(makePollFd(_serverSocket));
}

Server::~Server()
{
    std::cout << "Destructor called" << std::endl;
}

void Server::setupSocket(int _port)
{
    this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_serverSocket == -1)
    {
        std::cout << "errore di creazione socket" << std::endl;
        perror("socket");
        throw std::runtime_error("socket failed");
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cout << "errore di setsockopt" << std::endl;
        perror("setsockopt");
        close(this->_serverSocket);
        throw std::runtime_error("setsockopt failed");
    }

    if (fcntl(this->_serverSocket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "errore di fcntl" << std::endl;
        perror("fcntl");
        close(this->_serverSocket);
        throw std::runtime_error("fcntl failed");
    }

    if (bind(this->_serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cout << "errore di bind" << std::endl;
        close(this->_serverSocket);
        throw std::runtime_error("bind failed");
    }

    if (listen(this->_serverSocket, 5) == -1)
    {
        std::cout << "errore di listen" << std::endl;
        close(this->_serverSocket);
        throw std::runtime_error("listen failed");
    }
}

pollfd Server::makePollFd(int fd)
{
    pollfd entry;
    entry.fd = fd;
    entry.events = POLLIN;
    entry.revents = 0;
    return entry;
}

void Server::acceptClient()
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    int acc;
    acc = accept(this->_serverSocket, (sockaddr *)&addr, &addrLen);
    if (acc == -1)
    {
        std::cout << "errore di accept" << std::endl;
        perror("accept");
        return;
    }
    if (fcntl(acc, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "errore di fcntl" << std::endl;
        perror("fcntl");
        close(acc);
        return;
    }
    Client *newClient = new Client(acc);
    _clients.insert(std::pair<int, Client *>(acc, newClient));
    this->_fds.push_back(makePollFd(acc));
}

int Server::receiveClient(int i)
{
    char buffer[1024];
    ssize_t bytes = recv(this->_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        Server::disconnectClient(this->_fds[i].fd);
        return -1;
    }
    std::string recived(buffer, bytes);
    std::map<int, Client *>::iterator it = _clients.find(_fds[i].fd);
    if (it != _clients.end())
    {
        it->second->appendBuffer(recived);
        while (true)
        {
            size_t pos = it->second->getBuffer().find('\n');
            if (pos != std::string::npos)
            {
                std::string command = it->second->getBuffer().substr(0, pos);
                it->second->getBuffer().erase(0, pos + 1);
                Command cmd = parseCommand(command);
                std::cout << "comando: " << cmd.command << std::endl;
                for (size_t p = 0; p < cmd.params.size(); p++)
                    std::cout << "parametro: " << cmd.params[p] << std::endl;
            }
            else
                break;
        }
    }
    else
        std::cout << "client non trovato o errato" << std::endl;
    return 0;
}

void Server::disconnectClient(int fd)
{
    close(fd);
    size_t i = 0;
    while (i < _fds.size())
        if (fd != Server::_fds[i].fd)
            i++;
        else
        {
            _fds.erase(_fds.begin() + i);
            std::map<int, Client *>::iterator it = _clients.find(fd);
            if (it != _clients.end())
            {
                delete (it->second);
                _clients.erase(it);
            }
            std::cout << "client [" << i << "] disconnesso" << std::endl;
            break;
        }
}

void Server::run()
{
    while (true)
    {
        for (size_t j = 0; j < _fds.size(); j++)
        {
            if (_fds[j].fd != _serverSocket)
            {
                std::map<int, Client *>::iterator it = _clients.find(_fds[j].fd);
                if (it != _clients.end())
                {
                    if (it->second->getSendBuffer().empty())
                        _fds[j].events = POLLIN;
                    else
                        _fds[j].events = POLLIN | POLLOUT;
                }
            }
        }
        poll(&_fds[0], _fds.size(), -1);

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == _serverSocket)
                {
                    Server::acceptClient();
                    std::cout << "Client connesso" << std::endl;
                }
                else
                {
                    if (Server::receiveClient(i) < 0)
                        i--;
                }
            }
            if (_fds[i].revents & POLLOUT)
            {
                Server::flushClient(_fds[i].fd);
            }
        }
    }
}

bool Server::findClient(std::string value)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == value)
            return true;

        if (it->second->getUsername() == value)
            return true;
    }
    return false;
}

Channel *Server::findChannel(const std::string &name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);

    if (it == _channels.end())
        return NULL;

    return it->second;
}

void Server::sendToClient(Client &client, const std::string &message)
{
    client.appendSendBuffer(message);
}

void Server::flushClient(int fd)
{

    std::map<int, Client *>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        ssize_t sent = send(fd, it->second->getSendBuffer().c_str(), it->second->getSendBuffer().size(), 0);
        if (sent > 0)
            it->second->getSendBuffer().erase(0, sent);
        else if (sent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            perror("send");
        }
    }
}

void Server::broadcast(const Channel &channel, const std::string &message)
{
    const std::set<const Client *> &members = channel.getMembers();
    for (std::set<const Client *>::const_iterator it = members.begin(); it != members.end(); it++)
        sendToClient(const_cast<Client &>(**it), message);
}

void Server::kick(const std::string &name, Client &executor, Client &target, std::string &reason)
{

    Channel *chan = findChannel(name);

    if (!chan)
    {
        sendToClient(executor,ERR_NOSUCHCHANNEL(executor.getNickname(), name) + "\r\n");
        return;
    }
    if (!(chan->isMember(&executor)))
    {
        sendToClient(executor, ERR_NOTONCHANNEL(executor.getNickname(), name)+ "\r\n");
        return;
    }
    if (!(chan->isOperator(executor)))
    {
        sendToClient(executor, ERR_CHANOPRIVSNEEDED(executor.getNickname(), name)+ "\r\n");
        return;
    }
    if (!(chan->isMember(&target)))
    {
        sendToClient(executor, ERR_USERNOTINCHANNEL(executor.getNickname(), target.getNickname(), name)+ "\r\n");
        return;
    }
    std::string message = RPL_KICK(executor.getNickname(), name, target.getNickname(), reason) + "\r\n";
    broadcast(*chan, message);
    chan->removeMember(target);
    target.removeChannel(chan);
}

void Server::join(const std::string &name, Client &client, const std::string &key)
{
    if (!client.isRegistered())
    {
        sendToClient(client, ERR_NOTREGISTERED(client.getNickname()) + "\r\n");
        return;
    }
    Channel *chan = findChannel(name);
    if (!chan)
    {
        chan = new Channel(name);
        _channels.insert(std::make_pair(name, chan));
        chan->addOperator(client);
    }
    Channel::JoinResult res = chan->canJoin(client, key);
    if (res != Channel::JOIN_OK)
    {
        std::string err;
        switch (res)
        {
        case Channel::JOIN_ERR_INVITE_ONLY:
            err = ERR_INVITEONLYCHAN(client.getNickname(), name);
            break;
        case Channel::JOIN_ERR_BAD_KEY:
            err = ERR_BADCHANNELKEY(client.getNickname(), name);
            break;
        case Channel::JOIN_ERR_CHANNEL_FULL:
            err = ERR_CHANNELISFULL(client.getNickname(), name);
            break;
        case Channel::JOIN_ERR_ALREADY_IN:
            err = ERR_USERONCHANNEL(client.getNickname(), name);
            break;
        default:
            err = ERR_UNKNOWNCOMMAND(client.getNickname(), "JOIN");
            break;
        }
        sendToClient(client, err + "\r\n");
        return;
    }

    chan->addMember(client);
    std::string message = RPL_JOIN(client.getNickname(), name) + "\r\n";
    broadcast(*chan, message);
    client.addChannel(chan);
}

void Server::topic(Client &executor, std::string &name, std::string &topic){
    Channel *chan = findChannel(name);

    if (!chan)
    {
        sendToClient(executor,ERR_NOSUCHCHANNEL(executor.getNickname(), name) + "\r\n");
        return;
    }
    if (!(chan->isMember(&executor)))
    {
        sendToClient(executor, ERR_NOTONCHANNEL(executor.getNickname(), name)+ "\r\n");
        return;
    }
    if(topic.empty()) // view topic 
    {
        // topic chan unset
        if (chan->getTopic().empty())
        {
            sendToClient(executor, RPL_NOTOPIC(executor.getNickname(), name)+ "\r\n");
            return;
        }
        // topic chan set
        else {
            sendToClient(executor, RPL_TOPIC(executor.getNickname(), name, chan->getTopic())+ "\r\n");
            return;
        }
    }
    else // set topic
    {
        if(!chan->canChangeTopic(executor))
        {
            sendToClient(executor, ERR_CHANOPRIVSNEEDED(executor.getNickname(), name)+ "\r\n");
            return;
        }
        else
        {
            chan->setTopic(executor, topic);
            std::string message = ":" + executor.getNickname() + " TOPIC " + name + " :" + chan->getTopic() + "\r\n";
            broadcast(*chan, message);
        }
    }
}