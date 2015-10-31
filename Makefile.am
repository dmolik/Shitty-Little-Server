AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I build

AM_CFLAGS = -Wall -g $(DEPS_CFLAGS)

core_src :=
core_src += src/logger.c
core_src += src/server.c

shittyd_SOURCES = $(core_src)
sbin_PROGRAMS   = shittyd

dist_man_MANS  =
dist_man_MANS += man/shittyd.8
dist_man_MANS += man/shittyd.conf.5

man/%.1: man/%.1.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.5: man/%.5.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.7: man/%.7.pod
	./man/build $< @PACKAGE_VERSION@ > $@
man/%.8: man/%.8.pod
	./man/build $< @PACKAGE_VERSION@ > $@

clean-local:
	git clean -xfd
