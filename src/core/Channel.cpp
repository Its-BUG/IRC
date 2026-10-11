#include "core/Channel.hpp"

#include <sys/epoll.h>

#include <sstream>

#include "interfaces/IServerContext.hpp"
#include "sockets/ClientSocket.hpp"

Channel::Channel(std::string name) : name_(name), modes_(0), usersLimit_(0) {}

void Channel::broadcastMessage(IServerContext* context, ClientSocket* sender,
                               const std::string& command,
                               const std::string& target,
                               const std::string& message) {
  std::set<ClientSocket*>::iterator it = members_.begin();
  for (; it != members_.end(); it++) {
    if ((*it) == sender) continue;
    *(*it) << sender << " " << command;
    if (!target.empty()) *(*it) << " " << target;
    *(*it) << " " << message << CRLF;
    context->updateSocket((*it), EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
  }
}

void Channel::setUserLimit(size_t limit) { usersLimit_ = limit; }
void Channel::setTopic(const std::string& topic) { topic_ = topic; }
void Channel::setPassword(const std::string& password) { password_ = password; }

const std::string& Channel::getName() const { return name_; }
const std::string& Channel::getTopic() const { return topic_; }

size_t Channel::getModes() const { return modes_; }
void Channel::setMode(size_t mode) { modes_ |= mode; }
void Channel::unsetMode(size_t mode) { modes_ &= ~mode; }
std::string Channel::getModeString() const {
  std::string modeChars = "+";
  std::string modeArgs = "";

  if (modes_ & INVITE_ONLY) modeChars += "i";
  if (modes_ & TOPIC_RESTRICTED) modeChars += "t";

  if (modes_ & CHANNEL_LOCKED) {
    modeChars += "k";
    modeArgs += " *";
  }

  if (modes_ & USERS_LIMIT) {
    modeChars += "l";
    std::stringstream ss;
    ss << this->usersLimit_;
    modeArgs += " " + ss.str();
  }
  if (modeChars == "+") return "";

  return modeChars + modeArgs;
}

const std::set<ClientSocket*>& Channel::getMembers() const { return members_; }
void Channel::addMember(ClientSocket* member) { members_.insert(member); }
void Channel::removeMember(ClientSocket* member) {
  members_.erase(member);
  invitees_.erase(member);
  operators_.erase(member);
}

void Channel::removeOperator(ClientSocket* member) { operators_.erase(member); }
void Channel::addOperator(ClientSocket* member) { operators_.insert(member); }
const std::set<ClientSocket*>& Channel::getOperators() const {
  return operators_;
}
void Channel::inviteMember(ClientSocket* member) { invitees_.insert(member); }
void Channel::removeInvite(ClientSocket* member) { invitees_.erase(member); }

bool Channel::isFull() const { return members_.size() >= usersLimit_; }
bool Channel::isEmpty() const { return members_.empty(); }

bool Channel::isOperator(ClientSocket* member) const {
  return operators_.count(member);
}
bool Channel::isInvited(ClientSocket* member) const {
  return invitees_.count(member);
}
bool Channel::isMember(ClientSocket* member) const {
  return members_.count(member);
}

bool Channel::checkPassword(const std::string& key) const {
  return password_ == key;
}

Channel::~Channel() {}
