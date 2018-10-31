// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "AmbientSounds.h"
#include "BaseSphere.h"
#include "CargoBody.h"
#include "CityOnPlanet.h"
#include "DeathView.h"
#include "EnumStrings.h"
#include "FaceParts.h"
#include "Factions.h"
#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "Input.h"
#include "Intro.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "LuaComms.h"
#include "LuaConsole.h"
#include "LuaConstants.h"
#include "LuaDev.h"
#include "LuaEngine.h"
#include "LuaEvent.h"
#include "LuaFileSystem.h"
#include "LuaFormat.h"
#include "LuaGame.h"
#include "LuaInput.h"
#include "LuaJson.h"
#include "LuaLang.h"
#include "LuaManager.h"
#include "LuaMissile.h"
#include "LuaMusic.h"
#include "LuaNameGen.h"
#include "LuaPiGui.h"
#include "LuaRef.h"
#include "LuaServerAgent.h"
#include "LuaShipDef.h"
#include "LuaSpace.h"
#include "LuaTimer.h"
#include "Missile.h"
#include "ModManager.h"
#include "ModelCache.h"
#include "NavLights.h"
#include "OS.h"
#include "ObjectViewerView.h"
#include "PiGui.h"
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include "Propulsion.h"
#include "SDLWrappers.h"
#include "SectorView.h"
#include "ServerAgent.h"
#include "Sfx.h"
#include "Shields.h"
#include "ShipController.h"
#include "ShipCpanel.h"
#include "ShipType.h"
#include "Sound.h"
#include "SoundMusic.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Star.h"
#include "StringF.h"
#include "SystemInfoView.h"
#include "SystemView.h"
#include "Tombstone.h"
#include "UIView.h"
#include "WorldView.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/StarSystem.h"
#include "gameui/Lua.h"
#include "libs.h"
// ------------------------------------------------------------
#include "graphics/opengl/RendererGL.h"
// ------------------------------------------------------------
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"
#include "graphics/Stats.h"
#include "gui/Gui.h"
#include "scenegraph/Lua.h"
#include "scenegraph/Model.h"
#include "ui/Context.h"
#include "ui/Lua.h"
#include "versioningInfo.h"
#include <algorithm>
#include <sstream>

#ifdef PROFILE_LUA_TIME
#include <time.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
// RegisterClassA and RegisterClassW are defined as macros in WinUser.h
#ifdef RegisterClass
#undef RegisterClass
#endif
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#define _popen popen
#define _pclose pclose
#endif

float Pi::gameTickAlpha;
sigc::signal<void> Pi::onPlayerChangeTarget;
sigc::signal<void> Pi::onPlayerChangeFlightControlState;
LuaSerializer *Pi::luaSerializer;
LuaTimer *Pi::luaTimer;
LuaNameGen *Pi::luaNameGen;
#ifdef ENABLE_SERVER_AGENT
ServerAgent *Pi::serverAgent;
#endif
Input Pi::input;
Player *Pi::player;
View *Pi::currentView;
TransferPlanner *Pi::planner;
LuaConsole *Pi::luaConsole;
Game *Pi::game;
Random Pi::rng;
float Pi::frameTime;
bool Pi::doingMouseGrab;
#if WITH_DEVKEYS
bool Pi::showDebugInfo = false;
#endif
#if PIONEER_PROFILER
std::string Pi::profilerPath;
bool Pi::doProfileSlow = false;
bool Pi::doProfileOne = false;
#endif
int Pi::statSceneTris = 0;
int Pi::statNumPatches = 0;
GameConfig *Pi::config;
DetailLevel Pi::detail;
bool Pi::navTunnelDisplayed = false;
bool Pi::speedLinesDisplayed = false;
bool Pi::hudTrailsDisplayed = false;
bool Pi::bRefreshBackgroundStars = true;
float Pi::amountOfBackgroundStarsDisplayed = 1.0f;
bool Pi::DrawGUI = true;
Graphics::Renderer *Pi::renderer;
RefCountedPtr<UI::Context> Pi::ui;
RefCountedPtr<PiGui> Pi::pigui;
ModelCache *Pi::modelCache;
Intro *Pi::intro;
SDLGraphics *Pi::sdl;
Graphics::RenderTarget *Pi::renderTarget;
RefCountedPtr<Graphics::Texture> Pi::renderTexture;
std::unique_ptr<Graphics::Drawables::TexturedQuad> Pi::renderQuad;
Graphics::RenderState *Pi::quadRenderState = nullptr;
bool Pi::bRequestEndGame = false;
bool Pi::isRecordingVideo = false;
FILE *Pi::ffmpegFile = nullptr;

Sound::MusicPlayer Pi::musicPlayer;
std::unique_ptr<AsyncJobQueue> Pi::asyncJobQueue;
std::unique_ptr<SyncJobQueue> Pi::syncJobQueue;

// Leaving define in place in case of future rendering problems.
#define USE_RTT 0

//static
void Pi::CreateRenderTarget(const Uint16 width, const Uint16 height)
{
	/*	@fluffyfreak here's a rendertarget implementation you can use for oculusing and other things. It's pretty simple:
		 - fill out a RenderTargetDesc struct and call Renderer::CreateRenderTarget
		 - pass target to Renderer::SetRenderTarget to start rendering to texture
		 - set up viewport, clear etc, then draw as usual
		 - SetRenderTarget(0) to resume render to screen
		 - you can access the attached texture with GetColorTexture to use it with a material
		You can reuse the same target with multiple textures.
		In that case, leave the color format to NONE so the initial texture is not created, then use SetColorTexture to attach your own.
	*/
#if USE_RTT
	Graphics::RenderStateDesc rsd;
	rsd.depthTest = false;
	rsd.depthWrite = false;
	rsd.blendMode = Graphics::BLEND_SOLID;
	quadRenderState = Pi::renderer->CreateRenderState(rsd);

	Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888,
		vector2f(width, height),
		Graphics::LINEAR_CLAMP, false, false, 0);
	Pi::renderTexture.Reset(Pi::renderer->CreateTexture(texDesc));
	Pi::renderQuad.reset(new Graphics::Drawables::TexturedQuad(
		Pi::renderer, Pi::renderTexture.Get(),
		vector2f(0.0f, 0.0f), vector2f(float(Graphics::GetScreenWidth()), float(Graphics::GetScreenHeight())),
		quadRenderState));

	// Complete the RT description so we can request a buffer.
	// NB: we don't want it to create use a texture because we share it with the textured quad created above.
	Graphics::RenderTargetDesc rtDesc(
		width,
		height,
		Graphics::TEXTURE_NONE, // don't create a texture
		Graphics::TEXTURE_DEPTH,
		false);
	Pi::renderTarget = Pi::renderer->CreateRenderTarget(rtDesc);

	Pi::renderTarget->SetColorTexture(Pi::renderTexture.Get());
#endif
}

//static
void Pi::DrawRenderTarget()
{
#if USE_RTT
	Pi::renderer->BeginFrame();
	Pi::renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
	Pi::renderer->SetTransform(matrix4x4f::Identity());

	{
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
		Pi::renderer->PushMatrix();
		Pi::renderer->SetOrthographicProjection(0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), 0, -1, 1);
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
		Pi::renderer->PushMatrix();
		Pi::renderer->LoadIdentity();
	}

	Pi::renderQuad->Draw(Pi::renderer);

	{
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
		Pi::renderer->PopMatrix();
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
		Pi::renderer->PopMatrix();
	}

	Pi::renderer->EndFrame();
#endif
}

//static
void Pi::BeginRenderTarget()
{
#if USE_RTT
	Pi::renderer->SetRenderTarget(Pi::renderTarget);
	Pi::renderer->ClearScreen();
#endif
}

//static
void Pi::EndRenderTarget()
{
#if USE_RTT
	Pi::renderer->SetRenderTarget(nullptr);
#endif
}

static void draw_progress(float progress)
{

	Pi::renderer->ClearScreen();
	PiGui::NewFrame(Pi::renderer->GetSDLWindow());
	Pi::DrawPiGui(progress, "INIT");
	Pi::renderer->SwapBuffers();
}

static void LuaInit()
{
	LuaObject<PropertiedObject>::RegisterClass();

	LuaObject<Body>::RegisterClass();
	LuaObject<Ship>::RegisterClass();
	LuaObject<SpaceStation>::RegisterClass();
	LuaObject<Planet>::RegisterClass();
	LuaObject<Star>::RegisterClass();
	LuaObject<Player>::RegisterClass();
	LuaObject<Missile>::RegisterClass();
	LuaObject<CargoBody>::RegisterClass();
	LuaObject<ModelBody>::RegisterClass();
	LuaObject<HyperspaceCloud>::RegisterClass();

	LuaObject<StarSystem>::RegisterClass();
	LuaObject<SystemPath>::RegisterClass();
	LuaObject<SystemBody>::RegisterClass();
	LuaObject<Random>::RegisterClass();
	LuaObject<Faction>::RegisterClass();

	Pi::luaSerializer = new LuaSerializer();
	Pi::luaTimer = new LuaTimer();

	LuaObject<LuaSerializer>::RegisterClass();
	LuaObject<LuaTimer>::RegisterClass();

	LuaConstants::Register(Lua::manager->GetLuaState());
	LuaLang::Register();
	LuaEngine::Register();
	LuaInput::Register();
	LuaFileSystem::Register();
	LuaJson::Register();
#ifdef ENABLE_SERVER_AGENT
	LuaServerAgent::Register();
#endif
	LuaGame::Register();
	LuaComms::Register();
	LuaFormat::Register();
	LuaSpace::Register();
	LuaShipDef::Register();
	LuaMusic::Register();
	LuaDev::Register();
	LuaConsole::Register();

	// XXX sigh
	UI::Lua::Init();
	GameUI::Lua::Init();
	SceneGraph::Lua::Init();

	LuaObject<PiGui>::RegisterClass();

	// XXX load everything. for now, just modules
	lua_State *l = Lua::manager->GetLuaState();
	pi_lua_import(l, "libs/autoload.lua", true);
	pi_lua_import_recursive(l, "ui");
	pi_lua_import(l, "pigui/pigui.lua", true);
	pi_lua_import(l, "pigui/game.lua", true);
	pi_lua_import(l, "pigui/init.lua", true);
	pi_lua_import(l, "pigui/mainmenu.lua", true);
	pi_lua_import_recursive(l, "pigui/modules");
	pi_lua_import_recursive(l, "modules");

	Pi::luaNameGen = new LuaNameGen(Lua::manager);
}

static void LuaUninit()
{
	delete Pi::luaNameGen;

	delete Pi::luaSerializer;
	delete Pi::luaTimer;

	Lua::Uninit();
}

static void LuaInitGame()
{
	LuaEvent::Clear();
}

SceneGraph::Model *Pi::FindModel(const std::string &name, bool allowPlaceholder)
{
	SceneGraph::Model *m = 0;
	try {
		m = Pi::modelCache->FindModel(name);
	} catch (const ModelCache::ModelNotFoundException &) {
		Output("Could not find model: %s\n", name.c_str());
		if (allowPlaceholder) {
			try {
				m = Pi::modelCache->FindModel("error");
			} catch (const ModelCache::ModelNotFoundException &) {
				Error("Could not find placeholder model");
			}
		}
	}

	return m;
}

const char Pi::SAVE_DIR_NAME[] = "savefiles";

std::string Pi::GetSaveDir()
{
	return FileSystem::JoinPath(FileSystem::GetUserDir(), Pi::SAVE_DIR_NAME);
}

void TestGPUJobsSupport()
{
	bool supportsGPUJobs = (Pi::config->Int("EnableGPUJobs") == 1);
	if (supportsGPUJobs) {
		Uint32 octaves = 8;
		for (Uint32 i = 0; i < 6; i++) {
			std::unique_ptr<Graphics::Material> material;
			Graphics::MaterialDescriptor desc;
			desc.effect = Graphics::EFFECT_GEN_GASGIANT_TEXTURE;
			desc.quality = (octaves << 16) | i;
			desc.textures = 3;
			material.reset(Pi::renderer->CreateMaterial(desc));
			supportsGPUJobs &= material->IsProgramLoaded();
		}
		if (!supportsGPUJobs) {
			// failed - retry

			// reset the GPU jobs flag
			supportsGPUJobs = true;

			// retry the shader compilation
			octaves = 5; // reduce the number of octaves
			for (Uint32 i = 0; i < 6; i++) {
				std::unique_ptr<Graphics::Material> material;
				Graphics::MaterialDescriptor desc;
				desc.effect = Graphics::EFFECT_GEN_GASGIANT_TEXTURE;
				desc.quality = (octaves << 16) | i;
				desc.textures = 3;
				material.reset(Pi::renderer->CreateMaterial(desc));
				supportsGPUJobs &= material->IsProgramLoaded();
			}

			if (!supportsGPUJobs) {
				// failed
				Warning("EnableGPUJobs DISABLED");
				Pi::config->SetInt("EnableGPUJobs", 0); // disable GPU Jobs
				Pi::config->Save();
			}
		}
	}
}

// TODO: make this a part of the class and/or improve the mechanism
void RegisterInputBindings()
{
	PlayerShipController::RegisterInputBindings();

	WorldView::RegisterInputBindings();
}

void Pi::Init(const std::map<std::string, std::string> &options, bool no_gui)
{
#ifdef PIONEER_PROFILER
	Profiler::reset();
#endif

	Profiler::Timer timer;
	timer.Start();

	OS::EnableBreakpad();
	OS::NotifyLoadBegin();

	FileSystem::Init();
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
#ifdef PIONEER_PROFILER
	FileSystem::userFiles.MakeDirectory("profiler");
	profilerPath = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
#endif
	PROFILE_SCOPED()

	Pi::config = new GameConfig(options);

	if (config->Int("RedirectStdio"))
		OS::RedirectStdio();

	std::string version(PIONEER_VERSION);
	if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
	const char *platformName = SDL_GetPlatform();
	if (platformName)
		Output("ver %s on: %s\n\n", version.c_str(), platformName);
	else
		Output("ver %s but could not detect platform name.\n\n", version.c_str());

	Output("%s\n", OS::GetOSInfoString().c_str());

	ModManager::Init();

	Lang::Resource res(Lang::GetResource("core", config->String("Lang")));
	Lang::MakeCore(res);

	Pi::SetAmountBackgroundStars(config->Float("AmountOfBackgroundStars"));
	Pi::detail.planets = config->Int("DetailPlanets");
	Pi::detail.textures = config->Int("Textures");
	Pi::detail.fracmult = config->Int("FractalMultiple");
	Pi::detail.cities = config->Int("DetailCities");

	// Initialize SDL
	Uint32 sdlInitFlags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
#if defined(DEBUG) || defined(_DEBUG)
	sdlInitFlags |= SDL_INIT_NOPARACHUTE;
#endif
	if (SDL_Init(sdlInitFlags) < 0) {
		Error("SDL initialization failed: %s\n", SDL_GetError());
	}

	OutputVersioningInfo();

	Graphics::RendererOGL::RegisterRenderer();

	// determine what renderer we should use, default to Opengl 3.x
	const std::string rendererName = config->String("RendererName", Graphics::RendererNameFromType(Graphics::RENDERER_OPENGL_3x));
	Graphics::RendererType rType = Graphics::RENDERER_OPENGL_3x;
	//if(rendererName == Graphics::RendererNameFromType(Graphics::RENDERER_OPENGL_3x))
	//{
	//	rType = Graphics::RENDERER_OPENGL_3x;
	//}

	// Do rest of SDL video initialization and create Renderer
	Graphics::Settings videoSettings = {};
	videoSettings.rendererType = rType;
	videoSettings.width = config->Int("ScrWidth");
	videoSettings.height = config->Int("ScrHeight");
	videoSettings.fullscreen = (config->Int("StartFullscreen") != 0);
	videoSettings.hidden = no_gui;
	videoSettings.requestedSamples = config->Int("AntiAliasingMode");
	videoSettings.vsync = (config->Int("VSync") != 0);
	videoSettings.useTextureCompression = (config->Int("UseTextureCompression") != 0);
	videoSettings.useAnisotropicFiltering = (config->Int("UseAnisotropicFiltering") != 0);
	videoSettings.enableDebugMessages = (config->Int("EnableGLDebug") != 0);
	videoSettings.gl3ForwardCompatible = (config->Int("GL3ForwardCompatible") != 0);
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = "Pioneer";

	Pi::renderer = Graphics::Init(videoSettings);

	Pi::CreateRenderTarget(videoSettings.width, videoSettings.height);
	Pi::rng.IncRefCount(); // so nothing tries to free it
	Pi::rng.seed(time(0));

	input.Init();
	input.onKeyPress.connect(sigc::ptr_fun(&Pi::HandleKeyDown));

	// we can only do bindings once joysticks are initialised.
	if (!no_gui) // This re-saves the config file. With no GUI we want to allow multiple instances in parallel.
		KeyBindings::InitBindings();

	RegisterInputBindings();

	navTunnelDisplayed = (config->Int("DisplayNavTunnel")) ? true : false;
	speedLinesDisplayed = (config->Int("SpeedLines")) ? true : false;
	hudTrailsDisplayed = (config->Int("HudTrails")) ? true : false;

	TestGPUJobsSupport();

	EnumStrings::Init();

	// get threads up
	Uint32 numThreads = config->Int("WorkerThreads");
	const int numCores = OS::GetNumCores();
	assert(numCores > 0);
	if (numThreads == 0) numThreads = std::max(Uint32(numCores) - 1, 1U);
	asyncJobQueue.reset(new AsyncJobQueue(numThreads));
	Output("started %d worker threads\n", numThreads);
	syncJobQueue.reset(new SyncJobQueue);

	Output("ShipType::Init()\n");
	// XXX early, Lua init needs it
	ShipType::Init();

	// XXX UI requires Lua  but Pi::ui must exist before we start loading
	// templates. so now we have crap everywhere :/
	Output("Lua::Init()\n");
	Lua::Init();

	Pi::pigui.Reset(new PiGui);
	Pi::pigui->Init(Pi::renderer->GetSDLWindow());

	float ui_scale = config->Float("UIScaleFactor", 1.0f);
	if (Graphics::GetScreenHeight() < 768) {
		ui_scale = float(Graphics::GetScreenHeight()) / 768.0f;
	}

	Pi::ui.Reset(new UI::Context(
		Lua::manager,
		Pi::renderer,
		Graphics::GetScreenWidth(),
		Graphics::GetScreenHeight(),
		ui_scale));

#ifdef ENABLE_SERVER_AGENT
	Pi::serverAgent = 0;
	if (config->Int("EnableServerAgent")) {
		const std::string endpoint(config->String("ServerEndpoint"));
		if (endpoint.size() > 0) {
			Output("Server agent enabled, endpoint: %s\n", endpoint.c_str());
			Pi::serverAgent = new HTTPServerAgent(endpoint);
		}
	}
	if (!Pi::serverAgent) {
		Output("Server agent disabled\n");
		Pi::serverAgent = new NullServerAgent();
	}
#endif

	LuaInit();

	Gui::Init(renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), 800, 600);

	// twice, to initialize the font correctly
	draw_progress(0.01f);
	draw_progress(0.01f);

	Output("GalaxyGenerator::Init()\n");
	if (config->HasEntry("GalaxyGenerator"))
		GalaxyGenerator::Init(config->String("GalaxyGenerator"),
			config->Int("GalaxyGeneratorVersion", GalaxyGenerator::LAST_VERSION));
	else
		GalaxyGenerator::Init();

	draw_progress(0.1f);

	Output("FaceParts::Init()\n");
	FaceParts::Init();
	draw_progress(0.2f);

	Output("new ModelCache\n");
	modelCache = new ModelCache(Pi::renderer);
	draw_progress(0.3f);

	Output("Shields::Init\n");
	Shields::Init(Pi::renderer);
	draw_progress(0.4f);

	//unsigned int control_word;
	//_clearfp();
	//_controlfp_s(&control_word, _EM_INEXACT | _EM_UNDERFLOW | _EM_ZERODIVIDE, _MCW_EM);
	//double fpexcept = Pi::timeAccelRates[1] / Pi::timeAccelRates[0];

	Output("BaseSphere::Init\n");
	BaseSphere::Init();
	draw_progress(0.5f);

	Output("CityOnPlanet::Init\n");
	CityOnPlanet::Init();
	draw_progress(0.6f);

	Output("SpaceStation::Init\n");
	SpaceStation::Init();
	draw_progress(0.7f);

	Output("NavLights::Init\n");
	NavLights::Init(Pi::renderer);
	draw_progress(0.75f);

	Output("Sfx::Init\n");
	SfxManager::Init(Pi::renderer);
	draw_progress(0.8f);

	if (!no_gui && !config->Int("DisableSound")) {
		Output("Sound::Init\n");
		Sound::Init();
		Sound::SetMasterVolume(config->Float("MasterVolume"));
		Sound::SetSfxVolume(config->Float("SfxVolume"));
		GetMusicPlayer().SetVolume(config->Float("MusicVolume"));

		Sound::Pause(0);
		if (config->Int("MasterMuted")) Sound::Pause(1);
		if (config->Int("SfxMuted")) Sound::SetSfxVolume(0.f);
		if (config->Int("MusicMuted")) GetMusicPlayer().SetEnabled(false);
	}
	draw_progress(0.9f);

	OS::NotifyLoadEnd();
	draw_progress(0.95f);

#if 0
	// frame test code

	Frame *root = new Frame(0, "root", 0);
	Frame *p1 = new Frame(root, "p1", Frame::FLAG_HAS_ROT);
	Frame *p1r = new Frame(p1, "p1r", Frame::FLAG_ROTATING);
	Frame *m1 = new Frame(p1, "m1", Frame::FLAG_HAS_ROT);
	Frame *m1r = new Frame(m1, "m1r", Frame::FLAG_ROTATING);
	Frame *p2 = new Frame(root, "p2", Frame::FLAG_HAS_ROT);
	Frame *p2r = new Frame(p2, "pr2", Frame::FLAG_ROTATING);

	p1->SetPosition(vector3d(1000,0,0));
	p1->SetVelocity(vector3d(0,1,0));
	p2->SetPosition(vector3d(0,2000,0));
	p2->SetVelocity(vector3d(-2,0,0));
	p1r->SetAngVelocity(vector3d(0,0,0.0001));
	p1r->SetOrient(matrix3x3d::BuildRotate(M_PI/4, vector3d(0,0,1)));
	p2r->SetAngVelocity(vector3d(0,0,-0.0004));
	p2r->SetOrient(matrix3x3d::BuildRotate(-M_PI/2, vector3d(0,0,1)));
	root->UpdateOrbitRails(0, 0);

	CargoBody *c1 = new CargoBody(Equip::Type::SLAVES);
	c1->SetFrame(p1r);
	c1->SetPosition(vector3d(0,180,0));
//	c1->SetVelocity(vector3d(1,0,0));
	CargoBody *c2 = new CargoBody(Equip::Type::SLAVES);
	c2->SetFrame(p1r);
	c2->SetPosition(vector3d(0,200,0));
//	c2->SetVelocity(vector3d(1,0,0));

	vector3d pos = c1->GetPositionRelTo(p1);
	vector3d vel = c1->GetVelocityRelTo(p1);
	double speed = vel.Length();
	vector3d pos2 = c2->GetPositionRelTo(p1);
	vector3d vel2 = c2->GetVelocityRelTo(p1);
	double speed2 = vel2.Length();

	double speed3 = c2->GetVelocityRelTo(c1).Length();
	c2->SwitchToFrame(p1);
	vector3d vel4 = c2->GetVelocityRelTo(c1);
	double speed4 = c2->GetVelocityRelTo(c1).Length();

	root->UpdateOrbitRails(0, 1.0);

	//buildrotate test

	matrix3x3d m = matrix3x3d::BuildRotate(M_PI/2, vector3d(0,0,1));
	vector3d v = m * vector3d(1,0,0);

/*	vector3d pos = p1r->GetPositionRelTo(p2r);
	vector3d vel = p1r->GetVelocityRelTo(p2r);
	matrix3x3d o1 = p1r->GetOrientRelTo(p2r);
	double speed = vel.Length();
	vector3d pos2 = p2r->GetPositionRelTo(p1r);
	vector3d vel2 = p2r->GetVelocityRelTo(p1r);
	matrix3x3d o2 = p2r->GetOrientRelTo(p1r);
	double speed2 = vel2.Length();
*/	root->UpdateOrbitRails(0, 1.0/60);

	delete p2r; delete p2; delete m1r; delete m1; delete p1r; delete p1; delete root;
	delete c1; delete c2;

#endif

#if 0
	// test code to produce list of ship stats

	FILE *pStatFile = fopen("shipstat.csv","wt");
	if (pStatFile)
	{
		fprintf(pStatFile, "name,modelname,hullmass,capacity,fakevol,rescale,xsize,ysize,zsize,facc,racc,uacc,sacc,aacc,exvel\n");
		for (auto iter : ShipType::types)
		{
			const ShipType *shipdef = &(iter.second);
			SceneGraph::Model *model = Pi::FindModel(shipdef->modelName, false);

			double hullmass = shipdef->hullMass;
			double capacity = shipdef->capacity;

			double xsize = 0.0, ysize = 0.0, zsize = 0.0, fakevol = 0.0, rescale = 0.0, brad = 0.0;
			if (model) {
				std::unique_ptr<SceneGraph::Model> inst(model->MakeInstance());
				model->CreateCollisionMesh();
				Aabb aabb = model->GetCollisionMesh()->GetAabb();
				xsize = aabb.max.x-aabb.min.x;
				ysize = aabb.max.y-aabb.min.y;
				zsize = aabb.max.z-aabb.min.z;
				fakevol = xsize*ysize*zsize;
				brad = aabb.GetRadius();
				rescale = pow(fakevol/(100 * (hullmass+capacity)), 0.3333333333);
			}

			double simass = (hullmass + capacity) * 1000.0;
			double angInertia = (2/5.0)*simass*brad*brad;
			double acc1 = shipdef->linThrust[Thruster::THRUSTER_FORWARD] / (9.81*simass);
			double acc2 = shipdef->linThrust[Thruster::THRUSTER_REVERSE] / (9.81*simass);
			double acc3 = shipdef->linThrust[Thruster::THRUSTER_UP] / (9.81*simass);
			double acc4 = shipdef->linThrust[Thruster::THRUSTER_RIGHT] / (9.81*simass);
			double acca = shipdef->angThrust/angInertia;
			double exvel = shipdef->effectiveExhaustVelocity;

			fprintf(pStatFile, "%s,%s,%.1f,%.1f,%.1f,%.3f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%f,%.1f\n",
				shipdef->name.c_str(), shipdef->modelName.c_str(), hullmass, capacity,
				fakevol, rescale, xsize, ysize, zsize, acc1, acc2, acc3, acc4, acca, exvel);
		}
		fclose(pStatFile);
	}
#endif

	luaConsole = new LuaConsole();
	KeyBindings::toggleLuaConsole.onPress.connect(sigc::mem_fun(Pi::luaConsole, &LuaConsole::Toggle));

	planner = new TransferPlanner();

	draw_progress(1.0f);

	timer.Stop();
#ifdef PIONEER_PROFILER
	Profiler::dumphtml(profilerPath.c_str());
#endif
	Output("\n\nLoading took: %lf milliseconds\n", timer.millicycles());
}

bool Pi::IsConsoleActive()
{
	return luaConsole && luaConsole->IsActive();
}

void Pi::Quit()
{
	if (Pi::ffmpegFile != nullptr) {
		_pclose(Pi::ffmpegFile);
	}
	Projectile::FreeModel();
	delete Pi::intro;
	delete Pi::luaConsole;
	NavLights::Uninit();
	Shields::Uninit();
	SfxManager::Uninit();
	Sound::Uninit();
	CityOnPlanet::Uninit();
	BaseSphere::Uninit();
	FaceParts::Uninit();
	Graphics::Uninit();
	Pi::pigui->Uninit();
	Pi::ui.Reset(0);
	Pi::pigui.Reset(0);
	LuaUninit();
	Gui::Uninit();
	delete Pi::modelCache;
	delete Pi::renderer;
	delete Pi::config;
	GalaxyGenerator::Uninit();
	delete Pi::planner;
	SDL_Quit();
	FileSystem::Uninit();
	asyncJobQueue.reset();
	syncJobQueue.reset();
	exit(0);
}

void Pi::BoinkNoise()
{
	Sound::PlaySfx("Click", 0.3f, 0.3f, false);
}

void Pi::SetView(View *v)
{
	if (currentView) currentView->Detach();
	currentView = v;
	if (currentView) currentView->Attach();
}

void Pi::OnChangeDetailLevel()
{
	BaseSphere::OnChangeDetailLevel();
}

void Pi::HandleKeyDown(SDL_Keysym *key)
{
	if (key->sym == SDLK_ESCAPE) {
		if (Pi::game) {
			// only accessible once game started
			HandleEscKey();
		}
		return;
	}
	const bool CTRL = input.KeyState(SDLK_LCTRL) || input.KeyState(SDLK_RCTRL);

	// special keys.
	if (CTRL) {
		switch (key->sym) {
		case SDLK_q: // Quit
			if (Pi::game)
				Pi::EndGame();
			Pi::Quit();
			break;
		case SDLK_PRINTSCREEN: // print
		case SDLK_KP_MULTIPLY: // screen
		{
			char buf[256];
			const time_t t = time(0);
			struct tm *_tm = localtime(&t);
			strftime(buf, sizeof(buf), "screenshot-%Y%m%d-%H%M%S.png", _tm);
			Graphics::ScreendumpState sd;
			Pi::renderer->Screendump(sd);
			write_screenshot(sd, buf);
			break;
		}

		case SDLK_SCROLLLOCK: // toggle video recording
			Pi::isRecordingVideo = !Pi::isRecordingVideo;
			if (Pi::isRecordingVideo) {
				char videoName[256];
				const time_t t = time(0);
				struct tm *_tm = localtime(&t);
				strftime(videoName, sizeof(videoName), "pioneer-%Y%m%d-%H%M%S", _tm);
				const std::string dir = "videos";
				FileSystem::userFiles.MakeDirectory(dir);
				const std::string fname = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot() + "/" + dir, videoName);
				Output("Video Recording started to %s.\n", fname.c_str());
				// start ffmpeg telling it to expect raw rgba 720p-60hz frames
				// -i - tells it to read frames from stdin
				// if given no frame rate (-r 60), it will just use vfr
				char cmd[256] = { 0 };
				snprintf(cmd, sizeof(cmd), "ffmpeg -f rawvideo -pix_fmt rgba -s %dx%d -i - -threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip %s.mp4", config->Int("ScrWidth"), config->Int("ScrHeight"), fname.c_str());

				// open pipe to ffmpeg's stdin in binary write mode
#if defined(_MSC_VER) || defined(__MINGW32__)
				Pi::ffmpegFile = _popen(cmd, "wb");
#else
				Pi::ffmpegFile = _popen(cmd, "w");
#endif
			} else {
				Output("Video Recording ended.\n");
				if (Pi::ffmpegFile != nullptr) {
					_pclose(Pi::ffmpegFile);
					Pi::ffmpegFile = nullptr;
				}
			}
			break;
#if WITH_DEVKEYS
		case SDLK_i: // Toggle Debug info
			Pi::showDebugInfo = !Pi::showDebugInfo;
			break;

#ifdef PIONEER_PROFILER
		case SDLK_p: // alert it that we want to profile
			if (input.KeyState(SDLK_LSHIFT) || input.KeyState(SDLK_RSHIFT))
				Pi::doProfileOne = true;
			else {
				Pi::doProfileSlow = !Pi::doProfileSlow;
				Output("slow frame profiling %s\n", Pi::doProfileSlow ? "enabled" : "disabled");
			}
			break;
#endif

		case SDLK_F12: {
			if (Pi::game) {
				vector3d dir = -Pi::player->GetOrient().VectorZ();
				/* add test object */
				if (input.KeyState(SDLK_RSHIFT)) {
					Missile *missile =
						new Missile(ShipType::MISSILE_GUIDED, Pi::player);
					missile->SetOrient(Pi::player->GetOrient());
					missile->SetFrame(Pi::player->GetFrame());
					missile->SetPosition(Pi::player->GetPosition() + 50.0 * dir);
					missile->SetVelocity(Pi::player->GetVelocity());
					game->GetSpace()->AddBody(missile);
					missile->AIKamikaze(Pi::player->GetCombatTarget());
				} else if (input.KeyState(SDLK_LSHIFT)) {
					SpaceStation *s = static_cast<SpaceStation *>(Pi::player->GetNavTarget());
					if (s) {
						Ship *ship = new Ship(ShipType::POLICE);
						int port = s->GetFreeDockingPort(ship);
						if (port != -1) {
							Output("Putting ship into station\n");
							// Make police ship intent on killing the player
							ship->AIKill(Pi::player);
							ship->SetFrame(Pi::player->GetFrame());
							ship->SetDockedWith(s, port);
							game->GetSpace()->AddBody(ship);
						} else {
							delete ship;
							Output("No docking ports free dude\n");
						}
					} else {
						Output("Select a space station...\n");
					}
				} else {
					Ship *ship = new Ship(ShipType::POLICE);
					if (!input.KeyState(SDLK_LALT)) { //Left ALT = no AI
						if (!input.KeyState(SDLK_LCTRL))
							ship->AIFlyTo(Pi::player); // a less lethal option
						else
							ship->AIKill(Pi::player); // a really lethal option!
					}
					lua_State *l = Lua::manager->GetLuaState();
					pi_lua_import(l, "Equipment");
					LuaTable equip(l, -1);
					LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("laser").Sub("pulsecannon_dual_1mw"));
					LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("misc").Sub("laser_cooling_booster"));
					LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("misc").Sub("atmospheric_shielding"));
					lua_pop(l, 5);
					ship->SetFrame(Pi::player->GetFrame());
					ship->SetPosition(Pi::player->GetPosition() + 100.0 * dir);
					ship->SetVelocity(Pi::player->GetVelocity());
					ship->UpdateEquipStats();
					game->GetSpace()->AddBody(ship);
				}
			}
			break;
		}
#endif /* DEVKEYS */
#if WITH_OBJECTVIEWER
		case SDLK_F10:
			Pi::SetView(Pi::game->GetObjectViewerView());
			break;
#endif
		case SDLK_F11:
			// XXX only works on X11
			//SDL_WM_ToggleFullScreen(Pi::scrSurface);
#if WITH_DEVKEYS
			renderer->ReloadShaders();
#endif
			break;
		case SDLK_F9: // Quicksave
		{
			if (Pi::game) {
				if (Pi::game->IsHyperspace())
					Pi::game->log->Add(Lang::CANT_SAVE_IN_HYPERSPACE);

				else {
					const std::string name = "_quicksave";
					const std::string path = FileSystem::JoinPath(GetSaveDir(), name);
					try {
						Game::SaveGame(name, Pi::game);
						Pi::game->log->Add(Lang::GAME_SAVED_TO + path);
					} catch (CouldNotOpenFileException) {
						Pi::game->log->Add(stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path)));
					} catch (CouldNotWriteToFileException) {
						Pi::game->log->Add(Lang::GAME_SAVE_CANNOT_WRITE);
					}
				}
			}
			break;
		}
		default:
			break; // This does nothing but it stops the compiler warnings
		}
	}
}

void Pi::HandleEscKey()
{
	if (currentView != 0) {
		if (currentView == Pi::game->GetSectorView()) {
			SetView(Pi::game->GetWorldView());
		} else if ((currentView == Pi::game->GetSystemView()) || (currentView == Pi::game->GetSystemInfoView())) {
			SetView(Pi::game->GetSectorView());
		} else {
			UIView *view = dynamic_cast<UIView *>(currentView);
			if (view) {
				// checks the template name
				const char *tname = view->GetTemplateName();
				if (tname) {
					if (!strcmp(tname, "GalacticView")) {
						SetView(Pi::game->GetSectorView());
					} else if (!strcmp(tname, "InfoView") || !strcmp(tname, "StationView")) {
						SetView(Pi::game->GetWorldView());
					}
				}
			}
		}
	}
}

void Pi::HandleEvents()
{
	PROFILE_SCOPED()
	SDL_Event event;

	// XXX for most keypresses SDL will generate KEYUP/KEYDOWN and TEXTINPUT
	// events. keybindings run off KEYUP/KEYDOWN. the console is opened/closed
	// via keybinding. the console TextInput widget uses TEXTINPUT events. thus
	// after switching the console, the stray TEXTINPUT event causes the
	// console key (backtick) to appear in the text entry field. we hack around
	// this by setting this flag if the console was switched. if its set, we
	// swallow the TEXTINPUT event this hack must remain until we have a
	// unified input system
	bool skipTextInput = false;

	Pi::input.mouseMotion[0] = Pi::input.mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			if (Pi::game)
				Pi::EndGame();
			Pi::Quit();
		}

		Pi::pigui->ProcessEvent(&event);

		if (Pi::pigui->WantCaptureMouse()) {
			// don't process mouse event any further, imgui already handled it
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
			case SDL_MOUSEMOTION:
				continue;
			default: break;
			}
		}
		if (Pi::pigui->WantCaptureKeyboard()) {
			// don't process keyboard event any further, imgui already handled it
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
				continue;
			default: break;
			}
		}

		if (skipTextInput && event.type == SDL_TEXTINPUT) {
			skipTextInput = false;
			continue;
		}
		if (ui->DispatchSDLEvent(event))
			continue;

		bool consoleActive = Pi::IsConsoleActive();
		if (!consoleActive) {
			KeyBindings::DispatchSDLEvent(&event);
			if (currentView)
				currentView->HandleSDLEvent(event);
		} else
			KeyBindings::toggleLuaConsole.CheckSDLEventAndDispatch(&event);
		if (consoleActive != Pi::IsConsoleActive()) {
			skipTextInput = true;
			continue;
		}

		if (Pi::IsConsoleActive())
			continue;

		Gui::HandleSDLEvent(&event);
		input.HandleSDLEvent(event);
	}
}

void Pi::TombStoneLoop()
{
	std::unique_ptr<Tombstone> tombstone(new Tombstone(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight()));
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	do {
		Pi::HandleEvents();
		Pi::renderer->SetGrab(false);

		// render the scene
		Pi::BeginRenderTarget();
		Pi::renderer->BeginFrame();
		tombstone->Draw(_time);
		Pi::renderer->EndFrame();
		Gui::Draw();
		Pi::EndRenderTarget();

		Pi::DrawRenderTarget();
		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f * (SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	} while (!((_time > 2.0) && ((input.MouseButtonState(SDL_BUTTON_LEFT)) || input.KeyState(SDLK_SPACE))));
}

void Pi::InitGame()
{
	// this is a bit brittle. skank may be forgotten and survive between
	// games

	input.InitGame();

	if (!config->Int("DisableSound")) AmbientSounds::Init();

	LuaInitGame();
}

static void OnPlayerDockOrUndock()
{
	Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
	Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
}

void Pi::StartGame()
{
	Pi::player->onDock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onUndock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onLanded.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::game->GetCpan()->ShowAll();
	DrawGUI = true;
	SetView(game->GetWorldView());

#ifdef REMOTE_LUA_REPL
#ifndef REMOTE_LUA_REPL_PORT
#define REMOTE_LUA_REPL_PORT 12345
#endif
	luaConsole->OpenTCPDebugConnection(REMOTE_LUA_REPL_PORT);
#endif

	// fire event before the first frame
	LuaEvent::Queue("onGameStart");
	LuaEvent::Emit();
}

void Pi::Start(const SystemPath &startPath)
{
	Pi::bRequestEndGame = false;

	Pi::intro = new Intro(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
	if (startPath != SystemPath(0, 0, 0, 0, 0)) {
		Pi::game = new Game(startPath, 0.0);
	}
	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(51, 51, 51, 255));

	Uint32 last_time = SDL_GetTicks();
	float _time = 0;

	while (!Pi::game) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				Pi::Quit();
			else {
				Pi::pigui->ProcessEvent(&event);

				if (Pi::pigui->WantCaptureMouse()) {
					// don't process mouse event any further, imgui already handled it
					switch (event.type) {
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
					case SDL_MOUSEWHEEL:
					case SDL_MOUSEMOTION:
						continue;
					default: break;
					}
				}
				if (Pi::pigui->WantCaptureKeyboard()) {
					// don't process keyboard event any further, imgui already handled it
					switch (event.type) {
					case SDL_KEYDOWN:
					case SDL_KEYUP:
					case SDL_TEXTINPUT:
						continue;
					default: break;
					}
				}

#if 0 // Moved to Input::HandleSDLEvent, can be deleted when confirmed working \
	// joystick stuff for the options window
				switch (event.type) {
				case SDL_JOYAXISMOTION:
					if (!joysticks[event.jaxis.which].joystick)
						break;
					if (event.jaxis.value == -32768)
						joysticks[event.jaxis.which].axes[event.jaxis.axis] = 1.f;
					else
						joysticks[event.jaxis.which].axes[event.jaxis.axis] = -event.jaxis.value / 32767.f;
					break;
				case SDL_JOYBUTTONUP:
				case SDL_JOYBUTTONDOWN:
					if (!joysticks[event.jaxis.which].joystick)
						break;
					joysticks[event.jbutton.which].buttons[event.jbutton.button] = event.jbutton.state != 0;
					break;
				case SDL_JOYHATMOTION:
					if (!joysticks[event.jaxis.which].joystick)
						break;
					joysticks[event.jhat.which].hats[event.jhat.hat] = event.jhat.value;
					break;
				default: break;
				}
#endif
				ui->DispatchSDLEvent(event);

				input.HandleSDLEvent(event);
			}
			// XXX hack
			// if we hit our exit conditions then ignore further queued events
			// protects against eg double-click during game generation
			if (Pi::game)
				while (SDL_PollEvent(&event)) {
				}
		}

		Pi::BeginRenderTarget();
		Pi::renderer->BeginFrame();
		intro->Draw(_time);
		Pi::renderer->EndFrame();

		PiGui::NewFrame(Pi::renderer->GetSDLWindow());
		DrawPiGui(Pi::frameTime, "MAINMENU");

		Pi::EndRenderTarget();

		// render the rendertarget texture
		Pi::DrawRenderTarget();
		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f * (SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();

#ifdef ENABLE_SERVER_AGENT
		Pi::serverAgent->ProcessResponses();
#endif
	}

	delete Pi::intro;
	Pi::intro = 0;

	InitGame();
	StartGame();
	MainLoop();
}

// request that the game is ended as soon as safely possible
void Pi::RequestEndGame()
{
	Pi::bRequestEndGame = true;
}

void Pi::EndGame()
{
	// always reset this, otherwise we can never play again
	Pi::bRequestEndGame = false;

	Pi::SetMouseGrab(false);

	Pi::musicPlayer.Stop();
	Sound::DestroyAllEvents();

	// final event
	LuaEvent::Queue("onGameEnd");
	LuaEvent::Emit();

	luaTimer->RemoveAll();

	Lua::manager->CollectGarbage();

	if (!config->Int("DisableSound")) AmbientSounds::Uninit();
	Sound::DestroyAllEvents();

	assert(game);
	delete game;
	game = 0;
	player = 0;
}

void Pi::MainLoop()
{
	double time_player_died = 0;
#ifdef MAKING_VIDEO
	Uint32 last_screendump = SDL_GetTicks();
	int dumpnum = 0;
#endif /* MAKING_VIDEO */

#if WITH_DEVKEYS
	Uint32 last_stats = SDL_GetTicks();
	int frame_stat = 0;
	int phys_stat = 0;
	char fps_readout[2048];
	memset(fps_readout, 0, sizeof(fps_readout));
#endif

	int MAX_PHYSICS_TICKS = Pi::config->Int("MaxPhysicsCyclesPerRender");
	if (MAX_PHYSICS_TICKS <= 0)
		MAX_PHYSICS_TICKS = 4;

	double currentTime = 0.001 * double(SDL_GetTicks());
	double accumulator = Pi::game->GetTimeStep();
	Pi::gameTickAlpha = 0;

#ifdef PIONEER_PROFILER
	Profiler::reset();
#endif

	while (Pi::game) {
		PROFILE_SCOPED()

#ifdef ENABLE_SERVER_AGENT
		Pi::serverAgent->ProcessResponses();
#endif

		const Uint32 newTicks = SDL_GetTicks();
		double newTime = 0.001 * double(newTicks);
		Pi::frameTime = newTime - currentTime;
		if (Pi::frameTime > 0.25) Pi::frameTime = 0.25;
		currentTime = newTime;
		accumulator += Pi::frameTime * Pi::game->GetTimeAccelRate();

		const float step = Pi::game->GetTimeStep();
		if (step > 0.0f) {
			PROFILE_SCOPED_RAW("unpaused")
			int phys_ticks = 0;
			while (accumulator >= step) {
				if (++phys_ticks >= MAX_PHYSICS_TICKS) {
					accumulator = 0.0;
					break;
				}
				game->TimeStep(step);
				BaseSphere::UpdateAllBaseSphereDerivatives();

				accumulator -= step;
			}
			// rendering interpolation between frames: don't use when docked
			int pstate = Pi::game->GetPlayer()->GetFlightState();
			if (pstate == Ship::DOCKED || pstate == Ship::DOCKING || pstate == Ship::UNDOCKING)
				Pi::gameTickAlpha = 1.0;
			else
				Pi::gameTickAlpha = accumulator / step;

#if WITH_DEVKEYS
			phys_stat += phys_ticks;
#endif
		} else {
			// paused
			PROFILE_SCOPED_RAW("paused")
			BaseSphere::UpdateAllBaseSphereDerivatives();
		}
		frame_stat++;

		// did the player die?
		if (Pi::player->IsDead()) {
			if (time_player_died > 0.0) {
				if (Pi::game->GetTime() - time_player_died > 8.0) {
					Pi::SetView(0);
					Pi::TombStoneLoop();
					Pi::EndGame();
					break;
				}
			} else {
				Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
				Pi::game->GetDeathView()->Init();
				Pi::SetView(Pi::game->GetDeathView());
				time_player_died = Pi::game->GetTime();
			}
		}

		Pi::BeginRenderTarget();
		Pi::renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
		Pi::renderer->BeginFrame();
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		/* Calculate position for this rendered frame (interpolated between two physics ticks */
		// XXX should this be here? what is this anyway?
		for (Body *b : game->GetSpace()->GetBodies()) {
			b->UpdateInterpTransform(Pi::GetGameTickAlpha());
		}
		game->GetSpace()->GetRootFrame()->UpdateInterpTransform(Pi::GetGameTickAlpha());

		currentView->Update();
		currentView->Draw3D();

		// hide cursor for ship control. Do this before imgui runs, to prevent the mouse pointer from jumping
		Pi::SetMouseGrab(input.MouseButtonState(SDL_BUTTON_RIGHT) | input.MouseButtonState(SDL_BUTTON_MIDDLE));

		// XXX HandleEvents at the moment must be after view->Draw3D and before
		// Gui::Draw so that labels drawn to screen can have mouse events correctly
		// detected. Gui::Draw wipes memory of label positions.
		Pi::HandleEvents();

#ifdef REMOTE_LUA_REPL
		Pi::luaConsole->HandleTCPDebugConnections();
#endif

		if (Pi::bRequestEndGame) {
			Pi::EndGame();
		}

		Pi::renderer->EndFrame();

		Pi::renderer->ClearDepthBuffer();
		if (DrawGUI) {
			Gui::Draw();
		} else if (game && game->IsNormalSpace()) {
			if (config->Int("DisableScreenshotInfo") == 0) {
				const RefCountedPtr<StarSystem> sys = game->GetSpace()->GetStarSystem();
				const SystemPath sp = sys->GetPath();
				std::ostringstream pathStr;

				// fill in pathStr from sp values and sys->GetName()
				static const std::string comma(", ");
				pathStr << Pi::player->GetFrame()->GetLabel() << comma << sys->GetName() << " (" << sp.sectorX << comma << sp.sectorY << comma << sp.sectorZ << ")";

				// display pathStr
				Gui::Screen::EnterOrtho();
				Gui::Screen::PushFont("ConsoleFont");
				static RefCountedPtr<Graphics::VertexBuffer> s_pathvb;
				Gui::Screen::RenderStringBuffer(s_pathvb, pathStr.str(), 0, 0);
				Gui::Screen::PopFont();
				Gui::Screen::LeaveOrtho();
			}
		}

		// XXX don't draw the UI during death obviously a hack, and still
		// wrong, because we shouldn't this when the HUD is disabled, but
		// probably sure draw it if they switch to eg infoview while the HUD is
		// disabled so we need much smarter control for all this rubbish
		if ((!Pi::game || Pi::GetView() != Pi::game->GetDeathView()) && DrawGUI) {
			Pi::ui->Update();
			Pi::ui->Draw();
		}

		Pi::EndRenderTarget();
		Pi::DrawRenderTarget();
		bool endCameraFrame = false;
		if (Pi::game && !Pi::player->IsDead()) {
			if (Pi::GetView() == Pi::game->GetWorldView()) {
				Pi::game->GetWorldView()->BeginCameraFrame();
				endCameraFrame = true;
			}
			PiGui::NewFrame(Pi::renderer->GetSDLWindow());
			DrawPiGui(Pi::frameTime, "GAME");
			if (endCameraFrame) {
				Pi::game->GetWorldView()->EndCameraFrame();
			}
		}

#if WITH_DEVKEYS
		// NB: this needs to be rendered last so that it appears over all other game elements
		//	preferrably like this where it is just before the buffer swap
		if (Pi::showDebugInfo) {
			Gui::Screen::EnterOrtho();
			Gui::Screen::PushFont("ConsoleFont");
			static RefCountedPtr<Graphics::VertexBuffer> s_debugInfovb;
			Gui::Screen::RenderStringBuffer(s_debugInfovb, fps_readout, 0, 0);
			Gui::Screen::PopFont();
			Gui::Screen::LeaveOrtho();
		}
#endif

		Pi::renderer->SwapBuffers();

		// game exit will have cleared Pi::game. we can't continue.
		if (!Pi::game)
			return;

		if (Pi::game->UpdateTimeAccel())
			accumulator = 0; // fix for huge pauses 10000x -> 1x

		if (!Pi::player->IsDead()) {
			// XXX should this really be limited to while the player is alive?
			// this is something we need not do every turn...
			if (!config->Int("DisableSound")) AmbientSounds::Update();
		}
		Pi::game->GetCpan()->Update();
		musicPlayer.Update();

		syncJobQueue->RunJobs(SYNC_JOBS_PER_LOOP);
		asyncJobQueue->FinishJobs();
		syncJobQueue->FinishJobs();

#if WITH_DEVKEYS
		if (Pi::showDebugInfo && SDL_GetTicks() - last_stats > 1000) {
			size_t lua_mem = Lua::manager->GetMemoryUsage();
			int lua_memB = int(lua_mem & ((1u << 10) - 1));
			int lua_memKB = int(lua_mem >> 10) % 1024;
			int lua_memMB = int(lua_mem >> 20);
			const Graphics::Stats::TFrameData &stats = Pi::renderer->GetStats().FrameStatsPrevious();
			const Uint32 numDrawCalls = stats.m_stats[Graphics::Stats::STAT_DRAWCALL];
			const Uint32 numBuffersCreated = stats.m_stats[Graphics::Stats::STAT_CREATE_BUFFER];
			const Uint32 numDrawTris = stats.m_stats[Graphics::Stats::STAT_DRAWTRIS];
			const Uint32 numDrawPointSprites = stats.m_stats[Graphics::Stats::STAT_DRAWPOINTSPRITES];
			const Uint32 numDrawBuildings = stats.m_stats[Graphics::Stats::STAT_BUILDINGS];
			const Uint32 numDrawCities = stats.m_stats[Graphics::Stats::STAT_CITIES];
			const Uint32 numDrawGroundStations = stats.m_stats[Graphics::Stats::STAT_GROUNDSTATIONS];
			const Uint32 numDrawSpaceStations = stats.m_stats[Graphics::Stats::STAT_SPACESTATIONS];
			const Uint32 numDrawAtmospheres = stats.m_stats[Graphics::Stats::STAT_ATMOSPHERES];
			const Uint32 numDrawPatches = stats.m_stats[Graphics::Stats::STAT_PATCHES];
			const Uint32 numDrawPlanets = stats.m_stats[Graphics::Stats::STAT_PLANETS];
			const Uint32 numDrawGasGiants = stats.m_stats[Graphics::Stats::STAT_GASGIANTS];
			const Uint32 numDrawStars = stats.m_stats[Graphics::Stats::STAT_STARS];
			const Uint32 numDrawShips = stats.m_stats[Graphics::Stats::STAT_SHIPS];
			const Uint32 numDrawBillBoards = stats.m_stats[Graphics::Stats::STAT_BILLBOARD];
			snprintf(
				fps_readout, sizeof(fps_readout),
				"%d fps (%.1f ms/f), %d phys updates, %d triangles, %.3f M tris/sec, %d glyphs/sec, %d patches/frame\n"
				"Lua mem usage: %d MB + %d KB + %d bytes (stack top: %d)\n\n"
				"Draw Calls (%u), of which were:\n Tris (%u)\n Point Sprites (%u)\n Billboards (%u)\n"
				"Buildings (%u), Cities (%u), GroundStations (%u), SpaceStations (%u), Atmospheres (%u)\n"
				"Patches (%u), Planets (%u), GasGiants (%u), Stars (%u), Ships (%u)\n"
				"Buffers Created(%u)\n",
				frame_stat, (1000.0 / frame_stat), phys_stat, Pi::statSceneTris, Pi::statSceneTris * frame_stat * 1e-6,
				Text::TextureFont::GetGlyphCount(), Pi::statNumPatches,
				lua_memMB, lua_memKB, lua_memB, lua_gettop(Lua::manager->GetLuaState()),
				numDrawCalls, numDrawTris, numDrawPointSprites, numDrawBillBoards,
				numDrawBuildings, numDrawCities, numDrawGroundStations, numDrawSpaceStations, numDrawAtmospheres,
				numDrawPatches, numDrawPlanets, numDrawGasGiants, numDrawStars, numDrawShips, numBuffersCreated);
			frame_stat = 0;
			phys_stat = 0;
			Text::TextureFont::ClearGlyphCount();
			if (SDL_GetTicks() - last_stats > 1200)
				last_stats = SDL_GetTicks();
			else
				last_stats += 1000;
		}
		Pi::statSceneTris = 0;
		Pi::statNumPatches = 0;

#ifdef PIONEER_PROFILER
		const Uint32 profTicks = SDL_GetTicks();
		if (Pi::doProfileOne || (Pi::doProfileSlow && (profTicks - newTicks) > 100)) { // slow: < ~10fps
			Output("dumping profile data\n");
			Profiler::dumphtml(profilerPath.c_str());
			Pi::doProfileOne = false;
		}
#endif

#endif

#ifdef MAKING_VIDEO
		if (SDL_GetTicks() - last_screendump > 50) {
			last_screendump = SDL_GetTicks();
			std::string fname = stringf(Lang::SCREENSHOT_FILENAME_TEMPLATE, formatarg("index", dumpnum++));
			Screendump(fname.c_str(), Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
		}
#endif /* MAKING_VIDEO */

		if (isRecordingVideo && (Pi::ffmpegFile != nullptr)) {
			Graphics::ScreendumpState sd;
			Pi::renderer->FrameGrab(sd);
			fwrite(sd.pixels.get(), sizeof(uint32_t) * Pi::renderer->GetWindowWidth() * Pi::renderer->GetWindowHeight(), 1, Pi::ffmpegFile);
		}

#ifdef PIONEER_PROFILER
		Profiler::reset();
#endif
	}
}

float Pi::GetMoveSpeedShiftModifier()
{
	// Suggestion: make x1000 speed on pressing both keys?
	if (input.KeyState(SDLK_LSHIFT)) return 100.f;
	if (input.KeyState(SDLK_RSHIFT)) return 10.f;
	return 1;
}

void Pi::SetMouseGrab(bool on)
{
	if (!doingMouseGrab && on) {
		Pi::renderer->SetGrab(true);
		Pi::ui->SetMousePointerEnabled(false);
		doingMouseGrab = true;
	} else if (doingMouseGrab && !on) {
		Pi::renderer->SetGrab(false);
		Pi::ui->SetMousePointerEnabled(true);
		doingMouseGrab = false;
	}
}

void Pi::DrawPiGui(double delta, std::string handler)
{
//  #define PROFILE_LUA_TIME 1
#ifdef PROFILE_LUA_TIME
	auto before = clock();
#endif
	if (!IsConsoleActive())
		Pi::pigui->Render(delta, handler);
#ifdef PROFILE_LUA_TIME
	auto after = clock();
	Output("Lua PiGUI took %f\n", double(after - before) / CLOCKS_PER_SEC);
#endif
	PiGui::RenderImGui();
}
