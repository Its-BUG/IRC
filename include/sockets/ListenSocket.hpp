#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "ClientSocket.hpp"
#include "interfaces/IServerContext.hpp"
#include "interfaces/ISocket.hpp"

#define MAXCONN 10

class ListenSocket : public ISocket {
  sockaddr_in addr;

 public:
  ListenSocket(std::string sport);

  void processEvent(unsigned int& revents, IServerContext& context);

  ~ListenSocket();
};