# 8/10

## Server Class

### Header
 - Added comments of change proposal
     - _fds variable can be ranemae as _sockets variable 
     - makePollFd() method can become static

### Implementation
 - Changed the first and second for(;;) in run() method, extensively documented inline
 - Changed receiveClient() method to have receive the fd as parameter, not the index

# 7/10

## Style
 - Formatted includes with tab in 42 style

## Server class
 - Moved some methods from public to private when public access specifier was useless
    - void	setupSocket(int port);
	- void	flushClient(int fd);
 - Added some minimal comments
