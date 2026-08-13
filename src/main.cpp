#include "ft_irc.hpp"
#include "server.hpp"
#include <cstdlib>

int main(int argc, char **argv)
{
    if(argc == 3)
    {
        int port;
        std::string password;

        if (!isPositiveInteger(argv[1]))
        {
            std::cerr << "Errore: porta non valida" << std::endl;
            return 1;
        }
        port = std::atoi(argv[1]);
        if (port <= 1024)
        {
            std::cerr << "inserire una porta con valore > 1024" << std::endl;
            return 1;
        }
        password = argv[2];
        try
        {
            Server server(port, password);
            server.run();
        } 
        catch (const std::exception &e)
        {
            std::cerr << "Errore: " << e.what() << std::endl;
            return 1;
        }
    }
    return(0);
}
