#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <poll.h>

pollfd makePollFd(int fd)
{
    pollfd entry;
    entry.fd = fd;
    entry.events = POLLIN;
    entry.revents = 0;
    return entry;
}

int main()
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1)
    {
        std::cout<<"errore di creazione socket"<< std::endl;
        perror("socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1 )
    {
        std::cout<<"errore di bind"<< std::endl;
        perror("bind");
    }

    listen(serverSocket, 5);

    std::vector<pollfd> fds;

    fds.push_back(makePollFd(serverSocket));


    while (true)
    {
        poll( &fds[0], fds.size(), -1 );

        for(size_t i = 0; i < fds.size(); i++)
        {
            if ( fds[i].revents & POLLIN )
            {
                if ( fds[i].fd == serverSocket )
                {
                    sockaddr_in addr;
                    socklen_t addrLen = sizeof(addr);
                    int acc;

                    acc = accept(serverSocket, (sockaddr *)&addr, &addrLen);
                    if(acc == -1)
                    {
                        std::cout<<"errore di accept"<< std::endl;
                        perror("accept");
                    }
                    fds.push_back(makePollFd(acc));
                }
                else
                {
                    char buffer[1024];
                    ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);                    if (bytes > 0)
                        buffer[bytes] = '\0';

                    for(int s = 0; s < bytes; s++)
                        std::cout<< buffer[s];
                }
            }
        }
    }

}