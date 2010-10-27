include_directories(lua)

# =============================================================================
# libgui

set(SOURCES_GUI
	GuiButton.cpp Gui.cpp GuiFixed.cpp GuiScreen.cpp GuiLabel.cpp GuiToolTip.cpp GuiToggleButton.cpp GuiRadioButton.cpp
	GuiRadioGroup.cpp GuiImageButton.cpp GuiImage.cpp GuiImageRadioButton.cpp GuiMultiStateImageButton.cpp GuiWidget.cpp
	GuiContainer.cpp GuiVScrollPortal.cpp GuiVScrollBar.cpp GuiTabbed.cpp GuiBox.cpp GuiTextLayout.cpp GuiRepeaterButton.cpp
	GuiTextEntry.cpp
)
add_library(gui STATIC ${SOURCES_GUI})

# =============================================================================
# pioneer

set(SOURCES_PIONEER
	main.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp
	Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp
	StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp utils.cpp SpaceStation.cpp
	SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp
	ObjectViewerView.cpp custom_starsystems.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp
	EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp ShipFlavour.cpp
	Mission.cpp Polit.cpp CityOnPlanet.cpp ShipCpanelMultiFuncDisplays.cpp
	Projectile.cpp Render.cpp Sound.cpp CommodityTradeWidget.cpp GenericChatForm.cpp PoliceChatForm.cpp
	SysLoc.cpp GalacticView.cpp Galaxy.cpp GameMenuView.cpp Missile.cpp HyperspaceCloud.cpp
	LmrModel.cpp MyLuaMathTypes.cpp PiLuaModules.cpp LuaChatForm.cpp PiLuaAPI.cpp IniConfig.cpp
	GeoSphereStyle.cpp AmbientSounds.cpp KeyBindings.cpp mylua.cpp
)
add_executable(pioneer ${SOURCES_PIONEER})
set_target_properties(pioneer PROPERTIES OUTPUT_NAME_DEBUG pioneer-dbg)
target_link_libraries(pioneer
	lua oolua collider gui
	${SIGCXX_LIBRARIES}
	${OpenGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${FREETYPE_LIBRARIES}
	${OGGVORBIS_LIBRARIES}
)

# =============================================================================
# modelviewer

set(SOURCES_MODELVIEWER
	LuaModelViewer.cpp glfreetype.cpp LmrModel.cpp perlin.cpp Render.cpp utils.cpp MyLuaMathTypes.cpp mylua.cpp
)
add_executable(modelviewer ${SOURCES_MODELVIEWER})
set_target_properties(modelviewer PROPERTIES OUTPUT_NAME_DEBUG modelviewer-dbg)
target_link_libraries(modelviewer
	lua collider gui
	${SIGCXX_LIBRARIES}
	${OpenGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${SDL_LIBRARY}
	${SDLIMAGE_LIBRARY}
	${FREETYPE_LIBRARIES}
)
