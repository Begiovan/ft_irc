#include "ft_irc.hpp"

int main()
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cout << "errore di creazione socket" << std::endl;
        return (-1);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cout << "errore di bind" << std::endl;
        return(-1);
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cout << "errore di listen" << std::endl;
        return(-1);
    }

    makePoll(serverSocket);
}