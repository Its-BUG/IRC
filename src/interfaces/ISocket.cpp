#include "interfaces/ISocket.hpp"

#include <unistd.h>

int ISocket::getFd() { return fd_; }
bool ISocket::isDisconnected() { return disconnect_; }
ISocket::~ISocket() { close(fd_); }