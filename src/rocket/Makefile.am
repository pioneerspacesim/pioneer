include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

noinst_LIBRARIES = librocket.a
noinst_HEADERS = \
	RocketGaugeElement.h \
	RocketGaugeType.h \
	RocketGaugeTypeBar.h \
	RocketManager.h \
	RocketStash.h \
	RocketStashListElement.h

librocket_a_SOURCES = \
	RocketGaugeElement.cpp \
	RocketGaugeType.cpp \
	RocketGaugeTypeBar.cpp \
	RocketManager.cpp \
	RocketStash.cpp \
	RocketStashListElement.cpp

INCLUDES = -I$(srcdir)/..
INCLUDES += -isystem @top_srcdir@/contrib -isystem @top_srcdir@/contrib/rocket/include
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
