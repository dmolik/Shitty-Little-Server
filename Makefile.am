AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I build

AM_CFLAGS = -Wall $(DEPS_CFLAGS)

core_src :=
core_src += src/httpd.c

httpd_SOURCES = $(core_src)
bin_PROGRAMS = httpd
