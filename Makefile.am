AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I build

AM_CFLAGS = -Wall -g $(DEPS_CFLAGS)

core_src :=
core_src += src/http_parser.c
core_src += src/logger.c
core_src += src/server.c

shittyd_SOURCES = $(core_src)
sbin_PROGRAMS   = shittyd

clean-local:
	git clean -xfd
