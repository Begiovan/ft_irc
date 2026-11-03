#pragma once

#include <string>

#define SERVER_NAME "ircserv"

/* Error Responses */

#define ERR_NOTREGISTERED(source)                       std::string(":" SERVER_NAME " 451 ") + source + " :You have not registered"
#define ERR_ALREADYREGISTERED(source)                   std::string(":" SERVER_NAME " 462 ") + source + " :You may not reregister"
#define ERR_PASSWDMISMATCH(source)                      std::string(":" SERVER_NAME " 464 ") + source + " :Password is incorrect"
#define ERR_NONICKNAMEGIVEN(source)                     std::string(":" SERVER_NAME " 431 ") + source + " :Nickname not given"
#define ERR_NICKNAMEINUSE(source, nickname)             std::string(":" SERVER_NAME " 433 ") + source + " " + nickname  + " :Nickname is already in use"
#define ERR_ERRONEUSNICKNAME(source, nickname)          std::string(":" SERVER_NAME " 432 ") + source + " " + nickname + " :Erroneous nickname"

#define ERR_UNKNOWNCOMMAND(source, command)             std::string(":" SERVER_NAME " 421 ") + source + " " + command + " :Unknown command"
#define ERR_NEEDMOREPARAMS(source, command)             std::string(":" SERVER_NAME " 461 ") + source + " " + command + " :Not enough parameters"

#define ERR_TOOMANYCHANNELS(source, channel)            std::string(":" SERVER_NAME " 405 ") + source + " " + channel + " :You have joined too many channels"
#define ERR_NOTONCHANNEL(source, channel)               std::string(":" SERVER_NAME " 442 ") + source + " " + channel + " :You're not on that channel"
#define ERR_NOSUCHCHANNEL(source, channel)              std::string(":" SERVER_NAME " 403 ") + source + " " + channel + " :No such channel"
#define ERR_BADCHANNELKEY(source, channel)              std::string(":" SERVER_NAME " 475 ") + source + " " + channel + " :Cannot join channel (+k)"
#define ERR_CHANNELISFULL(source, channel)              std::string(":" SERVER_NAME " 471 ") + source + " " + channel + " :Cannot join channel (+l)"
#define ERR_INVITEONLYCHAN(source, channel)             std::string(":" SERVER_NAME " 473 ") + source + " " + channel + " :Cannot join channel (+i)"
#define ERR_USERONCHANNEL(source, target, channel)      std::string(":" SERVER_NAME " 443 ") + source + " " + target + " " + channel + " :is already on channel"
#define ERR_CANNOTSENDTOCHAN(source, channel)           std::string(":" SERVER_NAME " 404 ") + source + " " + channel + " :Cannot send to channel"
#define ERR_CHANOPRIVSNEEDED(source, channel)           std::string(":" SERVER_NAME " 482 ") + source + " " + channel + " :You're not channel operator"

#define ERR_NOSUCHNICK(source, nickname)                std::string(":" SERVER_NAME " 401 ") + source + " " + nickname + " :No such nick/channel"
#define ERR_USERNOTINCHANNEL(source, nickname, channel) std::string(":" SERVER_NAME " 441 ") + source + " " + nickname + " " + channel + " :They aren't on that channel"
#define ERR_UNKNOWNMODE(source, mode, channel)			std::string(":" SERVER_NAME " 472 ") + source + " " + mode + " :is unknown mode char to me for " + channel


/* Numeric Responses */

#define RPL_WELCOME(source)                             std::string(":" SERVER_NAME " 001 ") + source + " :Welcome " + source + " to the ft_irc network"
#define RPL_NAMREPLY(source, channel, users)            std::string(":" SERVER_NAME " 353 ") + source + " = " + channel + " :" + users
#define RPL_ENDOFNAMES(source, channel)                 std::string(":" SERVER_NAME " 366 ") + source + " " + channel + " :End of /NAMES list."
#define RPL_NOTOPIC(source, channel)                    std::string(":" SERVER_NAME " 331 ") + source + " " + channel + " :No topic set."
#define RPL_TOPIC(source, channel, topic)               std::string(":" SERVER_NAME " 332 ") + source + " " + channel + " :" + topic
#define RPL_INVITING(source, target, channel)           std::string(":" SERVER_NAME " 341 ") + source + " " + channel + " " + target

/* Command Responses */

#define RPL_NICK(oldnick, user, newnick)                std::string(":") + oldnick + "!" + user + "@" SERVER_NAME " NICK :" + newnick
#define RPL_JOIN(source, user, channel)                 std::string(":") + source + "!" + user + "@" SERVER_NAME " JOIN " + channel
#define RPL_PART(source, user, channel)                 std::string(":") + source + "!" + user + "@" SERVER_NAME " PART " + channel
#define RPL_TOPICCHANGE(source, user, channel, topic)   std::string(":") + source + "!" + user + "@" SERVER_NAME " TOPIC " + channel + " :" + topic
#define RPL_PING(source, token)                         std::string(":") + source + " PONG :" + token
#define RPL_PRIVMSG(source, user, target, message)      std::string(":") + source + "!" + user + "@" SERVER_NAME " PRIVMSG " + target + " :" + message
#define RPL_NOTICE(source, user, target, message)       std::string(":") + source + "!" + user + "@" SERVER_NAME " NOTICE " + target + " :" + message
#define RPL_QUIT(source, user, message)                 std::string(":") + source + "!" + user + "@" SERVER_NAME " QUIT :Quit: " + message
#define RPL_KICK(source, user, channel, target, reason) std::string(":") + source + "!" + user + "@" SERVER_NAME " KICK " + channel + " " + target + " :" + reason
#define RPL_MODE(source, user, channel, modes, args)    std::string(":") + source + "!" + user + "@" SERVER_NAME " MODE " + channel + " " + modes + " " + args
#define RPL_INVITE(source, user, target, channel)		std::string(":") + source + "!" + user + "@" SERVER_NAME " INVITE " + target + " :" + channel
