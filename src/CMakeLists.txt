# Note: Luomu proposed to split the project into (static) libraries (gui, collider, etc.). This may be a good idea.

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

set(GROUP_LIST base graphics gui collider platformSpecific)		# terrain

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

# Same as Pioneer.
set( ${TARGETNAME}_H_graphics
	${Pioneer_H_graphics}
)
set( ${TARGETNAME}_SRC_graphics
	${Pioneer_SRC_graphics}
)

set( ${TARGETNAME}_H_gui
	"gui/Gui.h"
	"gui/GuiAdjustment.h"
	"gui/GuiBox.h"
	"gui/GuiButton.h"
	"gui/GuiContainer.h"
	"gui/GuiEvents.h"
	"gui/GuiFixed.h"
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

# Same as Pioneer.
set( ${TARGETNAME}_H_collider
	${Pioneer_H_collider}
)
set( ${TARGETNAME}_SRC_collider
	${Pioneer_SRC_collider}
)

# "terrain" is not used, but we clear the variable just for safety
set( ${TARGETNAME}_H_terrain
)
set( ${TARGETNAME}_SRC_terrain
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

