#include "sockets/ClientSocket.hpp"

#include <cerrno>

ClientSocket::ClientSocket(int fd, const char* hostname)
    : passwordAccepted_(false), nicknameSet_(false), usernameSet_(false), registered_(false), hostname_(hostname), pendingDisconnect_(false)
    {
  this->disconnect_ = false;
  this->fd_ = fd;
}

int ClientSocket::getStatus(void) { return status_; }
std::string ClientSocket::getNickName(void) const { return nickname_; }
std::string ClientSocket::getUserName(void) const { return username_; }
std::string ClientSocket::getRealName(void) const { return realname_; }
std::string ClientSocket::getHostName(void) const { return hostname_; }

void ClientSocket::setNickName(std::string& nickname) { nickname_ = nickname; nicknameSet_ = true;  updateRegistration();}
void ClientSocket::setUserName(std::string& username) { username_ = username; usernameSet_ = true;   updateRegistration();}
void ClientSocket::setRealName(std::string& realname) { realname_ = realname; }
void ClientSocket::setPasswordAccepted()
{
    passwordAccepted_ = true;
    updateRegistration();
}

void ClientSocket::updateRegistration()
{
    if (passwordAccepted_
        && nicknameSet_
        && usernameSet_)
    {
        registered_ = true;
    }
}

bool ClientSocket::isRegistered() const
{
    return registered_;
}

void ClientSocket::processEvent(unsigned int& revents,
                                IServerContext& context) {
  if (revents & (EPOLLHUP | EPOLLERR)) {
    this->disconnect_ = true;
    return;
  }

  if (revents & EPOLLOUT) {
    if (this->send_() < 0) return;

    if (this->outBuffer_.str().empty() && this->pendingDisconnect_)
      this->disconnect_ = true;

    if (this->outBuffer_.str().empty())
      context.updateSocket(this, EPOLLIN, EPOLL_CTL_MOD);
  }

  if (revents & EPOLLIN) {
    if (this->recv_() < 0) return;
    context.onMessageReceived(this, inBuffer_);
    if (!this->outBuffer_.str().empty())
      context.updateSocket(this, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
  }
}

int ClientSocket::send_(void) {
  std::string line = outBuffer_.str();
  ssize_t sendBytes = send(fd_, line.c_str(), line.length(), MSG_NOSIGNAL);
  if (sendBytes < 0) return this->disconnect_ = true, -1;
  outBuffer_.str(line.substr(sendBytes));
  outBuffer_.seekp(0, std::ios::end);
  return 0;
}

int ClientSocket::recv_(void) {
  char buffer[512];
  int readBytes = recv(fd_, buffer, sizeof(buffer), 0);
  if (readBytes == 0) return this->disconnect_ = true, -1;
  if (readBytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
    return this->disconnect_ = true, -1;
  }
  inBuffer_.append(buffer, readBytes);
  return 0;
}

ClientSocket& ClientSocket::operator<<(int numericCode)
{
  outBuffer_ << SERVER_NAME ;
  outBuffer_ << std::setw(3) << std::setfill('0') << numericCode ; 
  outBuffer_ << " ";
  if(this->nickname_.empty()) outBuffer_ << "*";
  else outBuffer_ << nickname_ ;
  return *this ;
}

ClientSocket& ClientSocket::operator<<(const std::string& text) {
  outBuffer_ << text;
  return *this;
}

ClientSocket& ClientSocket::operator<<(ClientSocket* client) {
  outBuffer_ << ":" << client->getNickName() << "!" << client->getUserName()
             << "@" << client->getHostName();
  return *this;
}

void ClientSocket::joinChannel(Channel* channel) { channels_.insert(channel); }
void ClientSocket::leaveChannel(Channel* channel) { channels_.erase(channel); }

const std::set<Channel*>& ClientSocket::getJoinedChannels() const {
  return channels_;
}

void ClientSocket::markForDisconnect() {
  pendingDisconnect_ = true; }

ClientSocket::~ClientSocket() {}
