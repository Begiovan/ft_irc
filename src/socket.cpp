#include "../ft_irc.hpp"

pollfd makePollFd(int fd)
{
    pollfd entry;
    entry.fd = fd;
    entry.events = POLLIN;
    entry.revents = 0;
    return entry;
}

int newClient(std::vector<pollfd> &fds, int serverSocket)
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    int acc;
    acc = accept(serverSocket, (sockaddr *)&addr, &addrLen);
    if (acc == -1)
    {
        std::cout << "errore di accept" << std::endl;
        //perror("accept");
        return (-1);
    }
    if (fcntl(acc, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "errore di fcntl" << std::endl;
        //perror("fcntl");
        close(acc);
        return (-1);
    }
    fds.push_back(makePollFd(acc));
    return (0);
}

int incomingMsg(std::vector<pollfd> &fds, int i)
{
    char buffer[1024];
    ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        close(fds[i].fd);
        fds.erase(fds.begin() + i);
        std::cout << "client: " << i << " disconnesso" << std::endl;
        return (-1);
    }
    buffer[bytes] = '\0';

    std::cout << "client[" << i << "] >> ";
    for (int s = 0; s < bytes; s++)
        std::cout << buffer[s];
    return (0);
}

void makePoll(int serverSocket)
{
    std::vector<pollfd> fds;

    fds.push_back(makePollFd(serverSocket));

    while (true)
    {
        poll(&fds[0], fds.size(), -1);

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == serverSocket)
                {
                    if (newClient(fds, serverSocket) == -1)
                        continue;
                    std::cout << "Client connesso" << std::endl;
                }
                else
                {
                    if (incomingMsg(fds, i) < 0)
                        i--;
                }
            }
        }
    }
}
