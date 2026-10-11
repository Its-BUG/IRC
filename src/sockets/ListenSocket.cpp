#include "sockets/ListenSocket.hpp"

ListenSocket::ListenSocket(std::string sport) {
  this->disconnect_ = false;

  if (sport.find_first_not_of("0123456789") != std::string::npos)
    throw std::runtime_error("Invalid port");

  int port = std::atoi(sport.c_str());

  this->fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (this->fd_ < 0) throw std::runtime_error("Cannot Create Socket");

  int opt = 1;
  setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(this->fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
    throw std::runtime_error("Cannot Bind the Socket");

  if (listen(this->fd_, MAXCONN))
    throw std::runtime_error("Cannot make the socket passive");
}

void ListenSocket::processEvent(unsigned int& revents,
                                IServerContext& context) {
  sockaddr_in addr;
  socklen_t len = 0;

  if (revents & (EPOLLHUP | EPOLLERR)) {
    this->disconnect_ = true;
    return;
  }

  if (revents & EPOLLIN) {
    int newFd =
        accept4(fd_, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_NONBLOCK);
    if (newFd != -1) {
      ISocket* newClient = new ClientSocket(newFd, inet_ntoa(addr.sin_addr));
      context.updateSocket(newClient, EPOLLIN, EPOLL_CTL_ADD);
      context.insertSocket(newClient);
    }
  }
}

ListenSocket::~ListenSocket() {}