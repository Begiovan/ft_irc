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
  - [Build](#build)
  - [Run](#run)
  - [Other ways to build and run](#other-ways-to-build-and-run)
  - [Connect to the server](#connect-to-the-server)
  - [Features](#features)
  - [Available commands](#available-commands)
  - [Channel modes](#channel-modes)
- [Technical overview](#technical-overview)
  - [Server architecture](#server-architecture)
  - [Technical details](#technical-details)
  - [Known limitations](#known-limitations)
- [Resources](#resources)

<!-- /TOC -->


## Instructions

### Build
Run `make` in the project root to compile the **ircserv** program.

### Run
Run `/path/to/ircserv` `<port>` `<password>`, where `<port>` is the port used to connect to the server and `<password>` is the password required for authentication.

### Other ways to build and run
Another way to build and run the project is to use `make run` in the project root.

This will build the `ircserv` executable and automatically run it with the default overridable Makefile variables for `<port>` and `<password>`.

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
* There are commands specific to **channel operators** (KICK, INVITE, TOPIC, MODE):
* You can transfer files (depending on the client) via CTCP DCC SEND.
* A bot is automatically added to every new channel; use `!help` in chat to see the list of available commands.

### Available commands

| Command   | Parameters                                       | Description                                            | Requires auth |
| --------- | ------------------------------------------------ | ------------------------------------------------------ | :-----------: |
| `PING`    | `<token>`                                        | Checks that the client-server connection is alive      |               |
| `QUIT`    | `[message]`                                      | Terminates the client's connection to the server       |               |
| `PASS`    | `<password>`                                     | Provides the password for server authentication        |               |
| `NICK`    | `<nickname>`                                     | Sets or changes the user's nickname                    |               |
| `USER`    | `<username> <hostname> <servername> :<realname>` | Registers the user's username and real name            |               |
| `PRIVMSG` | `<target> :<text>`                               | Sends a private message to a user or channel           |               |
| `NOTICE`  | `<target> :<text>`                               | Sends an automated notice (expects no automatic reply) |               |
| `KICK`    | `<channel> <user> [comment]`                     | Removes a user from a channel                          |               |
| `JOIN`    | `<channel>[,channel*] [key]*`                    | Joins a channel                                        |               |
| `TOPIC`   | `<channel> [topic]`                              | Views or sets a channel's topic                        |               |
| `INVITE`  | `<nickname> <channel>`                           | Invites a user to join a channel                       |               |
| `MODE`    | `<channel>`                                      | Changes user or channel modes (e.g. +o, +t)            |               |

### Channel modes

| Flag | Effect                                                            |
| :--: | ----------------------------------------------------------------- |
| `i`  | Toggle Invite-only channel                                        |
| `t`  | Toggle the restrictions of the TOPIC command to channel operators |
| `k`  | Toggle the channel key (password)                                 |
| `o`  | Give or revoke channel operator privileges to a user             |
| `l`  | Toggle the user limit for the channel                              |

## Technical overview !!! TO REVIEW

### Server architecture

The core of the server is a single event loop based on `poll(2)`:

```
Client TCP
   |
   v
Server::acceptClient()
   |
   v
poll() on every fd
   |
   +-- POLLIN  --> Server::receiveClient()
   |                   |
   |                   v
   |               parseCommand()
   |                   |
   |                   v
   |               Server::dispatch()
   |                   |
   |                   v
   |               ACommand (Nick, Join, Privmsg, ...)
   |                   |
   |                   v
   |               Client::appendSendBuffer()
   |
   +-- POLLOUT --> Server::flushClient()  --> send() to client
```

Main steps:

1. Each loop updates the events to monitor (`POLLIN` is always watched, `POLLOUT` only when something needs to be sent).
2. For each fd ready for reading, bytes are accumulated in a per-client buffer until a full line (`\n`) is found.
3. The line is parsed into a `Command` object (command name + parameters).
4. `Server::dispatch()` instantiates the concrete command (Command pattern via `ACommand`).
5. Output is not written immediately with `send()`, but queued in a send buffer and flushed when the socket is ready for writing, avoiding blocking on slow clients.

---

### Technical details

- **Non-blocking I/O**: all sockets (server and clients) are configured with `fcntl(fd, F_SETFL, O_NONBLOCK)`.
- **A single `poll()`** monitors both the listening socket and all connected clients: no threads, no child processes.
- **Message buffering**: both inbound and outbound buffering are used to handle TCP correctly when data does not arrive as a complete line in a single `recv()`.
- **Command pattern**: each IRC command is a class derived from `ACommand`, instantiated dynamically in `Server::dispatch()` based on the received command name.
- **Disconnection handling**: both explicit `QUIT` and unexpected disconnects (socket closure, client crash) correctly notify other channel members with a `QUIT` message while keeping channel state consistent.

### Known limitations

- Does not implement TLS/SSL (plain-text connections)
- Does not implement all optional IRC commands (e.g. `WHOIS`, `LIST`, dedicated `PART`: a member leaves a channel only via `KICK` or `QUIT`)
- Designed for a single process / single server, without server federation (`SERVER` / `CONNECT`)

# Resources
