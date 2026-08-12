#pragma once

#include "ft_irc.hpp"

struct Command
{
    std::string command;
    std::vector<std::string> params;
};

Command parseCommand(const std::string &line);
