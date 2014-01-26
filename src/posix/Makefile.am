include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS) -DPIONEER_DATA_DIR=\"@PIONEER_DATA_DIR@\"
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(top_srcdir)/src -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libposix.a
libposix_a_SOURCES = \
	FileSystemPosix.cpp \
	OSPosix.cpp
