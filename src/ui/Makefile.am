include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

noinst_LIBRARIES = libui.a
noinst_HEADERS = \
	UIGaugeElement.h \
	UIGaugeType.h \
	UIGaugeTypeBar.h \
	UIManager.h \
	UIStash.h \
	UIStashListElement.h

libui_a_SOURCES = \
	UIGaugeElement.cpp \
	UIGaugeType.cpp \
	UIGaugeTypeBar.cpp \
	UIManager.cpp \
	UIStash.cpp \
	UIStashListElement.cpp

INCLUDES = -I$(srcdir)/..
INCLUDES += -isystem @top_srcdir@/contrib -isystem @top_srcdir@/contrib/rocket/include
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
