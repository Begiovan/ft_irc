#include "../server.hpp"

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
    std::cout<<"server creato" << std::endl << "porta usata: "<<_port<<std::endl;
    std::cout<<"the password is: "<<password <<std::endl;
    Server::setupSocket(_port);
    _fds.push_back(makePollFd(_serverSocket));
}

Server::~Server()
{
    std::cout<<"Destructor called"<<std::endl;
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
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if(setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
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
        return ;    
    }
    if (fcntl(acc, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "errore di fcntl" << std::endl;
        perror("fcntl");
        close(acc);
        return ;
    }
    Client *client = new Client(acc);
    _clients[acc] = client;
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
    buffer[bytes] = '\0';

    std::cout << "client[" << i << "] >> ";
    for (int s = 0; s < bytes; s++)
        std::cout << buffer[s];
    return 0;
}

void Server::disconnectClient(int fd)
{
    close(fd);
    size_t i = 0;
    while(i < _fds.size())
        if(fd != Server::_fds[i].fd)
            i++;
        else
        {
            _fds.erase(_fds.begin() + i);
            break ;
        }
    
}

void Server::run()
{
    while (true)
    {
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
                    if(Server::receiveClient(i) < 0)
                        i--;
                }
            }
        }
    }
}

bool Server::findClient(std::string value){
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
    if (it->second->getNickname() == value)
        return true;

    if (it->second->getUsername() == value)
        return true;
    }
    return false;
}