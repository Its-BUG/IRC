# *This project has been created as part of the 42 curriculum by <sessaki>, <iboubker>.<saamouss>*

# ft_irc

## Description

`ft_irc` is a C++ project whose goal is to implement a simplified **Internet Relay Chat (IRC) server** compatible with the IRC protocol (RFC 1459). The server allows multiple clients to connect simultaneously, authenticate, communicate through private messages, and interact inside channels.

The project focuses on low-level network programming, event-driven server architecture, socket programming, and object-oriented design in C++98.

### Main Objectives

* Implement a TCP IRC server.
* Handle multiple clients simultaneously without blocking.
* Respect the IRC protocol for supported commands.
* Manage users, nicknames, channels, and permissions.
* Communicate with standard IRC clients (e.g. HexChat).

---

## Features

### Mandatory

* TCP server using non-blocking sockets.
* Event-driven architecture using `epoll()`.
* Password authentication (`PASS`).
* User registration (`NICK`, `USER`).
* Private messaging (`PRIVMSG`).
* Channel management.
* IRC replies and error messages.
* Compatible with HexChat and other IRC clients.

### Bonus (if implemented)

* IRC Bot
* File transfer (DCC)

---

## Instructions

## Compilation

```bash
make
```

### Run

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 password
```

---

## Connecting with HexChat

1. Open HexChat.
2. Add a new network.
3. Set:

```
Host : localhost/<port>

Example:

localhost/6667
```

4. Connect.
5. Register using:

```irc
PASS password
NICK Alice
USER alice 0 * :Alice
```

---

## Supported Commands

Currently implemented:

* PASS
* NICK
* USER
* JOIN
* PART
* PRIVMSG
* QUIT
* PING
* TOPIC
* MODE
* INVITE
* KICK

---

## Example Usage

Private message

```irc
PRIVMSG Bob :Hello!
```

Join a channel

```irc
JOIN #42
```

Leave a channel

```irc
PART #42
```

Change nickname

```irc
NICK NewName
```

---

## Technical Choices

This project follows an event-driven architecture.

Main technologies used:

*
* TCP
* epoll
* Non-blocking sockets
* Object-Oriented Programming
* STL containers (`std::map`, `std::set`, `std::vector`)

Main components:

* Server
* ListenSocket
* ClientSocket
* Channel
* Parser
* Command Dispatcher

---

## Resources

### IRC Documentation

* RFC 1459 — Internet Relay Chat Protocol
* Socket Programming

- https://www.alien.net.au/irc/irc2numerics.html
- https://man7.org/linux/man-pages/man7/epoll.7.html
- https://www.linuxhowtos.org/C_C++/socket.htm

### epoll

* `man epoll`
* `man epoll_ctl`
* `man epoll_wait`

---

## AI Usage

Artificial intelligence was used **only as a learning and documentation assistant**.

It was used for:

* Understanding the IRC protocol.
* Learning socket programming concepts.
* Understanding `epoll()`, non-blocking sockets, and event-driven programming.
* Clarifying RFC specifications.
* Explaining C++ language features.
* Debugging and understanding compiler/runtime errors.
* Reviewing the architecture and discussing possible implementations.

All project design decisions, implementation, testing, debugging, and final code integration were completed manually by the project authors.

---