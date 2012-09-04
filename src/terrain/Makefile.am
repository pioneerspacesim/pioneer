include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -I$(srcdir)/..

noinst_LIBRARIES = libterrain.a
noinst_HEADERS = \
	Terrain.h \
	TerrainNoise.h \
	TerrainFeature.h

libterrain_a_SOURCES = \
	Terrain.cpp \
	TerrainFeature.cpp \
	TerrainHeightAsteroid.cpp \
	TerrainHeightAsteroid2.cpp \
	TerrainHeightAsteroid3.cpp \
	TerrainHeightAsteroid4.cpp \
	TerrainHeightBarrenRock.cpp \
	TerrainHeightBarrenRock2.cpp \
	TerrainHeightBarrenRock3.cpp \
	TerrainHeightFlat.cpp \
	TerrainHeightHillsCraters2.cpp \
	TerrainHeightHillsCraters.cpp \
	TerrainHeightHillsDunes.cpp \
	TerrainHeightHillsNormal.cpp \
	TerrainHeightHillsRidged.cpp \
	TerrainHeightHillsRivers.cpp \
	TerrainHeightMapped.cpp \
	TerrainHeightMapped2.cpp \
	TerrainHeightMountainsCraters2.cpp \
	TerrainHeightMountainsCraters.cpp \
	TerrainHeightMountainsNormal.cpp \
	TerrainHeightMountainsRidged.cpp \
	TerrainHeightMountainsRivers.cpp \
	TerrainHeightMountainsRiversVolcano.cpp \
	TerrainHeightMountainsVolcano.cpp \
	TerrainHeightRuggedDesert.cpp \
	TerrainHeightRuggedLava.cpp \
	TerrainHeightWaterSolidCanyons.cpp \
	TerrainHeightWaterSolid.cpp \
	TerrainColorAsteroid.cpp \
	TerrainColorBandedRock.cpp \
	TerrainColorDeadWithWater.cpp \
	TerrainColorDesert.cpp \
	TerrainColorEarthLike.cpp \
	TerrainColorGGJupiter.cpp \
	TerrainColorGGNeptune2.cpp \
	TerrainColorGGNeptune.cpp \
	TerrainColorGGSaturn2.cpp \
	TerrainColorGGSaturn.cpp \
	TerrainColorGGUranus.cpp \
	TerrainColorIce.cpp \
	TerrainColorMethane.cpp \
	TerrainColorRock2.cpp \
	TerrainColorRock.cpp \
	TerrainColorSolid.cpp \
	TerrainColorStarBrownDwarf.cpp \
	TerrainColorStarG.cpp \
	TerrainColorStarK.cpp \
	TerrainColorStarM.cpp \
	TerrainColorStarWhiteDwarf.cpp \
	TerrainColorTFGood.cpp \
	TerrainColorTFPoor.cpp \
	TerrainColorVolcanic.cpp

INCLUDES += -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif
