/*
  Copyright 2014 James Hunt <james@jameshunt.us>

  This file is part of libvigor.

  libvigor is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libvigor is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with libvigor.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _SHITTY_DAEMON_H
#define _SHITTY_DAEMON_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <fcntl.h>
#include <assert.h>
#include <errno.h>

int daemonize(const char *pidfile, const char *user, const char *group);
int cleanenv(int n, const char **keep);
#endif
