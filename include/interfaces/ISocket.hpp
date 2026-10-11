#pragma once

class IServerContext;
class ISocket {
 protected:
  int fd_;
  bool disconnect_;

 public:
  int getFd();
  bool isDisconnected();

  virtual void processEvent(unsigned int& revents, IServerContext& context) = 0;
  virtual ~ISocket();
};
