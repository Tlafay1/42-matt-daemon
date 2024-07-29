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
#include <sys/file.h>
#include <sys/wait.h>

#define PORT 4242
#define MAX_CLIENTS 3
#define LOCK_FILE "/var/log/matt_daemon.log"

#endif