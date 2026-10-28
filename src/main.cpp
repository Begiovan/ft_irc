#include "ft_irc.hpp"
#include "server.hpp"
#include <cstdlib>
#include <csignal>

volatile __sig_atomic_t g_stopRequested = 0;

static void handleSignal(int signum)
{
	(void)signum;
	g_stopRequested = 1;
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	sa.sa_handler = handleSignal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if(argc == 3)
	{
		int port;
		std::string password;

		if (!isPositiveInteger(argv[1]))
		{
			std::cerr << "Error: invalid port" << std::endl;
			return 1;
		}
		port = std::atoi(argv[1]);
		if (port <= 1024)
		{
			std::cerr << "Please enter a port value greater than 1024" << std::endl;
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
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
	}
	else 
	{
		std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
	}
	return(0);
}
