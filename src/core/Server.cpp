#include "core/Server.hpp"

#include <unistd.h>
#include <iostream>
#include <algorithm>

#include "core/Channel.hpp"
#include "protocol/numericCodes.hpp"
#include "sockets/ClientSocket.hpp"

static bool isRegistrationCommand(const std::string& command) {
  return command == "PASS" || command == "NICK" || command == "USER" ||
         command == "QUIT" || command == "PING";
}

static std::vector<std::string> parseParams(const std::string& rawMsg,
                                            size_t sPos) {
  std::vector<std::string> params;

  while (sPos != std::string::npos) {
    sPos = rawMsg.find_first_not_of(' ', sPos);
    if (sPos == std::string::npos) break;

    std::string param = rawMsg.substr(sPos, rawMsg.find(' ', sPos) - sPos);
    if (param.empty()) break;

    if (param[0] == ':') {
      params.push_back(rawMsg.substr(sPos));
      break;
    }

    sPos = rawMsg.find_first_not_of(' ', sPos + param.length());
    params.push_back(param);
  }

  return params;
}


Server::Server(const char* password) : password_(password) {
  commands_.insert(std::make_pair("PASS", &Server::passHandle));
  commands_.insert(std::make_pair("NICK", &Server::nickHandle));
  commands_.insert(std::make_pair("USER", &Server::userHandle));
  commands_.insert(std::make_pair("QUIT", &Server::quitHandle));
  commands_.insert(std::make_pair("PING", &Server::pingHandle));

  commands_.insert(std::make_pair("PRIVMSG", &Server::privmsgHandle));

  commands_.insert(std::make_pair("JOIN", &Server::joinHandle));
  commands_.insert(std::make_pair("PART", &Server::partHandle));
  commands_.insert(std::make_pair("KICK", &Server::kickHandle));
  commands_.insert(std::make_pair("TOPIC", &Server::topicHandle));
  commands_.insert(std::make_pair("MODE", &Server::modeHandle));
  commands_.insert(std::make_pair("INVITE", &Server::inviteHandle));

  this->epollFd_ = epoll_create1(0);
  if (this->epollFd_ < 0) throw std::runtime_error("Cannot create an Epoll");
}

int Server::getEpollFd() { return epollFd_; }

void Server::updateSocket(ISocket* sock, unsigned int event, int op) {
  epoll_event ev = {event, {.ptr = sock}};
  epoll_ctl(epollFd_, op, sock->getFd(), &ev);
}

void Server::insertSocket(ISocket* newSock) { sockets_.insert(newSock); }

void Server::removeSocket(ISocket* sock) {
  ClientSocket* client = static_cast<ClientSocket*>(sock);
  if (client) {
    std::set<Channel*>::const_iterator it = client->getJoinedChannels().begin();
    for (; it != client->getJoinedChannels().end(); it++) {
      Channel* channel = *it;
      channel->removeMember(client);
      if (channel->getOperators().empty() && !channel->getMembers().empty()) {
        ClientSocket* newOp = *(channel->getMembers().begin());
        channel->addOperator(newOp);
        *newOp << SERVER_NAME << "MODE" << channel->getName() << "+o"
               << newOp->getNickName() << CRLF;
      }
      channel->broadcastMessage(this, client, "QUIT", "", ":Client exited");
      if (channel->isEmpty()) this->deleteChannel(channel->getName());
    }
  }

  epoll_ctl(epollFd_, EPOLL_CTL_DEL, sock->getFd(), NULL);
  sockets_.erase(sock);
  clients_.erase(static_cast<ClientSocket*>(sock)->getNickName());

  close(sock->getFd());
  delete sock;
}

void Server::onMessageReceived(ISocket* sock, std::string& message) {
  ClientSocket* client = static_cast<ClientSocket*>(sock);

  size_t ePos;
  while ((ePos = message.find("\r\n")) != std::string::npos) {
    std::string rawMsg = message.substr(0, ePos);
    message.erase(0, ePos + 2);

    if (rawMsg.empty()) continue;

    size_t sPos = rawMsg.find(' ');
    std::string command ;
    if(sPos == std::string::npos)
      command = rawMsg;
    else
      command = rawMsg.substr(0, sPos);

    if (commands_.find(command) == commands_.end()) {
      *client << ERR_UNKNOWNCOMMAND << CRLF;
      continue;
    }

    if (!(client->isRegistered()) &&
        !isRegistrationCommand(command)) {
      *client << ERR_NOTREGISTERED << CRLF;
      continue;
    }
    std::vector<std::string> params = parseParams(rawMsg, sPos);

    (this->*Server::commands_[command])(client, params);
  }
}

Channel* Server::getChannel(const std::string& name) {
  if (!channels_.count(name)) return NULL;
  return channels_[name];
}

Channel* Server::createChannel(const std::string& name, ClientSocket* creator) {
  Channel* channel = new Channel(name);
  channels_.insert(std::make_pair(name, channel));
  channel->addOperator(creator);
  return channel;
}

void Server::deleteChannel(const std::string& name) {
  Channel* channel = channels_[name];
  channels_.erase(name);
  delete channel;
}

Server::~Server() {
  std::set<ISocket*>::iterator it = sockets_.begin();
  for (; it != sockets_.end(); it++) {
    delete *it;
  }

  for (std::map<std::string, Channel*>::iterator it = channels_.begin();
       it != channels_.end(); it++) {
    delete it->second;
  }
}
