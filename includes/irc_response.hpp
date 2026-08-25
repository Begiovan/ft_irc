#ifndef IRC_RESPONSE_HPP
#define IRC_RESPONSE_HPP

#include <string>

/* Error Responses */

#define ERR_NOTREGISTERED(source)                       std::string("451 ") + source + " :You have not registered"
#define ERR_ALREADYREGISTERED(source)                   std::string("462 ") + source + " :You may not register"
#define ERR_PASSWDMISMATCH(source)                      std::string("464 ") + source + " :Password is incorrect"
#define ERR_NONICKNAMEGIVEN(source)                     std::string("431 ") + source + " :Nickname not given"
#define ERR_NICKNAMEINUSE(source)                       std::string("433 ") + source + " " + source  + " :Nickname is already in use"
#define ERR_ERRONEUSNICKNAME(source, nickname)          std::string("432 ") + source + " " + nickname + " :Erroneous nickname"

#define ERR_UNKNOWNCOMMAND(source, command)             std::string("421 ") + source + " " + command + " :Unknown command"
#define ERR_NEEDMOREPARAMS(source, command)             std::string("461 ") + source + " " + command + " :Not enough parameters"

#define ERR_TOOMANYCHANNELS(source, channel)            std::string("405 ") + source + " " + channel + " :You have joined too many channels"
#define ERR_NOTONCHANNEL(source, channel)               std::string("442 ") + source + " " + channel + " :You're not on that channel"
#define ERR_NOSUCHCHANNEL(source, channel)              std::string("403 ") + source + " " + channel + " :No such channel"
#define ERR_BADCHANNELKEY(source, channel)              std::string("475 ") + source + " " + channel + " :Cannot join channel (+k)"
#define ERR_CHANNELISFULL(source, channel)              std::string("471 ") + source + " " + channel + " :Cannot join channel (+l)"
#define ERR_INVITEONLYCHAN(source, channel)             std::string("473 ") + source + " " + channel + " :Cannot join channel (+i)"
#define ERR_USERONCHANNEL(source, channel)              std::string("443 ") + source + " " + channel + " :is already on channel"
#define ERR_CANNOTSENDTOCHAN(source, channel)           std::string("404 ") + source + " " + channel + " :Cannot send to channel"
#define ERR_CHANOPRIVSNEEDED(source, channel)           std::string("482 ") + source + " " + channel + " :You're not channel operator"

#define ERR_NOSUCHNICK(source, nickname)                std::string("401 ") + source + " " + nickname + " :No such nick/channel"
#define ERR_USERNOTINCHANNEL(source, nickname, channel) std::string("441 ") + source + " " + nickname + " " + channel + " :They aren't on that channel"


/* Numeric Responses */

#define RPL_WELCOME(source)                             std::string("001 ") + source + " :Welcome " + source + " to the ft_irc network"
#define RPL_NAMREPLY(source, channel, users)            std::string("353 ") + source + " = " + channel + " :" + users
#define RPL_ENDOFNAMES(source, channel)                 std::string("366 ") + source + " " + channel + " :End of /NAMES list."
#define RPL_NOTOPIC(source, channel)                    std::string("331 ") + source + " " + channel + " :No topic set."
#define RPL_TOPIC(source, channel, topic)               std::string("332 ") + source + " " + channel + " :" + topic
#define RPL_INVITING(source, target, channel)           std::string("341 ") + source + " " + target + " " + channel

/* Command Responses */

#define RPL_JOIN(source, channel)                       std::string(":") + source + " JOIN :" + channel
#define RPL_PART(source, channel)                       std::string(":") + source + " PART :" + channel
#define RPL_PING(source, token)                         std::string(":") + source + " PONG :" + token
#define RPL_PRIVMSG(source, target, message)            std::string(":") + source + " PRIVMSG " + target + " :" + message
#define RPL_NOTICE(source, target, message)             std::string(":") + source + " NOTICE " + target + " :" + message
#define RPL_QUIT(source, message)                       std::string(":") + source + " QUIT :Quit: " + message
#define RPL_KICK(source, channel, target, reason)       std::string(":") + source + " KICK " + channel + " " + target + " :" + reason
#define RPL_MODE(source, channel, modes, args)          std::string(":") + source + " MODE " + channel + " " + modes + " " + args
#define RPL_INVITE(source, target, channel)				std::string(":") + source + " INVITE " + target + " :" + channel

#endif