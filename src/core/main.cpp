#include <csignal>
#include <iostream>

#define MAXCONN 10

#include "core/Server.hpp"
#include "sockets/ListenSocket.hpp"
#include "sys/epoll.h"

bool Server::G_SERVER_RUNNING = true;
static void sigint_handler(int) { Server::G_SERVER_RUNNING = false; }

int main(int ac, char** av) {
  try {
    if (ac < 3) throw std::runtime_error("Usage: ./ircserver port password");

    signal(SIGINT, sigint_handler);
    ListenSocket lsock(av[1]);

    Server server(av[2]);
    server.updateSocket(&lsock, EPOLLIN, EPOLL_CTL_ADD);

    epoll_event events[MAXCONN];
    while (Server::G_SERVER_RUNNING) {
      int nfds = epoll_wait(server.getEpollFd(), events, MAXCONN, -1);
      for (int i = 0; i < nfds; i++) {
        ISocket* socket = static_cast<ISocket*>(events[i].data.ptr);
        socket->processEvent(events[i].events, server);
        if (socket->isDisconnected()) server.removeSocket(socket);
      }
    }
  } catch (std::exception& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
    return -1;
  }
}