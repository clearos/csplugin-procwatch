#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([csplugin-procwatch], [1.0], [http://www.clearfoundation.com/docs/developer/bug_tracker/])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([csplugin-procwatch.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([procps], [openproc], [], [
        AC_MSG_ERROR([libprocps not found but is required.])])

# Checks for header files.
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([clearsync/csplugin.h], [], [AC_MSG_ERROR([clearsync header files not found.])])
AC_LANG_POP([C++])
AC_CHECK_HEADERS([proc/readproc.h], [], [AC_MSG_ERROR([procps header files not found.])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.

# Output files
AC_CONFIG_FILES([Makefile csplugin-procwatch.spec])
AC_OUTPUT
