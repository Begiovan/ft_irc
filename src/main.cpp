#include "../ft_irc.hpp"

int main()
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cout << "errore di creazione socket" << std::endl;
        //perror("socket");
        return (-1);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cout << "errore di setsockopt" << std::endl;
        //perror("setsockopt");
        close(serverSocket);
        return (-1);
    }

    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "errore di fcntl" << std::endl;
        //perror("fcntl");
        close(serverSocket);
        return (-1);
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cout << "errore di bind" << std::endl;
        close(serverSocket);
        return(-1);
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cout << "errore di listen" << std::endl;
        close(serverSocket);
        return(-1);
    }
	std::cout<< "server" << std::endl << std::endl;
    makePoll(serverSocket);
}
