#include <cstdlib>

#include "core/Channel.hpp"
#include "core/Server.hpp"
#include "protocol/numericCodes.hpp"
#include "sockets/ClientSocket.hpp"

// JOIN <channel> [<key>]
void Server::joinHandle(ClientSocket* client, Params& vec) {
  if (vec.empty()) return (void)(*client << ERR_NEEDMOREPARAMS("JOIN") << CRLF);

  const std::string& target = client->getNickName();
  const std::string& chname = vec[0];
  if (chname[0] != '#') return (void)(*client << ERR_BADCHANMASK << CRLF);

  Channel* channel;
  if (!channels_.count(chname)) {
    channel = createChannel(chname, client);
    if (vec.size() >= 2) {
      channel->setPassword(vec[1]);
      channel->setMode(CHANNEL_LOCKED);
    }
  } else
    channel = channels_[chname];

  if ((channel->getModes() & INVITE_ONLY) && !channel->isInvited(client))
    return (void)(*client << ERR_INVITEONLYCHAN << CRLF);

  if (channel->getModes() & USERS_LIMIT && channel->isFull())
    return (void)(*client << ERR_CHANNELISFULL << CRLF);

  if (channel->getModes() & CHANNEL_LOCKED &&
      (vec.size() < 2 || !channel->checkPassword(vec[1])))
    return (void)(*client << ERR_BADCHANNELKEY << CRLF);

  channel->addMember(client);
  client->joinChannel(channel);
  channel->removeInvite(client);
  channel->broadcastMessage(this, client, "JOIN", "", chname);
  *client << client << " JOIN :" << chname << CRLF;

  const std::string& topic = channel->getTopic();
  if (!topic.empty()) *client << RPL_TOPIC << CRLF;

  std::string memberList;
  std::set<ClientSocket*>::iterator it = channel->getMembers().begin();
  for (; it != channel->getMembers().end(); it++) {
    memberList += (it != channel->getMembers().begin() ? " " : "");
    memberList += (channel->isOperator((*it)) ? "@" : "");
    memberList += (*it)->getNickName();
  }
  *client << RPL_NAMREPLY(memberList) << CRLF;

  *client << RPL_ENDOFNAMES << CRLF;
}

// PART <channel> [<reason>]
void Server::partHandle(ClientSocket* client, Params& vec) {
  if (vec.empty()) return (void)(*client << ERR_NEEDMOREPARAMS("PART") << CRLF);

  const std::string& chname = vec[0];
  if (!channels_.count(chname))
    return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

  Channel* channel = channels_[chname];
  if (!channel->isMember(client))
    return (void)(*client << ERR_NOTONCHANNEL << CRLF);

  std::string reason = vec.size() >= 2 ? vec[1] : ":Left the channel";
  channel->broadcastMessage(this, client, "PART", chname, reason);
  *client << client << " PART " << chname << " " << reason << CRLF;

  channel->removeMember(client);
  if (channel->getOperators().empty() && !channel->getMembers().empty()) {
    ClientSocket* newOp = *(channel->getMembers().begin());
    channel->addOperator(newOp);
    *newOp << " MODE " << channel->getName() << " +o " << newOp->getNickName()
           << CRLF;
  }

  client->leaveChannel(channel);

  if (channel->isEmpty()) this->deleteChannel(chname);
}

// KICK <channel> <nickname> [<reason>]
void Server::kickHandle(ClientSocket* client, Params& vec) {
  if (vec.size() < 2)
    return (void)(*client << ERR_NEEDMOREPARAMS("KICK") << CRLF);

  const std::string& chname = vec[0];
  if (!channels_.count(chname))
    return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

  Channel* channel = channels_[chname];
  if (!channel->isMember(client))
    return (void)(*client << ERR_NOTONCHANNEL << CRLF);

  if (!channel->isOperator(client))
    return (void)(*client << ERR_CHANOPRIVSNEEDED << CRLF);

  const std::string& target = vec[1];
  if (!clients_.count(target) || !channel->isMember(clients_[target]))
    return (void)(*client << ERR_USERNOTINCHANNEL << CRLF);

  ClientSocket* toClient = clients_[target];
  std::string reason = vec.size() > 2 ? vec[2] : ":Kicked by operator";
  channel->broadcastMessage(this, client, "KICK", chname + " " + target,
                            reason);
  *client << client << " KICK " << chname + " " + target << " " << reason
          << CRLF;

  channel->removeMember(toClient);
  toClient->leaveChannel(channel);

  if (channel->isEmpty()) this->deleteChannel(chname);
}

// TOPIC <channel> [<topic>]
void Server::topicHandle(ClientSocket* client, Params& vec) {
  if (vec.empty())
    return (void)(*client << ERR_NEEDMOREPARAMS("TOPIC") << CRLF);

  const std::string& chname = vec[0];
  if (!channels_.count(chname))
    return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

  Channel* channel = channels_[chname];
  if (!channel->isMember(client))
    return (void)(*client << ERR_NOTONCHANNEL << CRLF);

  const std::string& target = client->getNickName();
  if (vec.size() == 1) {
    const std::string& topic = channel->getTopic();
    if (topic.empty())
      *client << RPL_NOTOPIC << CRLF;
    else
      *client << RPL_TOPIC << CRLF;
    return;
  }

  if ((channel->getModes() & TOPIC_RESTRICTED) && !channel->isOperator(client))
    return (void)(*client << ERR_CHANOPRIVSNEEDED << CRLF);

  channel->setTopic(vec[1]);
  channel->broadcastMessage(this, client, "TOPIC", chname, vec[1]);
  *client << client << " TOPIC " << chname << " " << vec[1] << CRLF;
}

static void appendMode(std::string& modes, char& lastSign, char sign,
                       char mode) {
  if (lastSign != sign) {
    modes += sign;
    lastSign = sign;
  }
  modes += mode;
}

// MODE <channel> [<mode_char>] [<mode_arguments...>]
void Server::modeHandle(ClientSocket* client, Params& vec) {
  if (vec.empty()) return (void)(*client << ERR_NEEDMOREPARAMS("MODE") << CRLF);

  const std::string& chname = vec[0];
  if (!channels_.count(chname))
    return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

  Channel* channel = channels_[chname];
  if (vec.size() == 1)
    return (void)(*client << RPL_CHANNELMODEIS(channel->getModeString())
                          << CRLF);

  if (!channel->isOperator(client))
    return (void)(*client << ERR_CHANOPRIVSNEEDED << CRLF);

  const std::string& modeString = vec[1];
  bool adding = true;
  size_t argIndex = 2;
  std::string appliedModes = "";
  std::string appliedArgs = "";
  char lastSign = '\0';

  for (size_t i = 0; i < modeString.length(); ++i) {
    char c = modeString[i];

    if (c == '+' || c == '-') {
      adding = (c == '+');
      continue;
    }

    switch (c) {
      case 'i':
        if (adding != (bool)(channel->getModes() & INVITE_ONLY)) {
          adding ? channel->setMode(INVITE_ONLY)
                 : channel->unsetMode(INVITE_ONLY);
          appendMode(appliedModes, lastSign, adding ? '+' : '-', c);
        }
        break;

      case 't':
        if (adding != (bool)(channel->getModes() & TOPIC_RESTRICTED)) {
          adding ? channel->setMode(TOPIC_RESTRICTED)
                 : channel->unsetMode(TOPIC_RESTRICTED);
          appendMode(appliedModes, lastSign, adding ? '+' : '-', c);
        }
        break;

      case 'k':
        if (adding && argIndex < vec.size()) {
          channel->setPassword(vec[argIndex]);
          channel->setMode(CHANNEL_LOCKED);
          appendMode(appliedModes, lastSign, '+', c);
          appliedArgs += " " + vec[argIndex++];
        } else if (!adding && (channel->getModes() & CHANNEL_LOCKED)) {
          channel->unsetMode(CHANNEL_LOCKED);
          channel->setPassword("");
          appendMode(appliedModes, lastSign, '-', c);
        }
        break;

      case 'l':
        if (adding && argIndex < vec.size()) {
          int limit = std::atoi(vec[argIndex].c_str());
          if (limit > 0) {
            channel->setUserLimit(limit);
            channel->setMode(USERS_LIMIT);
            appendMode(appliedModes, lastSign, '+', c);
            appliedArgs += " " + vec[argIndex];
          }
          argIndex++;
        } else if (!adding && (channel->getModes() & USERS_LIMIT)) {
          channel->unsetMode(USERS_LIMIT);
          channel->setUserLimit(0);
          appendMode(appliedModes, lastSign, '-', c);
        }
        break;

      case 'o':
        if (argIndex < vec.size()) {
          const std::string& target = vec[argIndex++];
          if (!clients_.count(target)) {
            *client << ERR_NOSUCHNICK << CRLF;
            continue;
          }
          ClientSocket* targetClient = clients_[target];
          if (!channel->isMember(targetClient)) {
            *client << ERR_USERNOTINCHANNEL << CRLF;
            continue;
          }
          adding ? channel->addOperator(targetClient)
                 : channel->removeOperator(targetClient);
          appendMode(appliedModes, lastSign, adding ? '+' : '-', c);
          appliedArgs += " " + target;
        }
        break;

      default:
        *client << ERR_UNKNOWNMODE(c) << CRLF;
        break;
    }
  }

  if (!appliedModes.empty()) {
    std::string fullModeMessage = appliedModes + appliedArgs;
    channel->broadcastMessage(this, client, "MODE", chname, fullModeMessage);
    *client << client << " MODE " << chname << " " << fullModeMessage << CRLF;
  }
}

// INVITE <nickname> <channel>
void Server::inviteHandle(ClientSocket* client, Params& vec) {
  if (vec.size() < 2)
    return (void)(*client << ERR_NEEDMOREPARAMS("INVITE") << CRLF);

  const std::string& chname = vec[1];
  if (!channels_.count(chname))
    return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

  Channel* channel = channels_[chname];
  if (!channel->isMember(client))
    return (void)(*client << ERR_NOTONCHANNEL << CRLF);

  if (channel->getModes() & INVITE_ONLY && !channel->isOperator(client))
    return (void)(*client << ERR_CHANOPRIVSNEEDED << CRLF);

  const std::string& target = vec[0];
  if (!clients_.count(target)) return (void)(*client << ERR_NOSUCHNICK << CRLF);

  ClientSocket* toClient = clients_[target];
  if (channel->isMember(toClient))
    return (void)(*client << ERR_USERONCHANNEL << CRLF);

  channel->inviteMember(toClient);
  *client << RPL_INVITING << CRLF;

  *toClient << client << " INVITE " << target << " " << chname << CRLF;
  this->updateSocket(toClient, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
}
