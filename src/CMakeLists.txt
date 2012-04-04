# Macro setting the common dependencies list, for this directory and subdirs
macro(SETUP_DEFAULT_DEPENDENCIES_LIST VARNAME)
	# Setup dependencies
	SET(${VARNAME} )

	IF(UNIX)
		LIST(APPEND ${VARNAME} MATH_LIBRARY X11)
		ADD_DEFINITIONS( -D${PROJECT_NAME}_USE_X11 )
	ENDIF()

	LIST(APPEND ${VARNAME} OPENGL)
	LIST(APPEND ${VARNAME} GLEW)
	LIST(APPEND ${VARNAME} FREETYPE SDL SDLIMAGE PNG)
	LIST(APPEND ${VARNAME} OGG VORBIS VORBISFILE)
	LIST(APPEND ${VARNAME} SIGCPP)

	#LIST(APPEND ${VARNAME} Boost_DATE_TIME Boost_FILESYSTEM Boost_PROGRAM_OPTIONS Boost_SERIALIZATION Boost_SYSTEM Boost_REGEX Boost_LOCALE Boost_THREAD)
	#LIST(APPEND ${VARNAME} ICU)		# For boost.regex & boost.locale
endmacro()


# --------------------------------------------------------------------------------
# Setup of libraries
add_subdirectory(graphics)
add_subdirectory(gui)
add_subdirectory(collider)
add_subdirectory(terrain)

# --------------------------------------------------------------------------------
set(TARGETNAME "Pioneer")

set(GROUP_LIST base platformSpecific)

set( ${TARGETNAME}_H_base
	"Aabb.h"
	"AmbientSounds.h"
	"Background.h"
	"BezierCurve.h"
	"Body.h"
	"BufferObject.h"
	"ByteRange.h"
	"Camera.h"
	"CargoBody.h"
	"ChatForm.h"
	"CityOnPlanet.h"
	"Color.h"
	"CommodityTradeWidget.h"
	"CRC32.h"
	"CustomSystem.h"
	"DeadVideoLink.h"
	"DeleteEmitter.h"
	"DynamicBody.h"
	"enum_table.h"
	"EquipType.h"
	"FaceVideoLink.h"
	"FileSelectorWidget.h"
	"FileSystem.h"
	"fixed.h"
	"Font.h"
	"FontCache.h"
	"FontConfig.h"
	"FormController.h"
	"Frame.h"
	"GalacticView.h"
	"Galaxy.h"
	"Game.h"
	"GameConfig.h"
	"gameconsts.h"
	"GameLoaderSaver.h"
	"GameMenuView.h"
	"GeoSphere.h"
	"HyperspaceCloud.h"
	"InfoView.h"
	"IniConfig.h"
	"KeyBindings.h"
	"libs.h"
	"Light.h"
	"LmrModel.h"
	"LuaBody.h"
	"LuaCargoBody.h"
	"LuaChatForm.h"
	"LuaConsole.h"
	"LuaConstants.h"
	"LuaEngine.h"
	"LuaEquipType.h"
	"LuaEventQueue.h"
	"LuaFormat.h"
	"LuaGame.h"
	"LuaLang.h"
	"LuaManager.h"
	"LuaMusic.h"
	"LuaNameGen.h"
	"LuaObject.h"
	"LuaPlanet.h"
	"LuaPlayer.h"
	"LuaRand.h"
	"LuaSBody.h"
	"LuaSerializer.h"
	"LuaShip.h"
	"LuaShipType.h"
	"LuaSpace.h"
	"LuaSpaceStation.h"
	"LuaStar.h"
	"LuaStarSystem.h"
	"LuaSystemPath.h"
	"LuaTimer.h"
	"LuaUI.h"
	"LuaUtils.h"
	"MarketAgent.h"
	"MathUtil.h"
	"matrix4x4.h"
	"Missile.h"
	"ModelBody.h"
	"mtrand.h"
	"MyLuaMathTypes.h"
	"Object.h"
	"ObjectViewerView.h"
	"perlin.h"
	"PersistSystemData.h"
	"Pi.h"
	"PiLuaClasses.h"
	"Planet.h"
	"Player.h"
	"Polit.h"
	"Projectile.h"
	"Quaternion.h"
	"RefCounted.h"
	"RefList.h"
	"SDLWrappers.h"
	"Sector.h"
	"SectorView.h"
	"Serializer.h"
	"Sfx.h"
	"Ship.h"
	"ShipAICmd.h"
	"ShipController.h"
	"ShipCpanel.h"
	"ShipCpanelMultiFuncDisplays.h"
	"ShipFlavour.h"
	"ShipSpinnerWidget.h"
	"ShipType.h"
	"SmartPtr.h"
	"Sound.h"
	"SoundMusic.h"
	"Space.h"
	"SpaceStation.h"
	"SpaceStationView.h"
	"Star.h"
	"StarSystem.h"
	"StationAdvertForm.h"
	"StationBulletinBoardForm.h"
	"StationCommodityMarketForm.h"
	"StationPoliceForm.h"
	"StationServicesForm.h"
	"StationShipEquipmentForm.h"
	"StationShipMarketForm.h"
	"StationShipRepairForm.h"
	"StationShipViewForm.h"
	"StationShipyardForm.h"
	"StringF.h"
	"StringRange.h"
	"SystemInfoView.h"
	"SystemView.h"
	"TerrainBody.h"
	"TextSupport.h"
	"TextureFont.h"
	"utils.h"
	"vector2.h"
	"vector3.h"
	"VectorFont.h"
	"VideoLink.h"
	"View.h"
	"WorldView.h"
	"WorldViewCamera.h"
)

set( ${TARGETNAME}_SRC_base
	"AmbientSounds.cpp"
	"Background.cpp"
	"Body.cpp"
	"Camera.cpp"
	"CargoBody.cpp"
	"ChatForm.cpp"
	"CityOnPlanet.cpp"
	"Color.cpp"
	"CommodityTradeWidget.cpp"
	"CRC32.cpp"
	"CustomSystem.cpp"
	"DeadVideoLink.cpp"
	"DynamicBody.cpp"
	"enum_table.cpp"
	"EquipType.cpp"
	"FaceVideoLink.cpp"
	"FileSelectorWidget.cpp"
	"FileSystem.cpp"
	"Font.cpp"
	"FontCache.cpp"
	"FontConfig.cpp"
	"FormController.cpp"
	"Frame.cpp"
	"GalacticView.cpp"
	"Galaxy.cpp"
	"Game.cpp"
	"GameConfig.cpp"
	"GameLoaderSaver.cpp"
	"GameMenuView.cpp"
	"GeoSphere.cpp"
	"HyperspaceCloud.cpp"
	"InfoView.cpp"
	"IniConfig.cpp"
	"KeyBindings.cpp"
	"Lang.cpp"
	"Light.cpp"
	"LmrModel.cpp"
	"LuaBody.cpp"
	"LuaCargoBody.cpp"
	"LuaChatForm.cpp"
	"LuaConsole.cpp"
	"LuaConstants.cpp"
	"LuaEngine.cpp"
	"LuaEquipType.cpp"
	"LuaEventQueue.cpp"
	"LuaFormat.cpp"
	"LuaGame.cpp"
	"LuaLang.cpp"
	"LuaManager.cpp"
	"LuaMusic.cpp"
	"LuaNameGen.cpp"
	"LuaObject.cpp"
	"LuaPlanet.cpp"
	"LuaPlayer.cpp"
	"LuaRand.cpp"
	"LuaSBody.cpp"
	"LuaSerializer.cpp"
	"LuaShip.cpp"
	"LuaShipType.cpp"
	"LuaSpace.cpp"
	"LuaSpaceStation.cpp"
	"LuaStar.cpp"
	"LuaStarSystem.cpp"
	"LuaSystemPath.cpp"
	"LuaTimer.cpp"
	"LuaUI.cpp"
	"LuaUtils.cpp"
	"main.cpp"
	"MarketAgent.cpp"
	"MathUtil.cpp"
	"Missile.cpp"
	"ModelBody.cpp"
	"mtrand.cpp"
	"MyLuaMathTypes.cpp"
	"ObjectViewerView.cpp"
	"perlin.cpp"
	"Pi.cpp"
	"PiLuaClasses.cpp"
	"Planet.cpp"
	"Player.cpp"
	"Polit.cpp"
	"Projectile.cpp"
	"SDLWrappers.cpp"
	"Sector.cpp"
	"SectorView.cpp"
	"Serializer.cpp"
	"Sfx.cpp"
	"Ship-AI.cpp"
	"Ship.cpp"
	"ShipAICmd.cpp"
	"ShipController.cpp"
	"ShipCpanel.cpp"
	"ShipCpanelMultiFuncDisplays.cpp"
	"ShipFlavour.cpp"
	"ShipSpinnerWidget.cpp"
	"ShipType.cpp"
	"Sound.cpp"
	"SoundMusic.cpp"
	"Space.cpp"
	"SpaceStation.cpp"
	"SpaceStationView.cpp"
	"Star.cpp"
	"StarSystem.cpp"
	"StationAdvertForm.cpp"
	"StationBulletinBoardForm.cpp"
	"StationCommodityMarketForm.cpp"
	"StationPoliceForm.cpp"
	"StationServicesForm.cpp"
	"StationShipEquipmentForm.cpp"
	"StationShipMarketForm.cpp"
	"StationShipRepairForm.cpp"
	"StationShipViewForm.cpp"
	"StationShipyardForm.cpp"
	"StringF.cpp"
	"SystemInfoView.cpp"
	"SystemView.cpp"
	"TerrainBody.cpp"
	"TextSupport.cpp"
	"TextureFont.cpp"
	"utils.cpp"
	"VectorFont.cpp"
	"View.cpp"
	"WorldView.cpp"
	"WorldViewCamera.cpp"
)

set( ${TARGETNAME}_H_platformSpecific )
set( ${TARGETNAME}_SRC_platformSpecific )

if (WIN32)
	set( ${TARGETNAME}_H_platformSpecific
		"${PROJECT_SOURCE_DIR}/win32/src/Gamma.h"
	)
	set( ${TARGETNAME}_SRC_platformSpecific
		"${PROJECT_SOURCE_DIR}/win32/src/FileSystemWin32.cpp"
		"${PROJECT_SOURCE_DIR}/win32/src/Gamma.cpp"
	)
else()
	list(APPEND ${TARGETNAME}_SRC_platformSpecific
		"FileSystemPosix.cpp"
	)
endif()

set(${TARGETNAME}_H_OTHER
	"${${PROJECT_NAME}_CONFIG_HEADER}"
	"${${PROJECT_NAME}_VERSION_HEADER}"
	Version.h
)
set(${TARGETNAME}_SRC_OTHER
	Version.cpp
)

# Setup resources (executable icon)
SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES )
IF(MSVC)
	SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES "${PROJECT_SOURCE_DIR}/data/appIcon/Pioneer.rc")
ENDIF()

INCLUDE(CMakeLists.sub.cmake)
target_link_libraries(${TARGETNAME} terrain)

# Use executables explicit naming to stick with existing implementation
set_target_properties(${TARGETNAME} PROPERTIES
	RUNTIME_OUTPUT_NAME                "pioneer"				# Default name (lowercase), for Release, MinSizeRel, others
	RUNTIME_OUTPUT_NAME_Debug          "pioneer-debug"
	RUNTIME_OUTPUT_NAME_RelWithDebInfo "pioneer-prerelease"
)

# Dependencies install
IF(${PROJECT_NAME}_INSTALL_DEPENDENCIES)
	# Install direct dependencies
	GET_TARGET_PROPERTY(LOCATION "${TARGETNAME}" LOCATION)
	FOREACH(CURCONFIG ${CMAKE_CONFIGURATION_TYPES})
		GET_TARGET_PROPERTY(LOCATION_${CURCONFIG} "${TARGETNAME}" LOCATION_${CURCONFIG})
	ENDFOREACH()
	CONFIGURE_FILE( 
		"${CMAKE_SOURCE_DIR}/dependencies.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/dependencies.${TARGETNAME}.cmake"
		@ONLY
	)
	SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
	INCLUDE(InstallRequiredSystemLibraries REQUIRED)
	INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dependencies.${TARGETNAME}.cmake")
ENDIF()



# --------------------------------------------------------------------------------
# ModelViewer

set(TARGETNAME "ModelViewer")

set(GROUP_LIST base platformSpecific)

set( ${TARGETNAME}_H_base
	"ByteRange.h"
	"Color.h"
	"CRC32.h"
	"enum_table.h"
	"FileSystem.h"
	"Font.h"
	"FontCache.h"
	"FontConfig.h"
	"IniConfig.h"
	"Lang.h"
	"libs.h"
	"LmrModel.h"
	"LuaConstants.h"
	"LuaUtils.h"
	"matrix4x4.h"
	"MyLuaMathTypes.h"
	"perlin.h"
	"SDLWrappers.h"
	"ShipType.h"
	"StringF.h"
	"StringRange.h"
	"TextSupport.h"
	"TextureFont.h"
	"utils.h"
	"vector2.h"
	"vector3.h"
	"VectorFont.h"
)

set( ${TARGETNAME}_SRC_base
	"Color.cpp"
	"CRC32.cpp"
	"enum_table.cpp"
	"FileSystem.cpp"
	"Font.cpp"
	"FontCache.cpp"
	"FontConfig.cpp"
	"IniConfig.cpp"
	"Lang.cpp"
	"LmrModel.cpp"
	"LuaConstants.cpp"
	"LuaModelViewer.cpp"
	"LuaUtils.cpp"
	"MyLuaMathTypes.cpp"
	"perlin.cpp"
	"SDLWrappers.cpp"
	"ShipType.cpp"
	"StringF.cpp"
	"TextSupport.cpp"
	"TextureFont.cpp"
	"utils.cpp"
	"VectorFont.cpp"
)

set( ${TARGETNAME}_H_platformSpecific )
set( ${TARGETNAME}_SRC_platformSpecific )

if (WIN32)
	set( ${TARGETNAME}_H_platformSpecific
		#"${PROJECT_SOURCE_DIR}/win32/src/Gamma.h"
	)
	set( ${TARGETNAME}_SRC_platformSpecific
		"${PROJECT_SOURCE_DIR}/win32/src/FileSystemWin32.cpp"
		#"${PROJECT_SOURCE_DIR}/win32/src/Gamma.cpp"
	)
else()
	list(APPEND ${TARGETNAME}_SRC_platformSpecific
		"FileSystemPosix.cpp"
	)
endif()

set(${TARGETNAME}_H_OTHER
	"${${PROJECT_NAME}_CONFIG_HEADER}"
	"${${PROJECT_NAME}_VERSION_HEADER}"
	Version.h
)
set(${TARGETNAME}_SRC_OTHER
	Version.cpp
)

# Setup resources (executable icon)
SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES )
IF(MSVC)
	SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES "${PROJECT_SOURCE_DIR}/data/appIcon/ModelViewer.rc")
ENDIF()

INCLUDE(CMakeLists.sub.cmake)

# Use executables explicit naming to stick with existing implementation
set_target_properties(${TARGETNAME} PROPERTIES
	RUNTIME_OUTPUT_NAME                "modelviewer"				# Default name (lowercase), for Release, MinSizeRel, others
	RUNTIME_OUTPUT_NAME_Debug          "modelviewer-debug"
	RUNTIME_OUTPUT_NAME_RelWithDebInfo "modelviewer-prerelease"
)

# Dependencies install
IF(${PROJECT_NAME}_INSTALL_DEPENDENCIES)
	# Install direct dependencies
	GET_TARGET_PROPERTY(LOCATION "${TARGETNAME}" LOCATION)
	FOREACH(CURCONFIG ${CMAKE_CONFIGURATION_TYPES})
		GET_TARGET_PROPERTY(LOCATION_${CURCONFIG} "${TARGETNAME}" LOCATION_${CURCONFIG})
	ENDFOREACH()
	CONFIGURE_FILE( 
		"${CMAKE_SOURCE_DIR}/dependencies.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/dependencies.${TARGETNAME}.cmake"
		@ONLY
	)
	SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
	INCLUDE(InstallRequiredSystemLibraries REQUIRED)
	INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dependencies.${TARGETNAME}.cmake")
ENDIF()

