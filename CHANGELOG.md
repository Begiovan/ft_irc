# 15/10

## Server class
 - Refactored receiveClient() method
     - Readbility changes
     - Moved index parameter in fd parameter
     - Moved iterators to client's map in a Client pointer since only that was used
 - Added explicative comments and function descriptions

# 7/10

## Style
 - Formatted includes with tab in 42 style

## Server class
 - Moved some methods from public to private when public access specifier was useless
	- void	setupSocket(int port);
	- void	flushClient(int fd);
 - Added some minimal comments
