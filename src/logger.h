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
#ifndef _SHITTY_LOG_H
#define _SHITTY_LOG_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <assert.h>
#include <errno.h>

#include <syslog.h>

void log_open (const char *ident, const char *facility);
void log_close(void);

int         log_level       (int level, const char *name);
const char* log_level_name  (int level);
int         log_level_number(const char *name);

void logger(int level, const char *fmt, ...);
#endif
