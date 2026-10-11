#pragma once

#include <string>

#include "ISocket.hpp"

class IServerContext {
 public:
  virtual ~IServerContext() {};
  virtual void removeSocket(ISocket* sock) = 0;
  virtual void insertSocket(ISocket* sock) = 0;
  virtual void updateSocket(ISocket* sock, unsigned int event, int op) = 0;
  virtual void onMessageReceived(ISocket* sock, std::string& message) = 0;
};
