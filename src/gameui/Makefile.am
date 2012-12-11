include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -isystem $(top_srcdir)/contrib -I$(srcdir)/..
if !HAVE_LUA
INCLUDES += -isystem $(top_srcdir)/contrib/lua
endif

noinst_LIBRARIES = libgameui.a
noinst_HEADERS = \
	GameUI.h \
	Lua.h \
	Panel.h \
	Face.h \
	ShipSpinner.h

libgameui_a_SOURCES = \
	Panel.cpp \
	Face.cpp \
	ShipSpinner.cpp \
	Lua.cpp \
	LuaFace.cpp \
	LuaShipSpinner.cpp
