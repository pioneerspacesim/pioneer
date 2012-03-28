include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = liblmr.a
noinst_HEADERS = \
	GeomBuffer.h \
	LmrModel.h \
	ShipThruster.h \
	Utils.h

liblmr_a_SOURCES = \
	Compiler.cpp \
	GeomBuffer.cpp \
	LmrModel.cpp \
	ShipThruster.cpp

INCLUDES += -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
