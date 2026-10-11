#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <ctime>
#include "interfaces/IServerContext.hpp"

#define CHARSET \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789[]{}|\\"
#define NICKLEN 31
#define USERLEN 18

#define CRLF "\r\n"

class ISocket;
class Channel;
class ClientSocket;

class Server : public IServerContext {
  int epollFd_;
  std::string password_;
  std::set<ISocket*> sockets_;
  std::map<std::string, ClientSocket*> clients_;

  
  typedef std::vector<std::string> Params ;
  typedef void (Server::*CommandFunc)(ClientSocket* , Params&);

  std::map<std::string, CommandFunc> commands_;

  std::map<std::string, Channel*> channels_;

 public:
  static bool G_SERVER_RUNNING;

  Server(const char* password);

  int getEpollFd(void);

  void removeSocket(ISocket* sock);
  void insertSocket(ISocket* sock);
  void updateSocket(ISocket* sock, unsigned int event, int op);
  void onMessageReceived(ISocket* sock, std::string& message);

  void passHandle(ClientSocket* client, Params& vec);
  void nickHandle(ClientSocket* client, Params& vec);
  void userHandle(ClientSocket* client, Params& vec);
  void quitHandle(ClientSocket* client, Params& vec);
  void pingHandle(ClientSocket* client, Params& vec);

  void botHandler(ClientSocket* client, std::string& message);
  void privmsgHandle(ClientSocket* client, Params& vec);

  void joinHandle(ClientSocket* client, Params& vec);
  void partHandle(ClientSocket* client, Params& vec);
  void kickHandle(ClientSocket* client, Params& vec);
  void topicHandle(ClientSocket* client, Params& vec);
  void modeHandle(ClientSocket* client, Params& vec);
  void inviteHandle(ClientSocket* client, Params& vec);

  void deleteChannel(const std::string& name);
  Channel* getChannel(const std::string& name);
  Channel* createChannel(const std::string& name, ClientSocket* creator);

  ~Server();
};