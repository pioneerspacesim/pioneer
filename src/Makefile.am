## Process this file with automake to produce Makefile.in
if BUILD_WIN32
    PLATFORM_SUBDIRS = win32
endif
if BUILD_POSIX
    PLATFORM_SUBDIRS = posix
endif
SUBDIRS = collider gui graphics galaxy scenegraph terrain text ui gameui $(PLATFORM_SUBDIRS)

include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

bin_PROGRAMS = pioneer
noinst_HEADERS = \
	Aabb.h \
	AmbientSounds.h \
	AnimationCurves.h \
	Background.h \
	BezierCurve.h \
	Body.h \
	ByteRange.h \
	Camera.h \
	CameraController.h \
	CargoBody.h \
	ChatForm.h \
	CityOnPlanet.h \
	Color.h \
	CommodityTradeWidget.h \
	Cutscene.h \
	CRC32.h \
	DeadVideoLink.h \
	DeathView.h \
	DeleteEmitter.h \
	DynamicBody.h \
	Easing.h \
	EnumStrings.h \
	EquipSet.h \
	EquipType.h \
	FaceVideoLink.h \
	Factions.h \
	FileSelectorWidget.h \
	FileSystem.h \
	FontCache.h \
	Form.h \
	FormController.h \
	Frame.h \
	GalacticView.h \
	Game.h \
	GameMenuView.h \
	GeoSphere.h \
	HyperspaceCloud.h \
	IniConfig.h \
	Intro.h \
	GameConfig.h \
	KeyBindings.h \
	Lang.h \
	LangStrings.inc.h \
	Lua.h \
	LuaChatForm.h \
	LuaComms.h \
	LuaConsole.h \
	LuaConstants.h \
	LuaDev.h \
	LuaEngine.h \
	LuaEquipDef.h \
	LuaEvent.h \
	LuaFileSystem.h \
	LuaFormat.h \
	LuaGame.h \
	LuaLang.h \
	LuaMatrix.h \
	LuaVector.h \
	LuaFixed.h \
	LuaManager.h \
	LuaMissile.h \
	LuaMusic.h \
	LuaNameGen.h \
	LuaObject.h \
	LuaPushPull.h \
	LuaRef.h \
	LuaSerializer.h \
	LuaShipDef.h \
	LuaSpace.h \
	LuaTable.h \
	LuaTimer.h \
	LuaUtils.h \
	LuaWrappable.h \
	MarketAgent.h \
	MathUtil.h \
	Missile.h \
	ModelBody.h \
	ModelCache.h \
	ModManager.h \
	ModelViewer.h \
	NavLights.h \
	Object.h \
	ObjectViewerView.h \
	Orbit.h \
	OS.h \
	PersistSystemData.h \
	Pi.h \
	Planet.h \
	Player.h \
	PngWriter.h \
	Polit.h \
	Projectile.h \
	Quaternion.h \
	Random.h \
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
	ShipSpinnerWidget.h \
	ShipType.h \
	Sound.h \
	SoundMusic.h \
	Space.h \
	SpaceStation.h \
	SpaceStationType.h \
	SpaceStationView.h \
	Star.h \
	SystemInfoView.h \
	SystemView.h \
	TerrainBody.h \
	Tombstone.h \
	UIView.h \
	VideoLink.h \
	View.h \
	WorldView.h \
	SmartPtr.h \
	buildopts.h \
	fixed.h \
	gameconsts.h \
	libs.h \
	matrix4x4.h \
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
	CameraController.cpp \
	CargoBody.cpp \
	Color.cpp \
	ChatForm.cpp \
	CityOnPlanet.cpp \
	CommodityTradeWidget.cpp \
	CRC32.cpp \
	DeadVideoLink.cpp \
	DeathView.cpp \
	DynamicBody.cpp \
	EnumStrings.cpp \
	EquipType.cpp \
	FaceVideoLink.cpp \
	Factions.cpp \
	FileSelectorWidget.cpp \
	FileSourceZip.cpp \
	FileSystem.cpp \
	FontCache.cpp \
	FormController.cpp \
	Frame.cpp \
	GalacticView.cpp \
	Game.cpp \
	GameMenuView.cpp \
	GeoSphere.cpp \
	HyperspaceCloud.cpp \
	IniConfig.cpp \
	Intro.cpp \
	GameConfig.cpp \
	KeyBindings.cpp \
	Lang.cpp \
	Lua.cpp \
	LuaBody.cpp \
	LuaCargoBody.cpp \
	LuaChatForm.cpp \
	LuaComms.cpp \
	LuaConsole.cpp \
	LuaConstants.cpp \
	LuaDev.cpp \
	LuaEngine.cpp \
	LuaEquipDef.cpp \
	LuaEvent.cpp \
	LuaFaction.cpp \
	LuaFileSystem.cpp \
	LuaFormat.cpp \
	LuaGame.cpp \
	LuaLang.cpp \
	LuaMatrix.cpp \
	LuaVector.cpp \
	LuaFixed.cpp \
	LuaManager.cpp \
	LuaMissile.cpp \
	LuaMusic.cpp \
	LuaNameGen.cpp \
	LuaObject.cpp \
	LuaPlanet.cpp \
	LuaPlayer.cpp \
	LuaRand.cpp \
	LuaRef.cpp \
	LuaSystemBody.cpp \
	LuaSystemPath.cpp \
	LuaSerializer.cpp \
	LuaShip.cpp \
	LuaShipDef.cpp \
	LuaSpace.cpp \
	LuaSpaceStation.cpp \
	LuaStar.cpp \
	LuaStarSystem.cpp \
	LuaTable.cpp \
	LuaTimer.cpp \
	LuaUtils.cpp \
	MarketAgent.cpp \
	MathUtil.cpp \
	Missile.cpp \
	ModelBody.cpp \
	ModelCache.cpp \
	ModManager.cpp \
	ModelViewer.cpp \
	NavLights.cpp \
	ObjectViewerView.cpp \
	Orbit.cpp \
	Pi.cpp \
	Planet.cpp \
	Player.cpp \
	PngWriter.cpp \
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
	ShipSpinnerWidget.cpp \
	ShipType.cpp \
	Sound.cpp \
	SoundMusic.cpp \
	Space.cpp \
	SpaceStation.cpp \
	SpaceStationType.cpp \
	SpaceStationView.cpp \
	Star.cpp \
	SystemInfoView.cpp \
	SystemView.cpp \
	TerrainBody.cpp \
	Tombstone.cpp \
	UIView.cpp \
	View.cpp \
	WorldView.cpp \
	main.cpp \
	perlin.cpp \
	utils.cpp \
	enum_table.cpp

pioneer_LDADD = \
	../contrib/miniz/libminiz.a \
	../contrib/jenkins/libjenkins.a \
	collider/libcollider.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	galaxy/libgalaxy.a \
	scenegraph/libscenegraph.a \
	text/libtext.a \
	terrain/libterrain.a \
	ui/libui.a \
	gameui/libgameui.a

pioneer_LDADD += \
	$(FREETYPE_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(GL_LIBS) \
	$(SDL_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(VORBIS_LIBS) \
	$(PNG_LIBS) $(ASSIMP_LIBS)

if BUILD_WIN32
pioneer_LDFLAGS = -Wl,-Map=pioneer.map
endif

if !HAVE_LUA
pioneer_LDADD += ../contrib/lua/liblua.a
endif

check_PROGRAMS = tests uitest textstress
tests_SOURCES = \
	StringF.cpp \
	tests.cpp \
	test_Frame.cpp \
	test_StringF.cpp \
	FileSystem.cpp \
	FileSourceZip.cpp \
	test_FileSystem.cpp \
	test_Random.cpp
TESTS = tests
tests_LDADD = \
	collider/libcollider.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	terrain/libterrain.a \
    posix/libposix.a \
	../contrib/miniz/libminiz.a \
	../contrib/jenkins/libjenkins.a \
	$(SIGC_LIBS)

uitest_SOURCES = \
	uitest.cpp \
	Color.cpp \
	FileSystem.cpp \
	SDLWrappers.cpp \
	FontCache.cpp \
	IniConfig.cpp \
	StringF.cpp \
	Lang.cpp \
	Lua.cpp \
	LuaManager.cpp \
	LuaUtils.cpp \
	LuaObject.cpp \
	LuaConstants.cpp \
	LuaRef.cpp \
	PngWriter.cpp \
	EnumStrings.cpp \
	enum_table.cpp \
	utils.cpp
uitest_LDADD = \
	../contrib/jenkins/libjenkins.a \
	ui/libui.a \
	gui/libgui.a \
	text/libtext.a \
	graphics/libgraphics.a \
	posix/libposix.a

uitest_LDADD += \
	$(FREETYPE_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(GL_LIBS) \
	$(SDL_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(PNG_LIBS)

if !HAVE_LUA
uitest_LDADD += ../contrib/lua/liblua.a
endif

textstress_SOURCES = \
	textstress.cpp \
	Color.cpp \
	FileSystem.cpp \
	SDLWrappers.cpp \
	FontCache.cpp \
	IniConfig.cpp \
	StringF.cpp \
	Lang.cpp \
	PngWriter.cpp \
	utils.cpp
textstress_LDADD = \
	gui/libgui.a \
	text/libtext.a \
	graphics/libgraphics.a \
	posix/libposix.a

textstress_LDADD += \
	$(FREETYPE_LIBS) $(GLEW_LIBS) $(GLU_LIBS) $(GL_LIBS) \
	$(SDL_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(PNG_LIBS)

if !HAVE_LUA
textstress_LDADD += ../contrib/lua/liblua.a
endif

INCLUDES = -isystem @top_srcdir@/contrib
if !HAVE_LUA
INCLUDES += -isystem @top_srcdir@/contrib/lua
endif

if BUILD_WIN32
    pioneer_LDADD += win32/libwin32.a $(MINGW_LIBS)
endif

if BUILD_POSIX
    pioneer_LDADD += posix/libposix.a
endif

if HAVE_WINDRES
# note: this can't be correctly packed into libwin32.a
SUFFIXES = .rc
pioneer_SOURCES += win32/pioneer.rc
.rc.o:
	$(AM_V_GEN)$(MXE_WINDRES) $< $@
endif
