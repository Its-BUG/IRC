#pragma once

#include <cstddef>
#include <set>
#include <string>

#define CRLF "\r\n"

class IServerContext;
class ClientSocket;

enum {
  USERS_LIMIT = 0x01,
  INVITE_ONLY = 0x02,
  CHANNEL_LOCKED = 0x04,
  TOPIC_RESTRICTED = 0x08,
};

class Channel {
  std::string name_;
  std::string topic_;
  std::string password_;

  size_t usersLimit_;
  size_t modes_;

  std::set<ClientSocket*> members_;
  std::set<ClientSocket*> invitees_;
  std::set<ClientSocket*> operators_;

 public:
  Channel(std::string name);

  void broadcastMessage(IServerContext* context, ClientSocket* sender,
                        const std::string& command, const std::string& target,
                        const std::string& message);

  void setUserLimit(size_t limit);
  void setTopic(const std::string& topic);
  void setPassword(const std::string& password);

  const std::string& getName() const;
  const std::string& getTopic() const;

  size_t getModes() const;
  void setMode(size_t mode);
  void unsetMode(size_t mode);
  std::string getModeString() const;

  const std::set<ClientSocket*>& getMembers() const;
  void removeMember(ClientSocket* member);
  void addMember(ClientSocket* member);

  const std::set<ClientSocket*>& getOperators() const;
  void removeOperator(ClientSocket* member);
  void addOperator(ClientSocket* member);

  void inviteMember(ClientSocket* member);
  void removeInvite(ClientSocket* member);

  bool isFull() const;
  bool isEmpty() const;

  bool isOperator(ClientSocket* member) const;
  bool isInvited(ClientSocket* member) const;
  bool isMember(ClientSocket* member) const;

  bool checkPassword(const std::string& key) const;

  ~Channel();
};