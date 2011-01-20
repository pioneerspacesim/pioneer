## Process this file with automake to produce Makefile.in
SUBDIRS = collider/ lua/ oolua/

bin_PROGRAMS = pioneer modelviewer
noinst_LIBRARIES = libgui.a
include_HEADERS = Body.h Frame.h GenericSystemView.h glfreetype.h GuiButton.h GuiContainer.h GuiEvents.h GuiFixed.h \
		Gui.h GuiImageButton.h GuiImage.h GuiImageRadioButton.h GuiLabel.h GuiMultiStateImageButton.h GuiRadioButton.h \
		GuiRadioGroup.h GuiScreen.h GuiToggleButton.h GuiWidget.h GuiToolTip.h libs.h matrix4x4.h mtrand.h Pi.h \
		Planet.h Player.h DynamicBody.h Sector.h SectorView.h ShipCpanel.h Ship.h Space.h Star.h StarSystem.h SystemInfoView.h \
		SystemView.h vector3.h View.h WorldView.h utils.h SpaceStation.h SpaceStationView.h ModelBody.h GuiISelectable.h \
		ShipType.h Object.h InfoView.h ObjectViewerView.h custom_starsystems.h Aabb.h Serializer.h \
		Sfx.h GuiVScrollPortal.h GuiVScrollBar.h GuiAdjustment.h MarketAgent.h GuiTabbed.h EquipType.h CargoBody.h \
		NameGenerator.h perlin.h GeoSphere.h GuiBox.h ShipFlavour.h GuiTextLayout.h Mission.h Polit.h \
		CityOnPlanet.h ShipCpanelMultiFuncDisplays.h Projectile.h Render.h Sound.h GuiRepeaterButton.h \
		CommodityTradeWidget.h GenericChatForm.h PoliceChatForm.h SysLoc.h PersistSystemData.h GalacticView.h \
		Galaxy.h GameMenuView.h GuiTextEntry.h Missile.h HyperspaceCloud.h LmrModel.h MyLuaMathTypes.h BezierCurve.h \
		PiLuaModules.h LuaChatForm.h mylua.h PiLuaAPI.h IniConfig.h GeoSphereStyle.h AmbientSounds.h KeyBindings.h \
		BufferObject.h GuiMeterBar.h

libgui_a_SOURCES = GuiButton.cpp Gui.cpp GuiFixed.cpp GuiScreen.cpp GuiLabel.cpp GuiToolTip.cpp GuiToggleButton.cpp GuiRadioButton.cpp \
		GuiRadioGroup.cpp GuiImageButton.cpp GuiImage.cpp GuiImageRadioButton.cpp GuiMultiStateImageButton.cpp GuiWidget.cpp \
		GuiContainer.cpp GuiVScrollPortal.cpp GuiVScrollBar.cpp GuiTabbed.cpp GuiBox.cpp GuiTextLayout.cpp GuiRepeaterButton.cpp \
		GuiTextEntry.cpp GuiMeterBar.cpp

pioneer_SOURCES	= main.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp \
		ObjectViewerView.cpp custom_starsystems.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp ShipFlavour.cpp \
		Mission.cpp Polit.cpp CityOnPlanet.cpp ShipCpanelMultiFuncDisplays.cpp \
		Projectile.cpp Render.cpp Sound.cpp CommodityTradeWidget.cpp GenericChatForm.cpp PoliceChatForm.cpp \
		SysLoc.cpp GalacticView.cpp Galaxy.cpp GameMenuView.cpp Missile.cpp HyperspaceCloud.cpp \
		LmrModel.cpp MyLuaMathTypes.cpp PiLuaModules.cpp LuaChatForm.cpp PiLuaAPI.cpp IniConfig.cpp \
		GeoSphereStyle.cpp AmbientSounds.cpp KeyBindings.cpp mylua.cpp
pioneer_LDADD =  lua/liblua.a oolua/liboolua.a collider/libcollider.a libgui.a

modelviewer_SOURCES = LuaModelViewer.cpp glfreetype.cpp LmrModel.cpp perlin.cpp Render.cpp utils.cpp MyLuaMathTypes.cpp \
		      mylua.cpp
modelviewer_LDADD = collider/libcollider.a lua/liblua.a libgui.a

check_PROGRAMS = tests
tests_SOURCES = tests.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp \
		ObjectViewerView.cpp custom_starsystems.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp 
tests_LDADD = collider/libcollider.a libgui.a

INCLUDES = -I@top_srcdir@/src/lua
