#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>

#include <iomanip>
#include <set>
#include <sstream>
#include <string>

#include "interfaces/IServerContext.hpp"
#include "interfaces/ISocket.hpp"

#define SERVER_NAME ":myIRCserver "


class Channel;
class ClientSocket : public ISocket {
 private:
  std::stringstream outBuffer_;
  std::string inBuffer_;

  std::string nickname_;
  std::string username_;
  std::string realname_;
  std::string hostname_;

  int status_;

  bool passwordAccepted_;
  bool nicknameSet_;
  bool usernameSet_;
  bool registered_;

  std::set<Channel*> channels_;

  bool pendingDisconnect_;

 public:
  ClientSocket(int fd, const char* hostname);

  int getStatus(void);
  std::string getNickName(void) const;
  std::string getUserName(void) const;
  std::string getRealName(void) const;
  std::string getHostName(void) const;

  void setNickName(std::string& nickname);
  void setUserName(std::string& username);
  void setRealName(std::string& realname);
  void setPasswordAccepted();
  void updateRegistration();
  bool isRegistered() const;

  void processEvent(unsigned int& revents, IServerContext& context);

  int send_(void);
  int recv_(void);

  ClientSocket& operator<<(int numericCode);
  ClientSocket& operator<<(const std::string& text);
  ClientSocket& operator<<(ClientSocket* client);

  void joinChannel(Channel* channel);
  void leaveChannel(Channel* channel);
  const std::set<Channel*>& getJoinedChannels() const;

  void markForDisconnect();

  ~ClientSocket();
};
