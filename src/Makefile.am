## Process this file with automake to produce Makefile.in
SUBDIRS = collider gui graphics galaxy terrain text

include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

bin_PROGRAMS = pioneer modelviewer
noinst_HEADERS = \
	Aabb.h \
	AmbientSounds.h \
	Background.h \
	BezierCurve.h \
	Body.h \
	BufferObject.h \
	ByteRange.h \
	Camera.h \
	CargoBody.h \
	ChatForm.h \
	CityOnPlanet.h \
	Color.h \
	CommodityTradeWidget.h \
	CRC32.h \
	DeadVideoLink.h \
	DeleteEmitter.h \
	DynamicBody.h \
	EquipSet.h \
	EquipType.h \
	FaceVideoLink.h \
	FileSelectorWidget.h \
	FileSystem.h \
	FontCache.h \
	FontConfig.h \
	Form.h \
	FormController.h \
	Frame.h \
	GalacticView.h \
	Game.h \
	GameLoaderSaver.h \
	GameMenuView.h \
	GeoSphere.h \
	HyperspaceCloud.h \
	InfoView.h \
	IniConfig.h \
	GameConfig.h \
	KeyBindings.h \
	Lang.h \
	LangStrings.inc.h \
	Light.h \
	LmrModel.h \
	LuaBody.h \
	LuaCargoBody.h \
	LuaChatForm.h \
	LuaComms.h \
	LuaConsole.h \
	LuaConstants.h \
	LuaEngine.h \
	LuaEquipType.h \
	LuaEventQueue.h \
	LuaFormat.h \
	LuaGame.h \
	LuaLang.h \
	LuaMatrix.h \
	LuaVector.h \
	LuaFixed.h \
	LuaManager.h \
	LuaMusic.h \
	LuaNameGen.h \
	LuaObject.h \
	LuaPlanet.h \
	LuaPlayer.h \
	LuaRand.h \
	LuaSystemBody.h \
	LuaSystemPath.h \
	LuaSerializer.h \
	LuaShip.h \
	LuaShipType.h \
	LuaSpace.h \
	LuaSpaceStation.h \
	LuaStar.h \
	LuaStarSystem.h \
	LuaTimer.h \
	LuaUtils.h \
	MarketAgent.h \
	MathUtil.h \
	Missile.h \
	ModelBody.h \
	ModManager.h \
	Object.h \
	ObjectViewerView.h \
	PersistSystemData.h \
	Pi.h \
	Planet.h \
	Player.h \
	Polit.h \
	Projectile.h \
	Quaternion.h \
	RefCounted.h \
	RefList.h \
	SDLWrappers.h \
	SectorView.h \
	Serializer.h \
	StationAdvertForm.h \
	StationBulletinBoardForm.h \
	StationCommodityMarketForm.h \
	StationPoliceForm.h \
	StationServicesForm.h \
	StationShipEquipmentForm.h \
	StationShipMarketForm.h \
	StationShipRepairForm.h \
	StationShipViewForm.h \
	StationShipyardForm.h \
	StringF.h \
	StringRange.h \
	Sfx.h \
	Ship.h \
	ShipAICmd.h \
	ShipController.h \
	ShipCpanel.h \
	ShipCpanelMultiFuncDisplays.h \
	ShipFlavour.h \
	ShipSpinnerWidget.h \
	ShipType.h \
	Sound.h \
	SoundMusic.h \
	Space.h \
	SpaceStation.h \
	SpaceStationView.h \
	Star.h \
	SystemInfoView.h \
	SystemView.h \
	TerrainBody.h \
	VideoLink.h \
	View.h \
	WorldView.h \
	WorldViewCamera.h \
	SmartPtr.h \
	buildopts.h \
	fixed.h \
	gameconsts.h \
	libs.h \
	matrix4x4.h \
	mtrand.h \
	perlin.h \
	utils.h \
	vector2.h \
	vector3.h \
	enum_table.h

pioneer_SOURCES	= \
	AmbientSounds.cpp \
	Background.cpp \
	Body.cpp \
	Camera.cpp \
	CargoBody.cpp \
	Color.cpp \
	ChatForm.cpp \
	CityOnPlanet.cpp \
	CommodityTradeWidget.cpp \
	CRC32.cpp \
	DeadVideoLink.cpp \
	DynamicBody.cpp \
	EquipType.cpp \
	FaceVideoLink.cpp \
	FileSelectorWidget.cpp \
	FileSourceZip.cpp \
	FileSystem.cpp \
	FontCache.cpp \
	FontConfig.cpp \
	FormController.cpp \
	Frame.cpp \
	GalacticView.cpp \
	Game.cpp \
	GameLoaderSaver.cpp \
	GameMenuView.cpp \
	GeoSphere.cpp \
	HyperspaceCloud.cpp \
	InfoView.cpp \
	IniConfig.cpp \
	GameConfig.cpp \
	KeyBindings.cpp \
	Light.cpp \
	Lang.cpp \
	LmrModel.cpp \
	LuaBody.cpp \
	LuaCargoBody.cpp \
	LuaChatForm.cpp \
	LuaComms.cpp \
	LuaConsole.cpp \
	LuaConstants.cpp \
	LuaEngine.cpp \
	LuaEquipType.cpp \
	LuaEventQueue.cpp \
	LuaFormat.cpp \
	LuaGame.cpp \
	LuaLang.cpp \
	LuaMatrix.cpp \
	LuaVector.cpp \
	LuaFixed.cpp \
	LuaManager.cpp \
	LuaMusic.cpp \
	LuaNameGen.cpp \
	LuaObject.cpp \
	LuaPlanet.cpp \
	LuaPlayer.cpp \
	LuaRand.cpp \
	LuaSystemBody.cpp \
	LuaSystemPath.cpp \
	LuaSerializer.cpp \
	LuaShip.cpp \
	LuaShipType.cpp \
	LuaSpace.cpp \
	LuaSpaceStation.cpp \
	LuaStar.cpp \
	LuaStarSystem.cpp \
	LuaTimer.cpp \
	LuaUtils.cpp \
	MarketAgent.cpp \
	MathUtil.cpp \
	Missile.cpp \
	ModelBody.cpp \
	ModManager.cpp \
	ObjectViewerView.cpp \
	Pi.cpp \
	Planet.cpp \
	Player.cpp \
	Polit.cpp \
	Projectile.cpp \
	SDLWrappers.cpp \
	SectorView.cpp \
	Serializer.cpp \
	StationAdvertForm.cpp \
	StationBulletinBoardForm.cpp \
	StationCommodityMarketForm.cpp \
	StationPoliceForm.cpp \
	StationServicesForm.cpp \
	StationShipEquipmentForm.cpp \
	StationShipMarketForm.cpp \
	StationShipRepairForm.cpp \
	StationShipViewForm.cpp \
	StationShipyardForm.cpp \
	StringF.cpp \
	Sfx.cpp \
	Ship-AI.cpp \
	Ship.cpp \
	ShipAICmd.cpp \
	ShipController.cpp \
	ShipCpanel.cpp \
	ShipCpanelMultiFuncDisplays.cpp \
	ShipFlavour.cpp \
	ShipSpinnerWidget.cpp \
	ShipType.cpp \
	Sound.cpp \
	SoundMusic.cpp \
	Space.cpp \
	SpaceStation.cpp \
	SpaceStationView.cpp \
	Star.cpp \
	SystemInfoView.cpp \
	SystemView.cpp \
	TerrainBody.cpp \
	View.cpp \
	WorldView.cpp \
	WorldViewCamera.cpp \
	main.cpp \
	mtrand.cpp \
	perlin.cpp \
	utils.cpp \
	enum_table.cpp

pioneer_LDADD = \
	../contrib/miniz/libminiz.a \
	../contrib/jenkins/libjenkins.a \
	collider/libcollider.a \
	text/libtext.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	galaxy/libgalaxy.a \
	terrain/libterrain.a

pioneer_LDADD += \
	$(FREETYPE_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(GL_LIBS) \
	$(SDL_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(VORBIS_LIBS) \
	$(PNG_LIBS)

if !HAVE_LUA
pioneer_LDADD += ../contrib/lua/liblua.a
endif

modelviewer_SOURCES = \
	Color.cpp \
	CRC32.cpp \
	FileSourceZip.cpp \
	FileSystem.cpp \
	FontCache.cpp \
	FontConfig.cpp \
	IniConfig.cpp \
	Lang.cpp \
	LuaMatrix.cpp \
	LuaVector.cpp \
	LuaFixed.cpp \
	LmrModel.cpp \
	LuaModelViewer.cpp \
	LuaUtils.cpp \
	LuaConstants.cpp \
	ModManager.cpp \
	EquipType.cpp \
	ShipType.cpp \
	SDLWrappers.cpp \
	StringF.cpp \
	perlin.cpp \
	utils.cpp \
	enum_table.cpp
modelviewer_LDADD = \
	../contrib/miniz/libminiz.a \
	../contrib/jenkins/libjenkins.a \
	collider/libcollider.a \
	text/libtext.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	terrain/libterrain.a \
		    $(FREETYPE_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(GL_LIBS) \
		    $(SDL_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(PNG_LIBS)
if !HAVE_LUA
modelviewer_LDADD += ../contrib/lua/liblua.a
endif

check_PROGRAMS = tests
tests_SOURCES = \
	StringF.cpp \
	tests.cpp \
	test_Frame.cpp \
	test_StringF.cpp \
	FileSystem.cpp \
	FileSystemPosix.cpp \
	FileSourceZip.cpp \
	test_FileSystem.cpp

TESTS = tests

tests_LDADD = collider/libcollider.a gui/libgui.a graphics/libgraphics.a terrain/libterrain.a \
	../contrib/miniz/libminiz.a

INCLUDES = -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif

if BUILD_WIN32
    pioneer_LDADD += ../win32/src/libwin32.a $(MINGW_LIBS)
    modelviewer_LDADD += ../win32/src/libwin32.a $(MINGW_LIBS)
endif

if BUILD_POSIX
    pioneer_SOURCES += FileSystemPosix.cpp
    modelviewer_SOURCES += FileSystemPosix.cpp
endif

