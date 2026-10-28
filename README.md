*This project has been created as part of the 42 curriculum by skayed, begiovan, rceschel*

# Descpription
This project aims to build a minimal, but functional, IRC server, without the federation with other servers.

# Instruction

## Build and run

#### Build
Run `make` in the root of the project, you will compile the **ircserv** program.

####  Execute
Run `/path/to/ircserv <port> <password>` where `<port>` is the port where to connect to the server, and `<password>` is the password used to authenticate to the server.

#### Other ways to build and run
Another way to build and run the project is to use `make run` in the root of the project

In this way, you will build the `ircserv` executable and automatically run it with defauld overridable makefile's variable for `<port>` and `<password>`.

To override the defualt values: `make run PORT=<port> PASS=<password>`

You can also run `make valgrind`: it works at the same as `make run`, but runs the executable with **valgrind** to perform leaks check

## Usage

#### Connect to the server
Once the server started with `ircserv <port> <password>`, you can connect to the server with the client do you prefer.
During develop and testing, our reference client has been **kvirc**.

#### Features

Below here's a list of the implemented features, from user perspective.

* You can authenticate, set a nickname, a username, join a channel, send and receive private messages.
* All the messages sent from one client to a channel will be forwarded to every other client that joined the channel.
* You can have **operators** and regular users.
* There are commands specific to **channel operators**
	* KICK - Eject a client from the channel
	* INVITE - Invite a client to a channel
	* TOPIC - Change or view the channel topic
	* MODE - Change the channel’s mode *(prefix '+' to set/give, '-' to remove/take)*:
		* i: Set/remove Invite-only channel
		* t: Set/remove the restrictions of the TOPIC command to channel operators
		* k: Set/remove the channel key (password)
		* o: Give/take channel operator privilege
		* l: Set/remove the user limit to channel
* You can transfer files (depending on the client) trought CTCP DCC SEND
* A bot will be automatically added to every new channel, use `!help` in chat to have the list of aviable commands.

#### Commands

Below here's a comprehensive list of available server's commands.

| Command | Description | Parameters |
| --- | --- | --- |
| **PING** | Checks that the client-server connection is alive | `<token>` |
| **QUIT** | Terminates the client's connection to the server | `[message]` |
| **PASS** | Provides the password for server authentication | `<password>` |
| **NICK** | Sets or changes the user's nickname | `<nickname>` |
| **USER** | Registers the user's username and real name | `<username> <hostname> <servername> :<realname>` |
| **PRIVMSG** | Sends a private message to a user or channel | `<target> :<text>` |
| **NOTICE** | Sends an automated notice (expects no automatic reply) | `<target> :<text>` |
| **KICK** | Removes a user from a channel | `<channel> <user> [comment]` |
| **JOIN** | Joins a channel | `<channel>[,channel*] [key]*` |
| **TOPIC** | Views or sets a channel's topic | `<channel> [topic]` |
| **INVITE** | Invites a user to join a channel | `<nickname> <channel>` |
| **MODE** | Changes user or channel modes (e.g. +o, +t) | `<channel\|user> <mode> [arguments]` |


# Resources
