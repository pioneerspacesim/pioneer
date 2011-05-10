## Process this file with automake to produce Makefile.in
SUBDIRS = collider/ lua/ oolua/
AM_CPPFLAGS = -DPIONEER_DATA_DIR=\"@PIONEER_DATA_DIR@\"

bin_PROGRAMS = pioneer modelviewer
noinst_LIBRARIES = libgui.a
noinst_HEADERS = \
	Aabb.h \
	AmbientSounds.h \
	BBAdvertChatForm.h \
	BezierCurve.h \
	Body.h \
	BufferObject.h \
	CargoBody.h \
	CityOnPlanet.h \
	Color.h \
	CommodityTradeWidget.h \
	CustomSystem.h \
	DeadVideoLink.h \
	DeleteEmitter.h \
	DynamicBody.h \
	EquipType.h \
	FaceVideoLink.h \
	Frame.h \
	GalacticView.h \
	Galaxy.h \
	GameMenuView.h \
	GenericChatForm.h \
	GeoSphere.h \
	GeoSphereStyle.h \
	Gui.h \
	GuiAdjustment.h \
	GuiBox.h \
	GuiButton.h \
	GuiContainer.h \
	GuiEvents.h \
	GuiFixed.h \
	GuiISelectable.h \
	GuiImage.h \
	GuiImageButton.h \
	GuiImageRadioButton.h \
	GuiLabel.h \
	GuiLabelSet.h \
	GuiMeterBar.h \
	GuiMultiStateImageButton.h \
	GuiRadioButton.h \
	GuiRadioGroup.h \
	GuiRepeaterButton.h \
	GuiScreen.h \
	GuiTabbed.h \
	GuiTextEntry.h \
	GuiTextLayout.h \
	GuiToggleButton.h \
	GuiToolTip.h \
	GuiVScrollBar.h \
	GuiVScrollPortal.h \
	GuiWidget.h \
	HyperspaceCloud.h \
	InfoView.h \
	IniConfig.h \
	KeyBindings.h \
	LmrModel.h \
	LuaBody.h \
	LuaCargoBody.h \
	LuaChatForm.h \
	LuaConstants.h \
	LuaEngine.h \
	LuaEquipType.h \
	LuaEventQueue.h \
	LuaFormat.h \
	LuaGame.h \
	LuaManager.h \
	LuaNameGen.h \
	LuaObject.h \
	LuaPlanet.h \
	LuaPlayer.h \
	LuaRand.h \
	LuaSBody.h \
	LuaSBodyPath.h \
	LuaSerializer.h \
	LuaShip.h \
	LuaShipType.h \
	LuaSpace.h \
	LuaSpaceStation.h \
	LuaStar.h \
	LuaStarSystem.h \
	LuaTimer.h \
	LuaUI.h \
	LuaUtils.h \
	MarketAgent.h \
	Missile.h \
	ModelBody.h \
	MyLuaMathTypes.h \
	NameGenerator.h \
	Object.h \
	ObjectViewerView.h \
	PersistSystemData.h \
	Pi.h \
	PiLuaClasses.h \
	Planet.h \
	Player.h \
	PoliceChatForm.h \
	Polit.h \
	Projectile.h \
	Quaternion.h \
	RefCounted.h \
	RefList.h \
	Render.h \
	Sector.h \
	SectorView.h \
	Serializer.h \
	Sfx.h \
	Ship.h \
	ShipAICmd.h \
	ShipCpanel.h \
	ShipCpanelMultiFuncDisplays.h \
	ShipFlavour.h \
	ShipType.h \
	Sound.h \
	Space.h \
	SpaceStation.h \
	SpaceStationView.h \
	Star.h \
	StarSystem.h \
	SysLoc.h \
	SystemInfoView.h \
	SystemView.h \
	VideoLink.h \
	View.h \
	WorldView.h \
	buildopts.h \
	fixed.h \
	gameconsts.h \
	glfreetype.h \
	libs.h \
	matrix4x4.h \
	mtrand.h \
	perlin.h \
	utils.h \
	vector3.h

libgui_a_SOURCES = GuiButton.cpp Gui.cpp GuiFixed.cpp GuiScreen.cpp GuiLabel.cpp GuiToolTip.cpp GuiToggleButton.cpp GuiRadioButton.cpp \
		GuiRadioGroup.cpp GuiImageButton.cpp GuiImage.cpp GuiImageRadioButton.cpp GuiMultiStateImageButton.cpp GuiWidget.cpp \
		GuiContainer.cpp GuiVScrollPortal.cpp GuiVScrollBar.cpp GuiTabbed.cpp GuiBox.cpp GuiTextLayout.cpp GuiRepeaterButton.cpp \
		GuiTextEntry.cpp GuiMeterBar.cpp GuiLabelSet.cpp

pioneer_SOURCES	= main.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp \
		ObjectViewerView.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp ShipFlavour.cpp \
		Polit.cpp CityOnPlanet.cpp ShipCpanelMultiFuncDisplays.cpp \
		Projectile.cpp Render.cpp Sound.cpp CommodityTradeWidget.cpp GenericChatForm.cpp PoliceChatForm.cpp \
		SysLoc.cpp GalacticView.cpp Galaxy.cpp GameMenuView.cpp Missile.cpp HyperspaceCloud.cpp \
		LmrModel.cpp MyLuaMathTypes.cpp LuaChatForm.cpp IniConfig.cpp \
		GeoSphereStyle.cpp AmbientSounds.cpp KeyBindings.cpp ShipAICmd.cpp PiLuaClasses.cpp LuaConstants.cpp \
		CustomSystem.cpp LuaManager.cpp LuaObject.cpp LuaEventQueue.cpp LuaUtils.cpp LuaSerializer.cpp \
		LuaBody.cpp LuaShip.cpp LuaSpaceStation.cpp LuaPlanet.cpp LuaStar.cpp LuaPlayer.cpp LuaStarSystem.cpp \
		LuaSBodyPath.cpp LuaShipType.cpp LuaSpace.cpp LuaGame.cpp LuaUI.cpp LuaFormat.cpp LuaTimer.cpp LuaRand.cpp \
		LuaEngine.cpp LuaNameGen.cpp LuaCargoBody.cpp LuaEquipType.cpp LuaSBody.cpp BBAdvertChatForm.cpp \
		DeadVideoLink.cpp FaceVideoLink.cpp
pioneer_LDADD =  lua/liblua.a oolua/liboolua.a collider/libcollider.a libgui.a

modelviewer_SOURCES = LuaModelViewer.cpp glfreetype.cpp LmrModel.cpp perlin.cpp Render.cpp utils.cpp MyLuaMathTypes.cpp LuaUtils.cpp
modelviewer_LDADD = collider/libcollider.a lua/liblua.a libgui.a

#check_PROGRAMS = tests
tests_SOURCES = tests.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp \
		ObjectViewerView.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp 
tests_LDADD = collider/libcollider.a libgui.a

INCLUDES = -I@top_srcdir@/src/lua
