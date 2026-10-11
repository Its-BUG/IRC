#define RPL_WELCOME 001 << " :Welcome to the IRC Network"
#define ERR_NOMOTD 422 << " :MOTD File is missing"

#define CMD_NOTFOUND " command not found :try help!"
#define ALLOWED_CMDS " :allowed cammands: help | time | upper" 
#define ERR_UNKNOWNCOMMAND 421 << " " << command << " :Unknown command"
#define ERR_NOSUCHNICK 401 << " " << target << " :No such nick/channel"
#define ERR_NONICKNAMEGIVEN 431 << " :No nickname given"
#define ERR_ERRONEUSNICKNAME 432 << " :Erroneus nickname"
#define ERR_NICKNAMEINUSE 433 << " :Nickname is already in use"
#define ERR_NOTREGISTERED 451 << " :You have not registered"
#define ERR_PASSWDMISMATCH 464 << " :Password incorrect"
#define ERR_NORECIPIENT(command) 411 << " :No recipient given" << command
#define ERR_NOTEXTTOSEND 412 << " :No text to send"
#define ERR_BADCHANNELKEY 475 << " " << chname << " :Cannot join channel (+k)"
#define ERR_BADCHANMASK 476 << " " << chname << " :Bad Channel Mask"
#define RPL_TOPIC 332 << " " << target << " " << chname << " " << topic
#define RPL_NAMREPLY(memlist) 353 << " = " << chname << " :" + memlist
#define RPL_ENDOFNAMES 366 << " " << chname << " :End of /NAMES list."
#define ERR_NOSUCHCHANNEL 403 << " " << chname << " :No such channel"
#define ERR_NOTONCHANNEL 442 << " " << chname << " :You're not on that channel"
#define RPL_INVITING 341 << " " << target << " " << chname
#define ERR_INVITEONLYCHAN 473 << " " << chname << " :Cannot join channel (+i)"
#define RPL_NOTOPIC 331 << " " << chname << " :No topic is set"
#define RPL_CHANNELMODEIS(modes) 324 << " " << chname << " " << modes
#define ERR_CHANNELISFULL 471 << " " << chname << " :Cannot join channel (+l)"
#define ERR_NEEDMOREPARAMS(command) \
  461 << " " << command << " :Not enough parameters"
#define ERR_ALREADYREGISTRED \
  462 << " " << " :Unauthorized command (already registered)"
#define ERR_UNKNOWNMODE(mode) \
  472 << " " << mode << " :is unknown mode char to me"
#define ERR_CHANOPRIVSNEEDED \
  482 << " " << chname << " :You're not channel operator"
#define ERR_USERNOTINCHANNEL \
  441 << " " << target << " " << chname << " :Aren't in channel"
#define ERR_USERONCHANNEL \
  443 << " " << target << " " << chname << " :is already on channel"
