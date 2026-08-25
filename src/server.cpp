#include "server.hpp"

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

const std::string &Server::getPassword() const
{
    return _password;
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

ACommand *Server::dispatch(Command cmd, bool isAuth, Client *client)
{
    if (isAuth)
    {
        if (isAuth && cmd.command == "QUIT")
            return new Quit(*this);
        if (isAuth && cmd.command == "PING")
            return new Ping(*this);
        if (isAuth && cmd.command == "KICK")
            return new Kick(*this);
        if (isAuth && cmd.command == "INVITE")
            return new Invite(*this);
        if (isAuth && cmd.command == "TOPIC")
            return new Topic(*this);
        if (isAuth && cmd.command == "JOIN")
            return new Join(*this);
        if (isAuth && cmd.command == "MODE")
            return new Mode(*this);
        if (isAuth && cmd.command == "NICK")
            return new Nick(*this);
        if (isAuth && cmd.command == "PRIVMSG")
            return new Privmsg(*this);
        if (isAuth && cmd.command == "NOTICE")
            return new Notice(*this);
    }
    else
    {
        if (cmd.command == "QUIT")
            return new Quit(*this);
        if (cmd.command == "PING")
            return new Ping(*this);
        if (cmd.command == "PASS")
            return new Pass(*this);
        if (cmd.command == "NICK")
            return new Nick(*this);
        if (cmd.command == "USER")
            return new User(*this);
        else
            sendToClient(*client, ERR_NOTREGISTERED(client->getNickname()) + "\r\n");
    }

    return NULL;
}

int Server::receiveClient(int i)
{
    bool disconnected = false;
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
                std::string commandLine = it->second->getBuffer().substr(0, pos);
                it->second->getBuffer().erase(0, pos + 1);

                if (!commandLine.empty() && commandLine[commandLine.size() - 1] == '\r')
                    commandLine.erase(commandLine.size() - 1);
                Command cmd = parseCommand(commandLine);
                std::map<int, Client *>::iterator clientIt = _clients.find(_fds[i].fd);
                if (clientIt != _clients.end())
                {
                    bool isAuth = clientIt->second->isRegistered();
                    ACommand *commandHandler = dispatch(cmd, isAuth, clientIt->second);
                    if (commandHandler)
                    {
                        commandHandler->execute(clientIt->second, cmd.params);
                        delete commandHandler;

                        if (_clients.find(_fds[i].fd) == _clients.end())
                        {
                            disconnected = true;
                            break;
                        }
                    }
                    else
                    {
                        sendToClient(*clientIt->second, ERR_UNKNOWNCOMMAND(clientIt->second->getNickname(), cmd.command) + "\r\n");
                    }
                }
            }
            else
                break;
        }
    }
    else
        std::cout << "client non trovato o errato" << std::endl;
    return disconnected ? -1 : 0;
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
                std::set<const Channel*> channelsCopy = it->second->getChannels();
                for (std::set<const Channel*>::iterator chIt = channelsCopy.begin(); chIt != channelsCopy.end(); ++chIt)
                {
                    Channel *chan = const_cast<Channel*>(*chIt);
                    chan->removeMember(*it->second);
                    removeEmptyChan(chan);
                }
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
    }
    return false;
}

Client *Server::returnClient(std::string value)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == value)
            return it->second;
    }
    return NULL;
}

Channel *Server::findChannel(const std::string &name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);

    if (it == _channels.end())
        return NULL;

    return it->second;
}

Channel *Server::createChannel(const std::string &name)
{
    Channel *channel = new Channel(name);

    _channels.insert(std::make_pair(name, channel));
    return channel;
}

void    Server::removeEmptyChan(Channel *channel){
    if (!channel->empty() || !channel)
        return;
    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++){
        if( it->second == channel)
        {
            delete channel;
            _channels.erase(it);
            return;
        }
    }
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

void Server::broadcast(const Channel &channel, const std::string &message, const Client *exclude)
{
    const std::set<const Client *> &members = channel.getMembers();
    for (std::set<const Client *>::const_iterator it = members.begin(); it != members.end(); it++)
    {
        if (*it == exclude)
            continue;
        sendToClient(const_cast<Client &>(**it), message);
    }
}
