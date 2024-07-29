#ifndef MATT_DAEMON_HPP
#define MATT_DAEMON_HPP

#include <unistd.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cerrno>
#include <sys/stat.h>
#include <stdio.h>
#include <cstdio>

#define PORT 4240
#define MAX_CLIENTS 3
#define DEBUG 1

#endif