/*
 * Copyright 2015 Dan Molik <dan@danmolik.com>
 *
 * This file is part of Shitty Little Server
 *
 * Shitty Little Server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Shitty Little Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 *along with Shitty Little Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SHITTY_SERVER_H
#define _SHITTY_SERVER_H

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <signal.h>
#include <sys/signalfd.h>

#include <sys/epoll.h>

#include <pthread.h>

#include "logger.h"
#include "../config.h"

#define MAX_EVENTS 8192
#define MAXMSG     1492

#define SERVER_NAME "shitty little server"

#define _ERROR(msg) \
	do { logger(LOG_ERR, "%s, %s", msg, strerror(errno)); exit(EXIT_FAILURE); } while (0)

#define PORT   3000


const char * time_s(void);

void s_content(int fd, char *msg);

int peer_helper(int fd, long tid);

#endif
