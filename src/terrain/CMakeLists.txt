set(TARGETNAME "terrain")

set( ${TARGETNAME}_H
	"Terrain.h"
	"TerrainFeature.h"
	"TerrainNoise.h"
)
set( ${TARGETNAME}_SRC
	"Terrain.cpp"
	"TerrainColorAsteroid.cpp"
	"TerrainColorBandedRock.cpp"
	"TerrainColorDeadWithWater.cpp"
	"TerrainColorDesert.cpp"
	"TerrainColorEarthLike.cpp"
	"TerrainColorGGJupiter.cpp"
	"TerrainColorGGNeptune.cpp"
	"TerrainColorGGNeptune2.cpp"
	"TerrainColorGGSaturn.cpp"
	"TerrainColorGGSaturn2.cpp"
	"TerrainColorGGUranus.cpp"
	"TerrainColorIce.cpp"
	"TerrainColorMethane.cpp"
	"TerrainColorRock.cpp"
	"TerrainColorRock2.cpp"
	"TerrainColorSolid.cpp"
	"TerrainColorStarBrownDwarf.cpp"
	"TerrainColorStarG.cpp"
	"TerrainColorStarK.cpp"
	"TerrainColorStarM.cpp"
	"TerrainColorStarWhiteDwarf.cpp"
	"TerrainColorTFGood.cpp"
	"TerrainColorTFPoor.cpp"
	"TerrainColorVolcanic.cpp"
	"TerrainFeature.cpp"
	"TerrainHeightAsteroid.cpp"
	"TerrainHeightFlat.cpp"
	"TerrainHeightHillsCraters.cpp"
	"TerrainHeightHillsCraters2.cpp"
	"TerrainHeightHillsDunes.cpp"
	"TerrainHeightHillsNormal.cpp"
	"TerrainHeightHillsRidged.cpp"
	"TerrainHeightHillsRivers.cpp"
	"TerrainHeightMapped.cpp"
	"TerrainHeightMapped2.cpp"
	"TerrainHeightMountainsCraters.cpp"
	"TerrainHeightMountainsCraters2.cpp"
	"TerrainHeightMountainsNormal.cpp"
	"TerrainHeightMountainsRidged.cpp"
	"TerrainHeightMountainsRivers.cpp"
	"TerrainHeightMountainsRiversVolcano.cpp"
	"TerrainHeightMountainsVolcano.cpp"
	"TerrainHeightRuggedDesert.cpp"
	"TerrainHeightRuggedLava.cpp"
	"TerrainHeightWaterSolid.cpp"
	"TerrainHeightWaterSolidCanyons.cpp"
)


# Setup dependencies
SETUP_DEFAULT_DEPENDENCIES_LIST(DEPENDENCIES_NAMES)

LINK_DIRS_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})
add_library("${TARGETNAME}" STATIC ${${TARGETNAME}_H} ${${TARGETNAME}_SRC})
LINK_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})

MAKE_ENUM_DEPEND(${TARGETNAME})
#target_link_libraries(${TARGETNAME} LUA OOLUA)

# ADD_DEFINITIONS( 
	# -DBOOST_ENABLE_ASSERT_HANDLER
	# -DBOOST_ALL_DYN_LINK	# Tell boost to use dynamic libraries
	# -DBOOST_ALL_NO_LIB		# No boost auto link
# )

IF (${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
	SET_USE_PRECOMPILED_HEADER(${TARGETNAME} ${PCH_DEFAULT_H} ${PCH_DEFAULT_CPP})
ENDIF()
