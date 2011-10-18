include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libterrain.a
noinst_HEADERS = \
	Terrain.h \
	TerrainNoise.h \
	TerrainComponent.h

libterrain_a_SOURCES = \
	Terrain.cpp \
	TerrainComponent.cpp \
	TerrainHeightFlat.cpp \
	TerrainColorSolid.cpp
	
INCLUDES += -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
