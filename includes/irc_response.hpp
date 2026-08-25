#ifndef IRC_RESPONSE_HPP
#define IRC_RESPONSE_HPP

#include <string>

/* Error Responses */

#define ERR_NOTREGISTERED(source)                       std::string(":ircserv 451 ") + source + " :You have not registered"
#define ERR_ALREADYREGISTERED(source)                   std::string(":ircserv 462 ") + source + " :You may not register"
#define ERR_PASSWDMISMATCH(source)                      std::string(":ircserv 464 ") + source + " :Password is incorrect"
#define ERR_NONICKNAMEGIVEN(source)                     std::string(":ircserv 431 ") + source + " :Nickname not given"
#define ERR_NICKNAMEINUSE(source)                       std::string(":ircserv 433 ") + source + " " + source  + " :Nickname is already in use"
#define ERR_ERRONEUSNICKNAME(source, nickname)          std::string(":ircserv 432 ") + source + " " + nickname + " :Erroneous nickname"

#define ERR_UNKNOWNCOMMAND(source, command)             std::string(":ircserv 421 ") + source + " " + command + " :Unknown command"
#define ERR_NEEDMOREPARAMS(source, command)             std::string(":ircserv 461 ") + source + " " + command + " :Not enough parameters"

#define ERR_TOOMANYCHANNELS(source, channel)            std::string(":ircserv 405 ") + source + " " + channel + " :You have joined too many channels"
#define ERR_NOTONCHANNEL(source, channel)               std::string(":ircserv 442 ") + source + " " + channel + " :You're not on that channel"
#define ERR_NOSUCHCHANNEL(source, channel)              std::string(":ircserv 403 ") + source + " " + channel + " :No such channel"
#define ERR_BADCHANNELKEY(source, channel)              std::string(":ircserv 475 ") + source + " " + channel + " :Cannot join channel (+k)"
#define ERR_CHANNELISFULL(source, channel)              std::string(":ircserv 471 ") + source + " " + channel + " :Cannot join channel (+l)"
#define ERR_INVITEONLYCHAN(source, channel)             std::string(":ircserv 473 ") + source + " " + channel + " :Cannot join channel (+i)"
#define ERR_USERONCHANNEL(source, channel)              std::string(":ircserv 443 ") + source + " " + channel + " :is already on channel"
#define ERR_CANNOTSENDTOCHAN(source, channel)           std::string(":ircserv 404 ") + source + " " + channel + " :Cannot send to channel"
#define ERR_CHANOPRIVSNEEDED(source, channel)           std::string(":ircserv 482 ") + source + " " + channel + " :You're not channel operator"

#define ERR_NOSUCHNICK(source, nickname)                std::string(":ircserv 401 ") + source + " " + nickname + " :No such nick/channel"
#define ERR_USERNOTINCHANNEL(source, nickname, channel) std::string(":ircserv 441 ") + source + " " + nickname + " " + channel + " :They aren't on that channel"


/* Numeric Responses */

#define RPL_WELCOME(source)                             std::string(":ircserv 001 ") + source + " :Welcome " + source + " to the ft_irc network"
#define RPL_NAMREPLY(source, channel, users)            std::string(":ircserv 353 ") + source + " = " + channel + " :" + users
#define RPL_ENDOFNAMES(source, channel)                 std::string(":ircserv 366 ") + source + " " + channel + " :End of /NAMES list."
#define RPL_NOTOPIC(source, channel)                    std::string(":ircserv 331 ") + source + " " + channel + " :No topic set."
#define RPL_TOPIC(source, channel, topic)               std::string(":ircserv 332 ") + source + " " + channel + " :" + topic
#define RPL_INVITING(source, target, channel)           std::string(":ircserv 341 ") + source + " " + target + " " + channel

/* Command Responses */

#define RPL_JOIN(source, user, channel)                 std::string(":") + source + "!" + user + "@ircserv JOIN :" + channel
#define RPL_PART(source, user, channel)                 std::string(":") + source + "!" + user + "@ircserv PART :" + channel
#define RPL_PING(source, token)                         std::string(":") + source + " PONG :" + token
#define RPL_PRIVMSG(source, user, target, message)      std::string(":") + source + "!" + user + "@ircserv PRIVMSG " + target + " :" + message
#define RPL_NOTICE(source, user, target, message)       std::string(":") + source + "!" + user + "@ircserv NOTICE " + target + " :" + message
#define RPL_QUIT(source, user, message)                 std::string(":") + source + "!" + user + "@ircserv QUIT :Quit: " + message
#define RPL_KICK(source, user, channel, target, reason) std::string(":") + source + "!" + user + "@ircserv KICK " + channel + " " + target + " :" + reason
#define RPL_MODE(source, user, channel, modes, args)    std::string(":") + source + "!" + user + "@ircserv MODE " + channel + " " + modes + " " + args
#define RPL_INVITE(source, user, target, channel)		std::string(":") + source + "!" + user + "@ircserv INVITE " + target + " :" + channel

#endif