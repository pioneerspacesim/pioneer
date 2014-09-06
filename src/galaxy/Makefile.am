include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -isystem $(top_srcdir)/contrib -I$(srcdir)/..
if !HAVE_LUA
AM_CPPFLAGS += -isystem $(top_srcdir)/contrib/lua
endif

noinst_LIBRARIES = libgalaxy.a
noinst_HEADERS = \
	CustomSystem.h \
	Economy.h \
	Galaxy.h \
	GalaxyCache.h \
	GalaxyGenerator.h \
	Sector.h \
	SectorGenerator.h \
	StarSystem.h \
	StarSystemGenerator.h \
	SystemPath.h

libgalaxy_a_SOURCES = \
	CustomSystem.cpp \
	Economy.cpp \
	Galaxy.cpp \
	GalaxyCache.cpp \
	GalaxyGenerator.cpp \
	Sector.cpp \
	SectorGenerator.cpp \
	StarSystem.cpp \
	StarSystemGenerator.cpp \
	SystemPath.cpp
