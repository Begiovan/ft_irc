#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <poll.h>

void makePoll(int serverSocket);