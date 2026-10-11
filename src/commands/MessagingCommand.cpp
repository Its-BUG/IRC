#include "core/Channel.hpp"
#include "core/Server.hpp"
#include "protocol/numericCodes.hpp"
#include "sockets/ClientSocket.hpp"

static bool not_alowed(std::string& message)
{
  return (message != ":help" && message != ":time" && message != ":upper");      
}
void Server::botHandler(ClientSocket* client, std::string& message)
{
    if(message == ":help"){
      *client << ":Bot PRIVMSG"
      << client->getNickName()
      << ALLOWED_CMDS 
      << CRLF ;
      this->updateSocket(client, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
    else if(message.find(":upper") == 0){
      std::string text = message.substr(7);
      for (size_t i = 0; i < text.length(); i++){
        text[i] = std::toupper(static_cast<unsigned char>(text[i]));
      }
      *client << ":Bot PRIVMSG "
          << client->getNickName()
          << " :"
          << text
          << CRLF;
        this->updateSocket(client, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
    else if (message == ":time"){
        time_t now = time(NULL);

        *client << ":Bot PRIVMSG "
                << client->getNickName()
                << " :"
                << ctime(&now)
                << CRLF;

        this->updateSocket(client, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
    else if(not_alowed(message))
    {
        *client << ":Bot PRIVMSG"
        << client->getNickName()
        << CMD_NOTFOUND 
        << CRLF ;
        this->updateSocket(client, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
  return;
}

// PRIVMSG <target> :<message text>
void Server::privmsgHandle(ClientSocket* client, Params& vec) {
  if (vec.empty() || vec[0][0] == ':')
    return (void)(*client << ERR_NORECIPIENT("PRIVMSG") << CRLF);

  if (vec.size() < 2 || vec[1][0] != ':')
    return (void)(*client << ERR_NOTEXTTOSEND << CRLF);

  std::string& target = vec.at(0);
  std::string& message = vec.back();

  if (target[0] == '#' || target[0] == '&')
  {
    std::string& chname = target;

    if (!channels_.count(chname))
      return (void)(*client << ERR_NOSUCHCHANNEL << CRLF);

    Channel* channel = channels_[chname];
    if (!channel->isMember(client))
      return (void)(*client << ERR_NOTONCHANNEL << CRLF);

    channels_[target]->broadcastMessage(this, client, "PRIVMSG", target,
                                        message);
  }
  else 
  {
    if(target == "Bot")
      return (void)(botHandler(client,message));
    
    if (!clients_.count(target))
      return (void)(*client << ERR_NOSUCHNICK << CRLF);

    ClientSocket* toClient = clients_[target];
    *toClient << client << " PRIVMSG " << target << " " << message << CRLF;
    this->updateSocket(toClient, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
  }
}
