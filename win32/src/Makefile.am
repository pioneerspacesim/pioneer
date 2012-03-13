include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libwin32.a
libwin32_a_SOURCES = \
	FileSystemWin32.cpp
