#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include "parser.hpp"
#include "irc_response.hpp"

bool isPositiveInteger(const std::string &value);
bool isValidNickname(const std::string &nick);
