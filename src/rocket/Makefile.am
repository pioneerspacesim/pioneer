include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

noinst_LIBRARIES = librocket.a
noinst_HEADERS = \
	RocketFaceElement.h \
	RocketShipSpinnerElement.h \
	RocketManager.h

librocket_a_SOURCES = \
	RocketFaceElement.cpp \
	RocketShipSpinnerElement.cpp \
	RocketManager.cpp

INCLUDES = -I$(srcdir)/..
INCLUDES += -isystem @top_srcdir@/contrib -isystem @top_srcdir@/contrib/rocket/include
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
