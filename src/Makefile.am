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
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

bin_PROGRAMS = pioneer modelcompiler
noinst_HEADERS = \
	Aabb.h \
	AmbientSounds.h \
	AnimationCurves.h \
	Background.h \
	BaseSphere.h \
	Body.h \
	ByteRange.h \
	Camera.h \
	CameraController.h \
	CargoBody.h \
	CityOnPlanet.h \
	Color.h \
	Cutscene.h \
	CRC32.h \
	DateTime.h \
	DeathView.h \
	DeleteEmitter.h \
	DynamicBody.h \
	Easing.h \
	EnumStrings.h \
	FaceParts.h \
	Factions.h \
	FileSystem.h \
	FixedGuns.h \
	FontCache.h \
	Frame.h \
	Game.h \
	GameLog.h \
	GasGiant.h \
	GasGiantJobs.h \
	GeoSphere.h \
	HudTrail.h \
	HyperspaceCloud.h \
	IniConfig.h \
	Intro.h \
	IterationProxy.h \
	JobQueue.h \
	GameConfig.h \
	KeyBindings.h \
	Lang.h \
	LangStrings.inc.h \
	Lua.h \
	LuaComms.h \
	LuaConsole.h \
	LuaConstants.h \
	LuaDev.h \
	LuaEngine.h \
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
	LuaPiGui.h \
	LuaPushPull.h \
	LuaRef.h \
	LuaSerializer.h \
	LuaShipDef.h \
	LuaServerAgent.h \
	LuaSpace.h \
	LuaTable.h \
	LuaTimer.h \
	LuaUtils.h \
	LuaWrappable.h \
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
	PiGui.h \
	Plane.h \
	Planet.h \
	Player.h \
	PngWriter.h \
	Polit.h \
	Projectile.h \
	PropertiedObject.h \
	PropertyMap.h \
	Propulsion.h \
	Quaternion.h \
	Random.h \
	RandomColor.h \
	Range.h \
	RefCounted.h \
	SDLWrappers.h \
	SectorView.h \
	Sensors.h \
	Serializer.h \
	StringF.h \
	StringRange.h \
	Sfx.h \
	Shields.h \
	Ship.h \
	ShipAICmd.h \
	ShipCockpit.h \
	ShipController.h \
	ShipCpanel.h \
	ShipCpanelMultiFuncDisplays.h \
	ShipType.h \
	Sound.h \
	SoundMusic.h \
	Space.h \
	SpaceStation.h \
	SpaceStationType.h \
	SpeedLines.h \
	Sphere.h \
	Star.h \
	SystemInfoView.h \
	SystemView.h \
	TerrainBody.h \
	Tombstone.h \
	UIView.h \
	VideoLink.h \
	View.h \
	WorldView.h \
	ServerAgent.h \
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
	BaseSphere.cpp \
	Body.cpp \
	Camera.cpp \
	CameraController.cpp \
	CargoBody.cpp \
	Color.cpp \
	CollMesh.cpp \
	CityOnPlanet.cpp \
	CRC32.cpp \
	DateTime.cpp \
	DeathView.cpp \
	DynamicBody.cpp \
	EnumStrings.cpp \
	FaceParts.cpp \
	Factions.cpp \
	FileSourceZip.cpp \
	FileSystem.cpp \
	FixedGuns.cpp \
	FontCache.cpp \
	Frame.cpp \
	Game.cpp \
	GameLog.cpp \
	GasGiant.cpp \
	GasGiantJobs.cpp \
	GeoPatch.cpp \
	GeoPatchContext.cpp \
	GeoPatchID.cpp \
	GeoPatchJobs.cpp \
	GeoSphere.cpp \
	HudTrail.cpp \
	HyperspaceCloud.cpp \
	IniConfig.cpp \
	Intro.cpp \
	JobQueue.cpp \
	GameConfig.cpp \
	KeyBindings.cpp \
	Lang.cpp \
	Lua.cpp \
	LuaBody.cpp \
	LuaCargoBody.cpp \
	LuaComms.cpp \
	LuaConsole.cpp \
	LuaConstants.cpp \
	LuaDev.cpp \
	LuaEngine.cpp \
	LuaEvent.cpp \
	LuaFaction.cpp \
	LuaFileSystem.cpp \
	LuaFormat.cpp \
	LuaGame.cpp \
	LuaHyperspaceCloud.cpp \
	LuaLang.cpp \
	LuaMatrix.cpp \
	LuaVector.cpp \
	LuaFixed.cpp \
	LuaManager.cpp \
	LuaMissile.cpp \
	LuaModelBody.cpp \
	LuaMusic.cpp \
	LuaNameGen.cpp \
	LuaObject.cpp \
	LuaPiGui.cpp \
	LuaPlanet.cpp \
	LuaPlayer.cpp \
	LuaPropertiedObject.cpp \
	LuaRand.cpp \
	LuaRef.cpp \
	LuaServerAgent.cpp \
	LuaSystemBody.cpp \
	LuaSystemPath.cpp \
	LuaSerializer.cpp \
	LuaShip.cpp \
	LuaShipDef.cpp \
	LuaSpace.cpp \
	LuaSpaceStation.cpp \
	LuaStar.cpp \
	LuaStarSystem.cpp \
	LuaTimer.cpp \
	LuaUtils.cpp \
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
	PiGui.cpp \
	Plane.cpp \
	Planet.cpp \
	Player.cpp \
	PngWriter.cpp \
	Polit.cpp \
	Projectile.cpp \
	PropertyMap.cpp \
	Propulsion.cpp \
	RandomColor.cpp \
	SDLWrappers.cpp \
	SectorView.cpp \
	Sensors.cpp \
	Serializer.cpp \
	ServerAgent.cpp \
	StringF.cpp \
	Sfx.cpp \
	Shields.cpp \
	Ship-AI.cpp \
	Ship.cpp \
	ShipAICmd.cpp \
	ShipCockpit.cpp \
	ShipController.cpp \
	ShipCpanel.cpp \
	ShipCpanelMultiFuncDisplays.cpp \
	ShipType.cpp \
	Sound.cpp \
	SoundMusic.cpp \
	Space.cpp \
	SpaceStation.cpp \
	SpaceStationType.cpp \
	SpeedLines.cpp \
	Sphere.cpp \
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
	collider/libcollider.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	graphics/opengl/libgraphicsopengl.a \
	graphics/gl2/libgraphicsgl2.a \
	galaxy/libgalaxy.a \
	scenegraph/libscenegraph.a \
	text/libtext.a \
	terrain/libterrain.a \
	ui/libui.a \
	gameui/libgameui.a \
	../contrib/PicoDDS/libpicodds.a \
	../contrib/jenkins/libjenkins.a \
	../contrib/json/libjson.a \
	../contrib/glew/libglew.a \
	../contrib/profiler/libprofiler.a \
	../contrib/imgui/libimgui.a

pioneer_LDADD += \
	$(FREETYPE_LIBS) $(GL_LIBS) \
	$(SDL2_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(VORBIS_LIBS) \
	$(PNG_LIBS) $(CURL_LIBS) $(ASSIMP_LIBS) $(EXTRA_LIBS)

if BUILD_WIN32
pioneer_LDFLAGS = -Wl,-Map=pioneer.map
modelcompiler_LDFLAGS = -Wl,-Map=modelcompiler.map
endif

if !HAVE_LUA
pioneer_LDADD += ../contrib/lua/liblua.a
endif

if DO_STRIP
PIONEER_V_STRIP = $(PIONEER_V_STRIP_@AM_V@)
PIONEER_V_STRIP_ = $(PIONEER_V_STRIP_@AM_DEFAULT_V@)
PIONEER_V_STRIP_0 = @echo "  STRIP   " $<;
PIONEER_V_STRIP_1 =

all-local: $(bin_PROGRAMS)
	$(PIONEER_V_STRIP)$(STRIP) $(bin_PROGRAMS)
endif


check_PROGRAMS = tests uitest textstress
tests_SOURCES = \
	StringF.cpp \
	DateTime.cpp \
	tests.cpp \
	test_Frame.cpp \
	test_StringF.cpp \
	test_Random.cpp \
	test_DateTime.cpp
TESTS = tests
tests_LDADD = \
	collider/libcollider.a \
	gui/libgui.a \
	graphics/libgraphics.a \
	terrain/libterrain.a \
    posix/libposix.a \
	../contrib/PicoDDS/libpicodds.a \
	../contrib/jenkins/libjenkins.a \
	$(SIGC_LIBS)

uitest_SOURCES = \
	uitest.cpp \
	Color.cpp \
	DateTime.cpp \
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
	LuaPropertiedObject.cpp \
	LuaSerializer.cpp \
	PngWriter.cpp \
	EnumStrings.cpp \
	PropertyMap.cpp \
	enum_table.cpp \
	utils.cpp
uitest_LDADD = \
	ui/libui.a \
	gui/libgui.a \
	text/libtext.a \
	graphics/libgraphics.a \
	graphics/opengl/libgraphicsopengl.a \
	graphics/gl2/libgraphicsgl2.a \
	posix/libposix.a \
	../contrib/jenkins/libjenkins.a \
	../contrib/PicoDDS/libpicodds.a \
	../contrib/json/libjson.a \
	../contrib/glew/libglew.a \
	../contrib/profiler/libprofiler.a

uitest_LDADD += \
	$(FREETYPE_LIBS) $(GL_LIBS) \
	$(SDL2_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(PNG_LIBS)

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
	posix/libposix.a \
	../contrib/json/libjson.a \
	../contrib/glew/libglew.a \
	../contrib/profiler/libprofiler.a

textstress_LDADD += \
	$(FREETYPE_LIBS) $(GL_LIBS) \
	$(SDL2_LIBS) $(SIGC_LIBS) $(LUA_LIBS) $(PNG_LIBS)

if !HAVE_LUA
textstress_LDADD += ../contrib/lua/liblua.a
endif

modelcompiler_SOURCES = \
	modelcompiler.cpp \
	Color.cpp \
	CollMesh.cpp \
	DateTime.cpp \
	FileSourceZip.cpp \
	FileSystem.cpp \
	FontCache.cpp \
	IniConfig.cpp \
	GameConfig.cpp \
	JobQueue.cpp \
	Lang.cpp \
	ModManager.cpp \
	NavLights.cpp \
	PngWriter.cpp \
	SDLWrappers.cpp \
	Serializer.cpp \
	StringF.cpp \
	utils.cpp

modelcompiler_LDADD = \
	gui/libgui.a \
	graphics/libgraphics.a \
	graphics/dummy/libgraphicsdummy.a \
	scenegraph/libscenegraph.a \
	collider/libcollider.a \
	text/libtext.a \
	../contrib/PicoDDS/libpicodds.a \
	../contrib/json/libjson.a \
	../contrib/glew/libglew.a \
	../contrib/profiler/libprofiler.a

modelcompiler_LDADD += \
	$(FREETYPE_LIBS) \
	$(SDL2_LIBS) $(SIGC_LIBS) $(LUA_LIBS) \
	$(PNG_LIBS) $(ASSIMP_LIBS) $(EXTRA_LIBS)

if !HAVE_LUA
modelcompiler_LDADD += ../contrib/lua/liblua.a
endif

AM_CPPFLAGS += -isystem @top_srcdir@/contrib
if !HAVE_LUA
AM_CPPFLAGS += -isystem @top_srcdir@/contrib/lua
endif

if BUILD_WIN32
    pioneer_LDADD += win32/libwin32.a $(MINGW_LIBS)
    modelcompiler_LDADD += win32/libwin32.a $(MINGW_LIBS)
endif

if BUILD_POSIX
    pioneer_LDADD += posix/libposix.a
    modelcompiler_LDADD += posix/libposix.a
endif

if HAVE_WINDRES
# note: this can't be correctly packed into libwin32.a
SUFFIXES = .rc
pioneer_SOURCES += win32/pioneer.rc
.rc.o:
	$(AM_V_GEN)$(MXE_WINDRES) $< $@
endif
