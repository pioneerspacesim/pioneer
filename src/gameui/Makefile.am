include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -isystem $(top_srcdir)/contrib -I$(srcdir)/..
if !HAVE_LUA
AM_CPPFLAGS += -isystem $(top_srcdir)/contrib/lua
endif

noinst_LIBRARIES = libgameui.a
noinst_HEADERS = \
	GameUI.h \
	Lua.h \
	Panel.h \
	Face.h \
	ModelSpinner.h \
	BindingCapture.h

libgameui_a_SOURCES = \
	Panel.cpp \
	Face.cpp \
	ModelSpinner.cpp \
	BindingCapture.cpp \
	Lua.cpp \
	LuaFace.cpp \
	LuaModelSpinner.cpp \
	LuaBindingCapture.cpp
