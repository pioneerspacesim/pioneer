set(TARGETNAME "Pioneer")

set(GROUP_LIST base graphics gui collider terrain platformSpecific)

set( ${TARGETNAME}_H_base
	"Aabb.h"
	"AmbientSounds.h"
	"Background.h"
	"BezierCurve.h"
	"Body.h"
	"BufferObject.h"
	"buildopts.h"
	"ByteRange.h"
	"Camera.h"
	"CargoBody.h"
	"ChatForm.h"
	"CityOnPlanet.h"
	"Color.h"
	"CommodityTradeWidget.h"
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
	"Sector.h"
	"SectorView.h"
	"Serializer.h"
	"Sfx.h"
	"Ship.h"
	"ShipAICmd.h"
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
	"CustomSystem.cpp"
	"DeadVideoLink.cpp"
	"DynamicBody.cpp"
	"enum_table.cpp"
	"EquipType.cpp"
	"FaceVideoLink.cpp"
	"FileSelectorWidget.cpp"
	"FileSystem.cpp"
	#"FileSystemPosix.cpp"
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
	"Sector.cpp"
	"SectorView.cpp"
	"Serializer.cpp"
	"Sfx.cpp"
	"Ship-AI.cpp"
	"Ship.cpp"
	"ShipAICmd.cpp"
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
	"WorldView.cpp"
)

set( ${TARGETNAME}_H_graphics
	"graphics/Drawables.h"
	"graphics/Frustum.h"
	"graphics/Graphics.h"
	"graphics/Material.h"
	"graphics/Renderer.h"
	"graphics/RendererGL2.h"
	"graphics/RendererGLBuffers.h"
	"graphics/RendererLegacy.h"
	"graphics/Shader.h"
	"graphics/StaticMesh.h"
	"graphics/Surface.h"
	"graphics/Texture.h"
	"graphics/TextureBuilder.h"
	"graphics/TextureGL.h"
	"graphics/VertexArray.h"
)
set( ${TARGETNAME}_SRC_graphics
	"graphics/Drawables.cpp"
	"graphics/Frustum.cpp"
	"graphics/Graphics.cpp"
	"graphics/Material.cpp"
	"graphics/Renderer.cpp"
	"graphics/RendererGL2.cpp"
	"graphics/RendererLegacy.cpp"
	"graphics/Shader.cpp"
	"graphics/StaticMesh.cpp"
	"graphics/TextureBuilder.cpp"
	"graphics/TextureGL.cpp"
	"graphics/VertexArray.cpp"
)

set( ${TARGETNAME}_H_gui
	"gui/Gui.h"
	"gui/GuiAdjustment.h"
	"gui/GuiBox.h"
	"gui/GuiButton.h"
	"gui/GuiContainer.h"
	"gui/GuiEvents.h"
	"gui/GuiFixed.h"
	"gui/GuiGradient.h"
	"gui/GuiImage.h"
	"gui/GuiImageButton.h"
	"gui/GuiImageRadioButton.h"
	"gui/GuiISelectable.h"
	"gui/GuiLabel.h"
	"gui/GuiLabelSet.h"
	"gui/GuiMeterBar.h"
	"gui/GuiMultiStateImageButton.h"
	"gui/GuiRadioButton.h"
	"gui/GuiRadioGroup.h"
	"gui/GuiRepeaterButton.h"
	"gui/GuiScreen.h"
	"gui/GuiStack.h"
	"gui/GuiTabbed.h"
	"gui/GuiTextEntry.h"
	"gui/GuiTextLayout.h"
	"gui/GuiTexturedQuad.h"
	"gui/GuiToggleButton.h"
	"gui/GuiToolTip.h"
	"gui/GuiVScrollBar.h"
	"gui/GuiVScrollPortal.h"
	"gui/GuiWidget.h"
)
set( ${TARGETNAME}_SRC_gui
	"gui/Gui.cpp"
	"gui/GuiBox.cpp"
	"gui/GuiButton.cpp"
	"gui/GuiContainer.cpp"
	"gui/GuiFixed.cpp"
	"gui/GuiGradient.cpp"
	"gui/GuiImage.cpp"
	"gui/GuiImageButton.cpp"
	"gui/GuiImageRadioButton.cpp"
	"gui/GuiLabel.cpp"
	"gui/GuiLabelSet.cpp"
	"gui/GuiMeterBar.cpp"
	"gui/GuiMultiStateImageButton.cpp"
	"gui/GuiRadioButton.cpp"
	"gui/GuiRadioGroup.cpp"
	"gui/GuiRepeaterButton.cpp"
	"gui/GuiScreen.cpp"
	"gui/GuiStack.cpp"
	"gui/GuiTabbed.cpp"
	"gui/GuiTextEntry.cpp"
	"gui/GuiTextLayout.cpp"
	"gui/GuiTexturedQuad.cpp"
	"gui/GuiToggleButton.cpp"
	"gui/GuiToolTip.cpp"
	"gui/GuiVScrollBar.cpp"
	"gui/GuiVScrollPortal.cpp"
	"gui/GuiWidget.cpp"
)

set( ${TARGETNAME}_H_collider
	"collider/BVHTree.h"
	"collider/collider.h"
	"collider/CollisionContact.h"
	"collider/CollisionSpace.h"
	"collider/Geom.h"
	"collider/GeomTree.h"
)
set( ${TARGETNAME}_SRC_collider
	"collider/BVHTree.cpp"
	"collider/CollisionSpace.cpp"
	"collider/Geom.cpp"
	"collider/GeomTree.cpp"
)

set( ${TARGETNAME}_H_terrain
	"terrain/Terrain.h"
	"terrain/TerrainFeature.h"
	"terrain/TerrainNoise.h"
)
set( ${TARGETNAME}_SRC_terrain
	"terrain/Terrain.cpp"
	"terrain/TerrainColorAsteroid.cpp"
	"terrain/TerrainColorBandedRock.cpp"
	"terrain/TerrainColorDeadWithWater.cpp"
	"terrain/TerrainColorDesert.cpp"
	"terrain/TerrainColorEarthLike.cpp"
	"terrain/TerrainColorGGJupiter.cpp"
	"terrain/TerrainColorGGNeptune.cpp"
	"terrain/TerrainColorGGNeptune2.cpp"
	"terrain/TerrainColorGGSaturn.cpp"
	"terrain/TerrainColorGGSaturn2.cpp"
	"terrain/TerrainColorGGUranus.cpp"
	"terrain/TerrainColorIce.cpp"
	"terrain/TerrainColorMethane.cpp"
	"terrain/TerrainColorRock.cpp"
	"terrain/TerrainColorRock2.cpp"
	"terrain/TerrainColorSolid.cpp"
	"terrain/TerrainColorStarBrownDwarf.cpp"
	"terrain/TerrainColorStarG.cpp"
	"terrain/TerrainColorStarK.cpp"
	"terrain/TerrainColorStarM.cpp"
	"terrain/TerrainColorStarWhiteDwarf.cpp"
	"terrain/TerrainColorTFGood.cpp"
	"terrain/TerrainColorTFPoor.cpp"
	"terrain/TerrainColorVolcanic.cpp"
	"terrain/TerrainFeature.cpp"
	"terrain/TerrainHeightAsteroid.cpp"
	"terrain/TerrainHeightFlat.cpp"
	"terrain/TerrainHeightHillsCraters.cpp"
	"terrain/TerrainHeightHillsCraters2.cpp"
	"terrain/TerrainHeightHillsDunes.cpp"
	"terrain/TerrainHeightHillsNormal.cpp"
	"terrain/TerrainHeightHillsRidged.cpp"
	"terrain/TerrainHeightHillsRivers.cpp"
	"terrain/TerrainHeightMapped.cpp"
	"terrain/TerrainHeightMapped2.cpp"
	"terrain/TerrainHeightMountainsCraters.cpp"
	"terrain/TerrainHeightMountainsCraters2.cpp"
	"terrain/TerrainHeightMountainsNormal.cpp"
	"terrain/TerrainHeightMountainsRidged.cpp"
	"terrain/TerrainHeightMountainsRivers.cpp"
	"terrain/TerrainHeightMountainsRiversVolcano.cpp"
	"terrain/TerrainHeightMountainsVolcano.cpp"
	"terrain/TerrainHeightRuggedDesert.cpp"
	"terrain/TerrainHeightRuggedLava.cpp"
	"terrain/TerrainHeightWaterSolid.cpp"
	"terrain/TerrainHeightWaterSolidCanyons.cpp"
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
endif()


IF (${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
	LIST(APPEND ${TARGETNAME}_H_base "${PCH_DEFAULT_H}")
	LIST(APPEND ${TARGETNAME}_SRC_base "${PCH_DEFAULT_CPP}")
ENDIF()

# Add and group sources
SET (GROUP_NAME_HEADERS "Header files")
SET (GROUP_NAME_SOURCE "Source Files")
SET (TARGET_H )
SET (TARGET_SRC )
foreach(group ${GROUP_LIST})
	list(APPEND TARGET_H ${${TARGETNAME}_H_${group}})
	list(APPEND TARGET_SRC ${${TARGETNAME}_SRC_${group}})
	source_group("${GROUP_NAME_HEADERS}\\${group}" FILES ${${TARGETNAME}_H_${group}})
	source_group("${GROUP_NAME_SOURCE}\\${group}" FILES ${${TARGETNAME}_SRC_${group}})
endforeach()



# Setup resources (executable icon)
SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES )
IF(MSVC)
	SET(SETUP_${PROJECT_NAME}_EXE_RESSOURCES "${PROJECT_SOURCE_DIR}/data/appIcon/Pioneer.rc")
ENDIF()

SET(PLATFORM_SPECIFIC_CONTROL )
IF(WIN32 AND NOT ${PROJECT_NAME}_FORCE_CONSOLE_APPS)
	SET(PLATFORM_SPECIFIC_CONTROL WIN32)
ENDIF()

add_executable(${TARGETNAME} ${PLATFORM_SPECIFIC_CONTROL} ${TARGET_SRC} ${TARGET_H} ${SETUP_${PROJECT_NAME}_EXE_RESSOURCES})

# Setup dependencies
SET(DEPENDENCIES_NAMES )

IF(UNIX)
	LIST(APPEND DEPENDENCIES_NAMES MATH_LIBRARY X11)
	ADD_DEFINITIONS( -D${PROJECT_NAME}_USE_X11 )
ENDIF()

#LIST(APPEND DEPENDENCIES_NAMES osgGIS osgModeling PROJ4)

#LIST(APPEND DEPENDENCIES_NAMES Boost_DATE_TIME Boost_FILESYSTEM Boost_PROGRAM_OPTIONS Boost_SERIALIZATION Boost_SYSTEM Boost_REGEX Boost_LOCALE Boost_THREAD)
#LIST(APPEND DEPENDENCIES_NAMES ICU)		# For boost.regex & boost.locale

INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )

# ADD_DEFINITIONS( 
	# -DBOOST_ENABLE_ASSERT_HANDLER
	# -DBOOST_ALL_DYN_LINK	# Tell boost to use dynamic libraries
	# -DBOOST_ALL_NO_LIB		# No boost auto link
# )

IF (${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
	SET_USE_PRECOMPILED_HEADER(${TARGETNAME} ${PCH_DEFAULT_H} ${PCH_DEFAULT_CPP})
ENDIF()

IF(MSVC)
	# Ugly workaround to remove the "/debug" or "/release" in each output
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../")
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION})

#############################################################
# PACKAGE
INCLUDE(ModuleInstall)
#############################################################
