AC_INIT([shittyd], [0.0.1], [bugs@danmolik.com])
AC_PREREQ(2.68)
AC_CONFIG_SRCDIR([src/server.h])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC

AC_HAVE_LIBRARY(pthread,,  AC_MSG_ERROR(Missing pthread library))

AC_C_INLINE
AC_PROG_INSTALL

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
