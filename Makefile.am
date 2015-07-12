AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh csplugin-procwatch.conf csplugin-procwatch.h procwatch-conf.h procwatch-action-group.h

AM_CFLAGS = ${CFLAGS}
AM_CXXFLAGS = ${CXXFLAGS}

lib_LTLIBRARIES = libcsplugin-procwatch.la

libcsplugin_procwatch_la_SOURCES = csplugin-procwatch.cpp procwatch-conf.cpp procwatch-action-group.cpp
libcsplugin_procwatch_la_CXXFLAGS = ${AM_CXXFLAGS}
