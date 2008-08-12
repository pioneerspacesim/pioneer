## Process this file with automake to produce Makefile.in
SUBDIRS = sbre/

bin_PROGRAMS		= pioneer
pioneer_SOURCES	= main.cpp GuiButton.cpp Gui.cpp GuiFixed.cpp GuiScreen.cpp GuiLabel.cpp glfreetype.cpp \
		Body.cpp Space.cpp Ship.cpp Player.cpp GuiToggleButton.cpp GuiRadioButton.cpp \
		GuiRadioGroup.cpp DynamicBody.cpp Planet.cpp Star.cpp Frame.cpp GuiImageButton.cpp GuiImage.cpp \
		GuiImageRadioButton.cpp GuiMultiStateImageButton.cpp ShipCpanel.cpp GuiWidget.cpp SectorView.cpp \
		mtrand.cpp WorldView.cpp SystemView.cpp StarSystem.cpp Sector.cpp SystemInfoView.cpp GenericSystemView.cpp \
		GuiContainer.cpp date.cpp SpaceStation.cpp SpaceStationView.cpp ModelBody.cpp ShipType.cpp \
		InfoView.cpp ModelCollMeshData.cpp ObjectViewerView.cpp custom_starsystems.cpp
pioneer_LDADD = sbre/libsbre.a

include_HEADERS = Body.h Frame.h GenericSystemView.h glfreetype.h GuiButton.h GuiContainer.h GuiEvents.h GuiFixed.h \
		Gui.h GuiImageButton.h GuiImage.h GuiImageRadioButton.h GuiLabel.h GuiMultiStateImageButton.h GuiRadioButton.h \
		GuiRadioGroup.h GuiScreen.h GuiToggleButton.h GuiWidget.h libs.h matrix4x4.h mtrand.h Pi.h \
		Planet.h Player.h DynamicBody.h Sector.h SectorView.h ShipCpanel.h Ship.h Space.h Star.h StarSystem.h SystemInfoView.h \
		SystemView.h vector3.h View.h WorldView.h date.h SpaceStation.h SpaceStationView.h ModelBody.h GuiISelectable.h \
		ShipType.h Object.h InfoView.h ModelCollMeshData.h ObjectViewerView.h custom_starsystems.h

