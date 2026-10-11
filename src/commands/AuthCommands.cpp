#include <cctype>

#include "core/Channel.hpp"
#include "core/Server.hpp"
#include "protocol/numericCodes.hpp"
#include "sockets/ClientSocket.hpp"

// PASS <password>
void Server::passHandle(ClientSocket* client, Params& vec) {
  if (vec.empty()) {
    *client << ERR_NEEDMOREPARAMS("PASS") << CRLF;
    return;
  }

  if (client->isRegistered())
    return (void)(*client << ERR_ALREADYREGISTRED << CRLF);

  if (vec[0] != password_) {
    client->markForDisconnect();
    return (void)(*client << ERR_PASSWDMISMATCH << CRLF);
  }

  client->setPasswordAccepted();
  if (client->isRegistered()) {
    *client << RPL_WELCOME << CRLF;
    *client << ERR_NOMOTD << CRLF;
  }
}

// NICK <nickname>
void Server::nickHandle(ClientSocket* client, Params& vec) {
  if (vec.empty()) return (void)(*client << ERR_NONICKNAMEGIVEN << CRLF);

  std::string& nickname = vec.back();
  if (nickname == client->getNickName()) return;

  if (nickname[0] == '#' || nickname[0] == ':' ||
      std::isdigit(static_cast<unsigned char>(nickname[0])) ||
      nickname.length() > NICKLEN ||
      nickname.find_first_not_of(CHARSET) != std::string::npos)
    return (void)(*client << ERR_ERRONEUSNICKNAME << CRLF);

  if (clients_.count(nickname))
    return (void)(*client << ERR_NICKNAMEINUSE << CRLF);

  if (!client->getNickName().empty()) {
    std::set<ClientSocket*> observe;
    std::set<Channel*>::iterator oIt = client->getJoinedChannels().begin();
    for (; oIt != client->getJoinedChannels().end(); oIt++) {
      observe.insert((*oIt)->getMembers().begin(), (*oIt)->getMembers().end());
    }
    observe.insert(client);
    std::set<ClientSocket*>::iterator it = observe.begin();
    for (; it != observe.end(); it++) {
      *(*it) << client << "NICK " << ": " + nickname << CRLF;
      this->updateSocket(*it, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
    clients_.erase(client->getNickName());
  }

  client->setNickName(nickname);
  clients_.insert(std::make_pair(nickname, client));
  if (client->isRegistered()) {
    *client << RPL_WELCOME << CRLF;
    *client << ERR_NOMOTD << CRLF;
  }
}

// USER <username> 0 * :<Real Name>
void Server::userHandle(ClientSocket* client, Params& vec) {
  if (vec.size() < 4)
    return (void)(*client << ERR_NEEDMOREPARAMS("USER") << CRLF);

  if (client->isRegistered())
    return (void)(*client << ERR_ALREADYREGISTRED << CRLF);

  if (vec[0].length() > USERLEN) vec[0].resize(USERLEN);
  client->setUserName(vec[0]);

  client->setRealName(vec.back());
  if (client->isRegistered()) {
    *client << RPL_WELCOME << CRLF;
    *client << ERR_NOMOTD << CRLF;
  }
}

// QUIT [<reason>]
void Server::quitHandle(ClientSocket* client, Params&) {
  client->markForDisconnect();
}

// PING
void Server::pingHandle(ClientSocket* client, Params& vec) {
  *client << SERVER_NAME << "PONG " << "myIRCserver";
  if (!vec.empty()) *client << " :" + vec[0];
  *client << CRLF;
}