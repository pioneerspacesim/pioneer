## Process this file with automake to produce Makefile.in
SUBDIRS = sbre/ collider/ mods/

bin_PROGRAMS = pioneer sbreviewer
noinst_LIBRARIES = libgui.a
include_HEADERS = Body.h Frame.h GenericSystemView.h glfreetype.h GuiButton.h GuiContainer.h GuiEvents.h GuiFixed.h \
		Gui.h GuiImageButton.h GuiImage.h GuiImageRadioButton.h GuiLabel.h GuiMultiStateImageButton.h GuiRadioButton.h \
		GuiRadioGroup.h GuiScreen.h GuiToggleButton.h GuiWidget.h GuiToolTip.h libs.h matrix4x4.h mtrand.h Pi.h \
		Planet.h Player.h DynamicBody.h Sector.h SectorView.h ShipCpanel.h Ship.h Space.h Star.h StarSystem.h SystemInfoView.h \
		SystemView.h vector3.h View.h WorldView.h utils.h SpaceStation.h SpaceStationView.h ModelBody.h GuiISelectable.h \
		ShipType.h Object.h InfoView.h ModelCollMeshData.h ObjectViewerView.h custom_starsystems.h Aabb.h Serializer.h \
		Sfx.h GuiVScrollPortal.h GuiVScrollBar.h GuiAdjustment.h MarketAgent.h GuiTabbed.h EquipType.h CargoBody.h \
		NameGenerator.h perlin.h GeoSphere.h GuiBox.h

libgui_a_SOURCES = GuiButton.cpp Gui.cpp GuiFixed.cpp GuiScreen.cpp GuiLabel.cpp GuiToolTip.cpp GuiToggleButton.cpp GuiRadioButton.cpp \
		GuiRadioGroup.cpp GuiImageButton.cpp GuiImage.cpp GuiImageRadioButton.cpp GuiMultiStateImageButton.cpp GuiWidget.cpp \
		GuiContainer.cpp GuiVScrollPortal.cpp GuiVScrollBar.cpp GuiTabbed.cpp GuiBox.cpp

pioneer_SOURCES	= main.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp ModelCollMeshData.cpp \
		ObjectViewerView.cpp custom_starsystems.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp
pioneer_LDADD = sbre/libsbre.a collider/libcollider.a mods/libmods.a libgui.a

sbreviewer_SOURCES = SbreViewer.cpp glfreetype.cpp model_compiler.cpp
sbreviewer_LDADD = sbre/libsbre.a collider/libcollider.a libgui.a

check_PROGRAMS = tests
tests_SOURCES = tests.cpp glfreetype.cpp Body.cpp Space.cpp Ship.cpp Player.cpp DynamicBody.cpp Planet.cpp \
		Star.cpp Frame.cpp ShipCpanel.cpp SectorView.cpp mtrand.cpp WorldView.cpp SystemView.cpp \
		StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp utils.cpp SpaceStation.cpp \
		SpaceStationView.cpp ModelBody.cpp ShipType.cpp InfoView.cpp ModelCollMeshData.cpp \
		ObjectViewerView.cpp custom_starsystems.cpp Serializer.cpp Sfx.cpp Ship-AI.cpp MarketAgent.cpp \
		EquipType.cpp CargoBody.cpp NameGenerator.cpp perlin.cpp GeoSphere.cpp Pi.cpp
tests_LDADD = collider/libcollider.a libgui.a mods/libmods.a sbre/libsbre.a

