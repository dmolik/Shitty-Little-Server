#
# Copyright 2015 Dan Molik <dan@danmolik.com>
#
# This file is part of Shitty Little Server
#
# Shitty Little Server is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Shitty Little Server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Shitty Little Server.  If not, see <http://www.gnu.org/licenses/>.
#


AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I build

AM_CFLAGS = -Wall -g $(DEPS_CFLAGS)

core_headers :=
core_headers += src/logger.h
core_headers += src/server.h

core_src :=
core_src += src/logger.c
core_src += src/server.c

shittyd_SOURCES = $(core_headers) $(core_src)
sbin_PROGRAMS   = shittyd

dist_man_MANS  =
dist_man_MANS += man/shittyd.8
dist_man_MANS += man/shittyd.conf.5

EXTRA_DIST = Changelog README.md

man/%.1: man/%.1.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.5: man/%.5.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.7: man/%.7.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.8: man/%.8.pod
	./man/build $< @PACKAGE_VERSION@ > $@
