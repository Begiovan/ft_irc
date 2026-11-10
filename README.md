*This project was created as part of the 42 curriculum by skayed, begiovan, and rceschel.*



<div align="center">

# ft_irc
	
## Description
	
### A lightweight IRC server built from scratch in C++98

No external IRC libraries. Just raw sockets, `poll()`, and a lot of RFC reading.

[![C++](https://img.shields.io/badge/C%2B%2B-98-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.wikipedia.org/wiki/C%2B%2B98)
[![42](https://img.shields.io/badge/42-project-000000?style=for-the-badge&logo=42&logoColor=white)](https://42.fr)
[![Platform](https://img.shields.io/badge/platform-Linux%20-lightgrey?style=for-the-badge)](#)

</div>

---


<!-- TOC -->

## Table of Contents
- [Instructions](#instructions)
  - [Requirements](#requirements)
  - [Build](#build)
  - [Run](#run)
  - [Other ways to build and run](#other-ways-to-build-and-run)
  - [Connect to the server](#connect-to-the-server)
  - [Features](#features)
  - [Available commands](#available-commands)
  - [Channel modes](#channel-modes)
- [Technical overview](#technical-overview)
  - [Project structure](#project-structure)
  - [Server architecture](#server-architecture)
  - [Technical details](#technical-details)
  - [Known limitations](#known-limitations)
- [Resources](#resources)

<!-- /TOC -->


## Instructions

### Requirements
- A Unix-like OS (Linux)
- `make`
- A C++ compiler available as `c++` (GCC or Clang) with C++98 support: the code is compiled with `-Wall -Wextra -Werror -std=c++98`
- No external libraries: only the C++ standard library and POSIX system calls
- Optional: `valgrind`, needed only for `make valgrind`

### Build
Run `make` in the project root to compile the **ircserv** program.

### Run
Run `/path/to/ircserv` `<port>` `<password>`, where `<port>` is the port used to connect to the server (it must be greater than 1024) and `<password>` is the password required for authentication.

### Other ways to build and run
Another way to build and run the project is to use `make run` in the project root.

This will build the `ircserv` executable and automatically run it with the default overridable Makefile variables for `<port>` (`4242`) and `<password>` (`password`).

To override the default values: `make run PORT=<port> PASS=<password>`

You can also run `make valgrind`: it behaves like `make run`, but executes the program with **valgrind** to check for leaks.

### Connect to the server
Once the server is started with `ircserv <port> <password>`, you can connect with any IRC client you prefer.
During development and testing, our reference client has been **kvirc**.

### Features

Below is a list of the available features from a user perspective.

* You can authenticate, set a nickname and username, join a channel, and send and receive private messages.
* All messages sent from one client to a channel are forwarded to every other client that has joined the channel.
* You can have **operators** and regular users.
* There are commands specific to **channel operators**: KICK, INVITE, MODE (to change modes) and TOPIC (to change the topic when the channel is `+t`).
* You can transfer files (depending on the client) via CTCP DCC SEND.
* A bot is automatically added to every new channel; use `!help` in chat to see the list of available commands.

### Available commands

| Command   | Parameters                                       | Description                                            | Requires auth |
| --------- | ------------------------------------------------ | ------------------------------------------------------ | :-----------: |
| `PING`    | `<token>`                                        | Checks that the client-server connection is alive      |      Yes      |
| `QUIT`    | `[message]`                                      | Terminates the client's connection to the server       |      Yes      |
| `PASS`    | `<password>`                                     | Provides the password for server authentication        |    No (¹)     |
| `NICK`    | `<nickname>`                                     | Sets or changes the user's nickname                    |    No (²)     |
| `USER`    | `<username> <hostname> <servername> :<realname>` | Registers the user's username                          |   No (¹)(²)   |
| `PRIVMSG` | `<target> :<text>`                               | Sends a private message to a user or channel           |      Yes      |
| `NOTICE`  | `<target> :<text>`                               | Sends an automated notice (expects no automatic reply) |      Yes      |
| `KICK`    | `<channel> <user> [comment]`                     | Removes a user from a channel                          |      Yes      |
| `JOIN`    | `<channel> [key]`                                | Joins a channel (creates it if it does not exist)      |      Yes      |
| `TOPIC`   | `<channel> [topic]`                              | Views or sets a channel's topic                        |      Yes      |
| `INVITE`  | `<nickname> <channel>`                           | Invites a user to join a channel                       |      Yes      |
| `MODE`    | `<channel> [<+/-modes> [mode params]]`           | Views or changes channel modes (see below)             |      Yes      |

*Requires auth*: the client must have completed registration (`PASS`, `NICK` and `USER`), otherwise the server replies with `451 ERR_NOTREGISTERED`.

(¹) Accepted only **before** registration is complete; afterwards the server replies with `462 ERR_ALREADYREGISTERED`.

(²) Requires a correct `PASS` first, otherwise the server replies with `464 ERR_PASSWDMISMATCH`.

### Channel modes

| Flag | Effect                                                            |
| :--: | ----------------------------------------------------------------- |
| `i`  | Toggle Invite-only channel                                        |
| `t`  | Toggle the restrictions of the TOPIC command to channel operators |
| `k`  | Toggle the channel key (password)                                 |
| `o`  | Give or revoke channel operator privileges to a user             |
| `l`  | Toggle the user limit for the channel                              |

## Technical overview

### Project structure

```
.
├── Makefile
├── includes/
│   ├── ft_irc.hpp        # common system headers and utility prototypes
│   ├── server.hpp        # Server: listening socket, poll loop, clients, channels, bot
│   ├── client.hpp        # Client: fd, nickname/username, I/O buffers, registration flags
│   ├── channel.hpp       # Channel: members, operators, invited users, modes
│   ├── ACommand.hpp      # abstract command class + one derived class per IRC command
│   ├── parser.hpp        # Command struct and parseCommand()
│   └── irc_response.hpp  # numeric replies and message formats
└── src/
    ├── main.cpp          # argument validation, signal handling, server start
    ├── server.cpp        # socket setup, event loop, dispatch, broadcast, bot
    ├── client.cpp
    ├── channel.cpp
    ├── parser.cpp
    ├── ACommand.cpp      # shared checks (params, channel, membership, operator, nickname)
    ├── utils.cpp         # port and nickname validation
    └── pass.cpp, nick.cpp, user.cpp, ping.cpp, quit.cpp, join.cpp, privmsg.cpp,
        notice.cpp, kick.cpp, invite.cpp, topic.cpp, mode.cpp   # one file per command
```

### Server architecture

The server runs in a single process with a single event loop built on one `poll(2)` call:

```
poll() on every fd (listening socket + clients)
   |
   +-- listening socket POLLIN --> Server::acceptClient()
   |                                  accept(), O_NONBLOCK, new Client
   |
   +-- client POLLIN  --> Server::receiveClient()
   |                         recv() --> Client input buffer
   |                         for every complete line:
   |                            parseCommand()      --> Command { command, params }
   |                            Server::dispatch()  --> ACommand* (lookup in the command map)
   |                            registration check  --> 421 / 451 / 462 on failure
   |                            ACommand::execute()
   |                               sendToClient() / broadcast() --> Client send buffer
   |
   +-- client POLLOUT --> Server::flushClient() --> send()
   |
   +-- POLLHUP / POLLERR / POLLNVAL, recv() or send() <= 0, QUIT
   |                  --> Server::markForDisconnect()
   |
   end of iteration --> Server::disconnectClient() for every marked fd
```

Main steps:

1. Before each `poll()`, every client fd is watched for `POLLIN`; `POLLOUT` is added only when the client's send buffer is not empty.
2. When the listening socket is readable, the new connection is accepted, set to non-blocking and stored as a `Client`.
3. When a client is readable, the received bytes are appended to its input buffer. Every complete line (terminated by `\n`, with an optional `\r` removed) is processed; an incomplete line stays in the buffer until the rest arrives.
4. `parseCommand()` splits the line into the command name (converted to uppercase) and its parameters; a parameter starting with `:` is the trailing parameter and may contain spaces.
5. `Server::dispatch()` looks up the handler in a `std::map<std::string, ACommand*>`, filled once in the `Server` constructor. Before calling `execute()`, the server checks the command's registration requirement.
6. Replies are never sent directly: they are queued in the client's send buffer and written with `send()` only when `poll()` reports the socket as writable.
7. Disconnections are only marked during the loop and executed at the end of the iteration, so a `Client` is never freed while it is still in use.

---

### Technical details

- **Non-blocking I/O**: the listening socket and every client socket are set to non-blocking with `fcntl(fd, F_SETFL, O_NONBLOCK)`. The listening socket uses `SO_REUSEADDR` and accepts IPv4 connections on all interfaces.
- **A single `poll()`** monitors the listening socket and all clients, and every `accept()`, `recv()` and `send()` happens only after `poll()` reports the fd as ready: no threads, no `fork()`.
- **Partial data**: both input and output are buffered per client, so commands received in several pieces are rebuilt before being executed, and slow clients never block the server.
- **Line length**: lines are limited to 512 bytes including CRLF. A longer line, or more than 512 bytes without a newline, disconnects the client.
- **Registration**: the client state is a bitmask (`PASS_OK`, `NICK_OK`, `USER_OK`). `PASS` must come first; when all three flags are set, the server sends `001 RPL_WELCOME`. Nicknames must be unique, at most 9 characters long and follow the RFC character set.
- **Command pattern**: each IRC command is a class derived from `ACommand`, which stores its name, the minimum number of parameters and its registration requirement (`REGISTERED`, `UNREGISTERED` or `ANY`), and provides the common checks (`ERR_NEEDMOREPARAMS`, `ERR_NOSUCHCHANNEL`, `ERR_NOTONCHANNEL`, `ERR_CHANOPRIVSNEEDED`, `ERR_NOSUCHNICK`).
- **Channels**: a channel is created by the first `JOIN` on a name starting with `#` or `&`, and its creator becomes operator. On join the server broadcasts the `JOIN`, then sends the topic (`331`/`332`) and the member list (`353`/`366`, operators prefixed with `@`). A channel is deleted when no real users are left.
- **Bot (bonus)**: `chanbot` is an internal client without a socket that joins every new channel. It answers `!help`, `!channels` and `!date`, both in channels and in private messages, and it is not counted when checking whether a channel is empty.
- **File transfer (bonus)**: DCC SEND is negotiated through CTCP messages inside `PRIVMSG`, which the server forwards like any other message; the file itself is transferred directly between the two clients.
- **Disconnection handling**: both `QUIT` and unexpected disconnects (closed socket, client crash, socket error) notify the members of the client's channels with a `QUIT` message, remove the client from its channels, close the fd and free the `Client`.
- **Signals**: `SIGINT` and `SIGTERM` stop the main loop, so the `Server` destructor can free commands, channels and clients and close every fd. `SIGPIPE` is caught so that writing to a closed socket does not terminate the server.

### Known limitations

- No TLS/SSL (plain-text connections only).
- Only the commands listed above are implemented: no `PART` (a member leaves a channel only via `KICK` or `QUIT`), `WHO`, `WHOIS`, `LIST`, `NAMES` or user modes.
- `JOIN`, `PRIVMSG`, `NOTICE` and `KICK` accept a single target: comma-separated lists are not supported.
- Nicknames and channel names are case-sensitive.
- Single server only, without server-to-server communication (`SERVER` / `CONNECT`).

# Resources

- [RFC2812 stadnard](https://www.rfc-editor.org/info/rfc2812)
- [42Peer guide about ft_irc](https://irc.ircgod.com/)

# Ai Usage

Claude and Copilot were used to deepen our understanding of the core principles of the project, the kernel part and of course they helped draft and correct ( as well as translate ) this amazing README.