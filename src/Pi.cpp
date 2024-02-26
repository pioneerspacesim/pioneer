// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "buildopts.h"

#include "Pi.h"

#include "Body.h"
#include "BodyComponent.h"

#include "BaseSphere.h"
#include "Beam.h"
#include "CityOnPlanet.h"
#include "DeathView.h"
#include "EnumStrings.h"
#include "FaceParts.h"
#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameLog.h"
#include "GameSaveError.h"
#include "Input.h"
#include "Intro.h"
#include "Lang.h"
#include "Missile.h"
#include "ModManager.h"
#include "ModelCache.h"
#include "NavLights.h"
#include "Player.h"
#include "PngWriter.h"
#include "Projectile.h"
#include "SectorView.h"
#include "Sfx.h"
#include "Shields.h"
#include "ShipType.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Star.h"
#include "StringF.h"
#include "Tombstone.h"
#include "TransferPlanner.h"
#include "WorldView.h"

#if WITH_OBJECTVIEWER
#include "ObjectViewerView.h"
#endif

#include "galaxy/GalaxyGenerator.h"

#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/opengl/RendererGL.h"

#include "core/GuiApplication.h"
#include "core/Log.h"
#include "core/OS.h"

#include "lua/Lua.h"
#include "lua/LuaConsole.h"
#include "lua/LuaEvent.h"
#include "lua/LuaTimer.h"

#include "pigui/LuaPiGui.h"
#include "pigui/PerfInfo.h"
#include "pigui/PiGui.h"

#include "sound/AmbientSounds.h"
#include "sound/Sound.h"
#include "sound/SoundMusic.h"

#include "profiler/Profiler.h"
#include "versioningInfo.h"

#include <SDL.h>

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

/*
===============================================================================
	DEFINITIONS
===============================================================================
*/

float Pi::gameTickAlpha;
LuaSerializer *Pi::luaSerializer;
LuaTimer *Pi::luaTimer;
LuaNameGen *Pi::luaNameGen;
#ifdef ENABLE_SERVER_AGENT
ServerAgent *Pi::serverAgent;
#endif
Input::Manager *Pi::input;
Player *Pi::player;
View *Pi::currentView;
TransferPlanner *Pi::planner;
std::unique_ptr<LuaConsole> Pi::luaConsole;
Game *Pi::game;
Random Pi::rng;
float Pi::frameTime;
bool Pi::showDebugInfo = false;
int Pi::statSceneTris = 0;
int Pi::statNumPatches = 0;
GameConfig *Pi::config;
DetailLevel Pi::detail;
bool Pi::navTunnelDisplayed = false;
bool Pi::speedLinesDisplayed = false;
bool Pi::hudTrailsDisplayed = false;
bool Pi::bRefreshBackgroundStars = true;
float Pi::amountOfBackgroundStarsDisplayed = 1.0f;
float Pi::starFieldStarSizeFactor = 1.0f;
bool Pi::DrawGUI = true;
Graphics::Renderer *Pi::renderer;
PiGui::Instance *Pi::pigui = nullptr;
ModelCache *Pi::modelCache;
Intro *Pi::intro;
SDLGraphics *Pi::sdl;
bool Pi::isRecordingVideo = false;
FILE *Pi::ffmpegFile = nullptr;

Pi::App *Pi::m_instance = nullptr;

Sound::MusicPlayer Pi::musicPlayer;

class StartupScreen : public Application::Lifecycle {
public:
	StartupScreen() :
		Lifecycle(true)
	{
	}

	std::unique_ptr<JobSet> asyncStartupQueue;
	std::unique_ptr<JobSet> currentStepQueue;

protected:
	struct LoadStep {
		// TODO: use a lighter-weight wrapper over lambdas instead of std::function
		std::function<void()> fn;
		std::string name;
	};

	std::vector<LoadStep> m_loaders;
	size_t m_currentLoader = 0;
	bool m_hasQueuedJobs = 0;

	template <typename T>
	void AddStep(std::string name, T fn)
	{
		m_loaders.push_back(LoadStep{ fn, name });
	}

	Profiler::Clock m_loadTimer;
	Profiler::Clock m_stepTimer;

	void Start() override;
	void Update(float) override;
	void End() override;

	void RunNewLoader();
	void FinishLoadStep();
	float GetProgress() { return (m_currentLoader) / float(m_loaders.size()); }
};

// FIXME: this is a hack, this class should have its lifecycle managed elsewhere
// Ideally an application framework class handles this (as well as the rest of the main loop)
// but for now this is the best we have.
std::unique_ptr<PiGui::PerfInfo> perfInfoDisplay;

class MainMenu : public Application::Lifecycle {
public:
	void SetStartPath(const SystemPath &path)
	{
		m_startPath = path;
		m_skipMenu = true;
	}

protected:
	std::unique_ptr<Intro> m_intro;

	void Start() override;
	void Update(float) override;
	void End() override;

	bool m_skipMenu;
	SystemPath m_startPath;
};

class GameLoop : public Application::Lifecycle {
protected:
	void Start() override;
	void Update(float) override;
	void End() override;

	void InitGame();
	void EndGame();

	double time_player_died;

	// Used to measure frame and physics performance timing info
	// with no portable way to map cycles -> ns, Profiler::Timer is useless for taking measurements
	// Use Profiler::Clock which is backed by std::chrono::steady_clock (== high_resolution_clock for 99% of uses)
	Profiler::Clock perfTimer;

	float frame_time_real; // higher resolution than SDL's 1ms, for detailed frame info
	float phys_time;
	float pigui_time;

	int frame_stat;
	int phys_stat;

	uint32_t profile_startup_ms;
	uint32_t startup_ticks;

	int MAX_PHYSICS_TICKS;
	double accumulator;

	Uint32 last_stats = SDL_GetTicks();
};

class TombstoneLoop : public Application::Lifecycle {
protected:
	void Start() override;
	void Update(float) override;

	double startTime = 0.0;
	double accumTime = 0.0;
	std::unique_ptr<Tombstone> tombstone;
};

/*
===============================================================================
	INITIALIZATION
===============================================================================
*/

// TODO: refine this interface
// We don't use options for anything but the config object,
// and instead of passing no_gui, we should instead use a different application
// object devoted to whatever headless work we intend to do
void Pi::Init(const std::map<std::string, std::string> &options, bool no_gui)
{
	PROFILE_SCOPED();
	Pi::config = new GameConfig(options);
	m_instance = new Pi::App();

	GetApp()->m_loader.Reset(new StartupScreen());
	GetApp()->m_mainMenu.Reset(new MainMenu());
	GetApp()->m_gameLoop.Reset(new GameLoop());

	m_instance->m_noGui = no_gui;

	m_instance->Startup();
}

void Pi::App::SetStartPath(const SystemPath &startPath)
{
	static_cast<MainMenu *>(m_mainMenu.Get())->SetStartPath(startPath);
}

void TestGPUJobsSupport()
{
	PROFILE_SCOPED()

	if (!Pi::config->Int("EnableGPUJobs"))
		return;

	Uint32 octaves = 8;
	Graphics::MaterialDescriptor desc;
	desc.quality = Graphics::MaterialQuality::HAS_OCTAVES | (octaves << 16);
	desc.textures = 3;

	Graphics::RenderStateDesc rsd;
	rsd.depthTest = false;
	rsd.depthWrite = false;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.primitiveType = Graphics::TRIANGLE_STRIP;

	std::unique_ptr<Graphics::Material> mat(Pi::renderer->CreateMaterial("gen_gas_giant_colour", desc, rsd));

	// failed - retry
	// reduce the number of octaves
	if (!mat->IsProgramLoaded()) {
		octaves = 5;
		desc.quality = Graphics::MaterialQuality::HAS_OCTAVES | (octaves << 16);
		mat.reset(Pi::renderer->CreateMaterial("gen_gas_giant_colour", desc, rsd));

		// if this works correctly with fewer octaves, enable the config flag.
		if (mat->IsProgramLoaded())
			Pi::config->SetInt("AMD_MESA_HACKS", 1);
	}

	if (!mat->IsProgramLoaded()) {
		Log::Warning("EnableGPUJobs is DISABLED: shader compilation produced errors. Check output.txt and opengl.txt.\n");
		Pi::config->SetInt("EnableGPUJobs", 0);
		Pi::config->Save();
	}
}

void Pi::App::OnStartup()
{
	PROFILE_SCOPED()
	Profiler::Clock startupTimer;
	startupTimer.Start();

	SetupProfiler(Pi::config);

	Log::GetLog()->SetLogFile("output.txt");

	if (config->Int("LogVerbose", 0))
		Log::GetLog()->SetFileSeverity(Log::Severity::Verbose);

	if (config->Int("LogVerbose", 0) > 1)
		Log::GetLog()->SetSeverity(Log::Severity::Verbose);

	std::string version(PIONEER_VERSION);
	if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
	const char *platformName = SDL_GetPlatform();
	if (platformName)
		Output("ver %s on: %s\n\n", version.c_str(), platformName);
	else
		Output("ver %s but could not detect platform name.\n\n", version.c_str());

	Output("%s\n", OS::GetOSInfoString().c_str());

	ModManager::Init();
	ModManager::LoadMods(config);

	Lang::Resource &res(Lang::GetResource("core", config->String("Lang")));
	Lang::MakeCore(res);

	// FIXME: move these out of the Pi namespace
	// TODO: add a better configuration interface for this kind of thing
	Pi::SetAmountBackgroundStars(config->Float("AmountOfBackgroundStars"));
	Pi::SetStarFieldStarSizeFactor(config->Float("StarFieldStarSizeFactor"));
	Pi::detail.planets = config->Int("DetailPlanets");
	Pi::detail.cities = config->Int("DetailCities");

	Graphics::RendererOGL::RegisterRenderer();
	Pi::renderer = StartupRenderer(Pi::config, false, config->Int("DebugWindowResize"));

	Pi::rng.IncRefCount(); // so nothing tries to free it
	Pi::rng.seed(time(0));

	Pi::input = StartupInput(config);
	Pi::input->onKeyPress.connect(sigc::ptr_fun(&Pi::HandleKeyDown));

	// Register all C++-side input bindings.
	// TODO: handle registering Lua input bindings in the startup phase.
	for (auto &registrar : Input::GetBindingRegistration()) {
		registrar(Pi::input);
	}

	Pi::pigui = StartupPiGui();

	// FIXME: move these into the appropriate class!
	navTunnelDisplayed = (config->Int("DisplayNavTunnel")) ? true : false;
	speedLinesDisplayed = (config->Int("SpeedLines")) ? true : false;
	hudTrailsDisplayed = (config->Int("HudTrails")) ? true : false;

	TestGPUJobsSupport();

	EnumStrings::Init();

	// Can be initialized directly after FileSystem::Init, but put it here for convenience
	GalacticEconomy::Init();

	BodyComponentDB::Init();

	Profiler::Clock threadTimer;
	threadTimer.Start();

	// get threads up
	Uint32 numThreads = config->Int("WorkerThreads");
	numThreads = numThreads ? numThreads : std::max(OS::GetNumCores() - 1, 1U);
	GetTaskGraph()->SetWorkerThreads(numThreads);

	threadTimer.Stop();
	Output("started %d worker threads in %.2fms\n", numThreads, threadTimer.milliseconds());

	QueueLifecycle(m_loader);

	// Don't start the main menu if we don't have a GUI
	if (!m_noGui)
		QueueLifecycle(m_mainMenu);

	startupTimer.Stop();
	Output("\n\nEngine startup took %.2fms\n", startupTimer.milliseconds());
}

/*
===============================================================================
	DEINITIALIZATION
===============================================================================
*/

// Immediately destroy everything and end the game.
void Pi::App::OnShutdown()
{
	PROFILE_SCOPED()
	Output("Pi shutting down.\n");

	// This function should only be called at the very end of the shutdown procedure.
	assert(Pi::game == nullptr);
	if (Pi::ffmpegFile != nullptr) {
		_pclose(Pi::ffmpegFile);
	}

	perfInfoDisplay.reset();

	// TODO: connect initializers and deinitializers in a single Module interface
	// Will need to think about dependency injection for e.g. modules which need a
	// reference to the renderer
	Projectile::FreeModel();
	Beam::FreeModel();
	delete Pi::intro;
	Pi::luaConsole.reset();
	NavLights::Uninit();
	Shields::Uninit();
	SfxManager::Uninit();
	Sound::Uninit();
	CityOnPlanet::Uninit();
	BaseSphere::Uninit();
	FaceParts::Uninit();
	Graphics::Uninit();

	PiGui::Lua::Uninit();
	ShutdownPiGui();
	Pi::pigui = nullptr;
	Lua::UninitModules();
	Lua::Uninit();

	delete Pi::modelCache;

	GalaxyGenerator::Uninit();

	BodyComponentDB::Uninit();

	ModManager::Uninit();

	ShutdownRenderer();
	Pi::renderer = nullptr;

	ShutdownInput();
	Pi::input = nullptr;

	delete Pi::config;
	delete Pi::planner;
}

void Pi::Uninit()
{
	m_instance->Shutdown();

	SDL_Quit();
	delete Pi::m_instance;
	exit(0);
}

/*
===============================================================================
	LOADING
===============================================================================
*/

JobSet *Pi::App::GetAsyncStartupQueue() const
{
	return static_cast<StartupScreen *>(m_loader.Get())->asyncStartupQueue.get();
}

JobSet *Pi::App::GetCurrentLoadStepQueue() const
{
	return static_cast<StartupScreen *>(m_loader.Get())->currentStepQueue.get();
}

// TODO: investigate constructing a DAG out of Init functions and dependencies
void StartupScreen::Start()
{
	PROFILE_SCOPED()

	asyncStartupQueue.reset(new JobSet(Pi::GetAsyncJobQueue()));
	currentStepQueue.reset(new JobSet(Pi::GetAsyncJobQueue()));

	Output("StartupScreen::Start()\n");
	m_loadTimer.Reset();
	m_loadTimer.Start();

	Output("ShipType::Init()\n");
	// XXX early, Lua init needs it
	ShipType::Init();

	// XXX UI requires Lua  but Pi::ui must exist before we start loading
	// templates. so now we have crap everywhere :/
	Output("Lua::Init()\n");
	Lua::Init();

	// TODO: Get the lua state responsible for drawing the init progress up as fast as possible
	// Investigate using a pigui-only Lua state that we can initialize without depending on
	// normal init flow, or drawing the init screen in C++ instead?
	// Loads just the PiGui class and PiGui-related modules
	PiGui::Lua::Init();
	// FIXME: this just exists to load the theme out-of-order from Lua::InitModules. Needs a better solution
	PiGui::LoadThemeFromDisk("default");
	PiGui::LoadTheme(ImGui::GetStyle(), "default");

	// Don't render the first frame, just make sure all of our fonts are loaded
	Pi::pigui->NewFrame();
	PiGui::RunHandler(0.01, "init");
	Pi::pigui->EndFrame();

	AddStep("Sound::Init", []() {
		if (Pi::GetApp()->HeadlessMode() || Pi::config->Int("DisableSound"))
			return;

		Sound::Init();
		Sound::SetMasterVolume(Pi::config->Float("MasterVolume"));
		Sound::SetSfxVolume(Pi::config->Float("SfxVolume"));
		Pi::GetMusicPlayer().SetVolume(Pi::config->Float("MusicVolume"));

		Sound::Pause(0);
		if (Pi::config->Int("MasterMuted")) Sound::Pause(1);
		if (Pi::config->Int("SfxMuted")) Sound::SetSfxVolume(0.f);
		if (Pi::config->Int("MusicMuted")) Pi::GetMusicPlayer().SetEnabled(false);
	});

#ifdef ENABLE_SERVER_AGENT
	AddStep("Initialize ServerAgent", []() {
		Pi::serverAgent = 0;
		if (Pi::config->Int("EnableServerAgent")) {
			const std::string endpoint(Pi::config->String("ServerEndpoint"));
			if (endpoint.size() > 0) {
				Output("Server agent enabled, endpoint: %s\n", endpoint.c_str());
				Pi::serverAgent = new HTTPServerAgent(endpoint);
			}
		}
		if (!Pi::serverAgent) {
			Output("Server agent disabled\n");
			Pi::serverAgent = new NullServerAgent();
		}
	});
#endif

	// TODO: expose the AddStep interface so Lua::InitModules can granularize its registration
	AddStep("Lua::InitModules()", &Lua::InitModules);

	AddStep("GalaxyGenerator::Init()", []() {
		if (Pi::config->HasEntry("GalaxyGenerator"))
			GalaxyGenerator::Init(Pi::config->String("GalaxyGenerator"),
				Pi::config->Int("GalaxyGeneratorVersion", GalaxyGenerator::LAST_VERSION));
		else
			GalaxyGenerator::Init();
	});

	AddStep("FaceParts::Init()", &FaceParts::Init);

	AddStep("new ModelCache", []() {
		Pi::modelCache = new ModelCache(Pi::renderer);
	});

	AddStep("Shields::Init", []() {
		Shields::Init(Pi::renderer);
	});

	AddStep("BaseSphere::Init", &BaseSphere::Init);

	AddStep("CityOnPlanet::Init", &CityOnPlanet::Init);

	AddStep("SpaceStation::Init", &SpaceStation::Init);

	AddStep("NavLights::Init", []() {
		NavLights::Init(Pi::renderer);
	});

	AddStep("Sfx::Init", []() {
		SfxManager::Init(Pi::renderer);
	});

	AddStep("PostLoad", []() {
		Pi::luaConsole.reset(new LuaConsole());
		Pi::luaConsole->SetupBindings();

		Pi::planner = new TransferPlanner();

		perfInfoDisplay.reset(new PiGui::PerfInfo());
	});
}

void StartupScreen::Update(float deltaTime)
{
	PROFILE_SCOPED()

	// if we have queued jobs from the current loader step and they're all done, finish up
	if (m_hasQueuedJobs && currentStepQueue->IsEmpty())
		FinishLoadStep();

	if (!m_hasQueuedJobs) {
		if (m_currentLoader < m_loaders.size())
			RunNewLoader();
		// finish loading once all steps are complete and there's nothing left in the queue.
		else if (asyncStartupQueue->IsEmpty())
			return RequestEndLifecycle();
	}

	Pi::pigui->NewFrame();
	PiGui::EmitEvents();
	PiGui::RunHandler(GetProgress(), "init");
	Pi::pigui->Render();
}

void StartupScreen::RunNewLoader()
{
	// don't increment the current loader count and run the loader if async jobs haven't finished yet
	LoadStep &loader = m_loaders[m_currentLoader];
	Output("Loading [%02.f%%]: %s started\n", GetProgress() * 100., loader.name.c_str());

	m_stepTimer.SoftReset();
	loader.fn();

	// if we haven't queued any jobs, just finish this step and skip to the next one
	m_hasQueuedJobs = !currentStepQueue->IsEmpty();
	if (currentStepQueue->IsEmpty())
		FinishLoadStep();
}

void StartupScreen::FinishLoadStep()
{
	m_hasQueuedJobs = false;
	m_stepTimer.Stop();
	Output("Loading [%02.f%%]: %s took %.2fms\n", GetProgress() * 100.,
		m_loaders[m_currentLoader].name.c_str(), m_stepTimer.milliseconds());
	m_currentLoader++;
}

void StartupScreen::End()
{
	OS::NotifyLoadEnd();
	Pi::GetApp()->RequestProfileFrame();

	m_loadTimer.Stop();
	Output("\n\nPioneer loading took %.2fms\n", m_loadTimer.milliseconds());
}

/*
===============================================================================
	MAIN MENU
===============================================================================
*/

void MainMenu::Start()
{
	// TODO: just calculate this at draw time inside Intro
	Pi::intro = new Intro(Pi::renderer, Pi::renderer->GetWindowWidth(), Pi::renderer->GetWindowHeight());
	if (m_skipMenu) {
		Output("Loading new game immediately!\n");
		Pi::StartGame(new Game(m_startPath, 0.0));
		m_skipMenu = false; // Show the main menu once we're done here.
	}

	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(51, 51, 51, 255));

	perfInfoDisplay->ClearCounter(PiGui::PerfInfo::COUNTER_PHYS);
	perfInfoDisplay->ClearCounter(PiGui::PerfInfo::COUNTER_PIGUI);
}

void MainMenu::Update(float deltaTime)
{
	Pi::GetApp()->HandleEvents();

	if (Pi::MustRefreshBackgroundClearFlag())
		Pi::intro->RefreshBackground(Pi::renderer);

	Pi::intro->Draw(deltaTime);

	Pi::pigui->NewFrame();
	PiGui::EmitEvents();
	PiGui::RunHandler(deltaTime, "mainMenu");

	perfInfoDisplay->Update(deltaTime);

	if (Pi::showDebugInfo) {
		Pi::pigui->SetDebugStyle();
		perfInfoDisplay->Draw();
		Pi::pigui->SetNormalStyle();
	}

	Pi::renderer->ClearDepthBuffer();
	Pi::pigui->Render();

	if (Pi::game) {
		RequestEndLifecycle();
	}

#ifdef ENABLE_SERVER_AGENT
	Pi::serverAgent->ProcessResponses();
#endif
}

void MainMenu::End()
{
	delete Pi::intro;
	Pi::intro = nullptr;
}

// Not much better than setting Pi::game in a Lua function,
// but it works for now.
void Pi::StartGame(Game *game)
{
	// FIXME: Game.cpp sets Pi::game because some other things depend on that variable
	// if (Pi::game != nullptr)
	// 	Error("Attempt to start a new game while one is already running!\n");

	Pi::game = game;

	Pi::GetApp()->GetActiveLifecycle()->RequestEndLifecycle();
	Pi::GetApp()->QueueLifecycle(Pi::GetApp()->m_gameLoop);
}

/*
===============================================================================
	EVENT HANDLING
===============================================================================
*/

void Pi::HandleKeyDown(SDL_Keysym *key)
{
	const bool CTRL = input->KeyState(SDLK_LCTRL) || input->KeyState(SDLK_RCTRL);
	if (!CTRL) {
		return;
	}

	// special keys: CTRL+[KEY].
	switch (key->sym) {
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

#if 0 // FIXME: find a better home / interface for video recording
	case SDLK_SCROLLLOCK: // toggle video recording
		SetVideoRecording(!Pi::isRecordingVideo);
		break;
#endif

	case SDLK_i: // Toggle Debug info
		Pi::showDebugInfo = !Pi::showDebugInfo;
		break;

#if WITH_DEVKEYS
#ifdef PIONEER_PROFILER
	case SDLK_p: // alert it that we want to profile
		if (input->KeyState(SDLK_LSHIFT) || input->KeyState(SDLK_RSHIFT))
			Pi::GetApp()->RequestProfileFrame();
		break;
#endif

	case SDLK_F11: // Reload shaders
		renderer->ReloadShaders();
		break;
#endif /* DEVKEYS */

#if WITH_OBJECTVIEWER
	case SDLK_F10: {
		if (!Pi::game)
			break;

		if (Pi::GetView() == Pi::game->GetObjectViewerView())
			Pi::SetView(Pi::game->GetWorldView());
		else if (Pi::player->GetNavTarget())
			Pi::SetView(Pi::game->GetObjectViewerView());
		break;
	}
#endif

	case SDLK_F9: // Quicksave
	{
		if (!Pi::game)
			break;

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
		break;
	}
	default:
		break; // This does nothing but it stops the compiler warnings
	}
}

void Pi::App::HandleRequests()
{
	for (auto request : internalRequests) {
		switch (request) {
		case InternalRequests::END_GAME: {
			if (!Pi::game)
				break;

			m_gameLoop->RequestEndLifecycle();
		} break;
		case InternalRequests::QUIT_GAME: {
			GetActiveLifecycle()->RequestEndLifecycle();
			RequestQuit();
		} break;
		case InternalRequests::DETAIL_LEVEL_CHANGED: {
			if (!Pi::game)
				break;

			BaseSphere::OnChangeDetailLevel();
		} break;
		default:
			Output("Pi::HandleRequests, unhandled request type %d processed.\n", int(request));
			break;
		}
	}
	internalRequests.clear();
}

/*
===============================================================================
	GAME LOOP
===============================================================================
*/

void Pi::App::PreUpdate()
{
	PROFILE_SCOPED()
	Pi::frameTime = DeltaTime();
}

void Pi::App::PostUpdate()
{
	PROFILE_SCOPED()

	HandleRequests();
}

// FIXME: delete/move this function out of Pi.cpp
static void OnPlayerDockOrUndock();

void GameLoop::Start()
{
	PROFILE_SCOPED()

	// NOTE: this is here because Clang 15+ and GCC 13+ ignore fp-model when
	// generating vectorized/optimized code and will happily perform exception-
	// raising operations on the contents of uninitialized or aliased memory
	OS::DisableFPE();

	// this is a bit brittle. skank may be forgotten and survive between
	// games
	Pi::input->InitGame();

	if (!Pi::config->Int("DisableSound")) AmbientSounds::Init();

	LuaEvent::Clear();

	Pi::player->onDock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onUndock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onLanded.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::DrawGUI = true;
	Pi::SetView(Pi::game->GetWorldView());

#ifdef REMOTE_LUA_REPL
#ifndef REMOTE_LUA_REPL_PORT
#define REMOTE_LUA_REPL_PORT 12345
#endif
	Pi::luaConsole->OpenTCPDebugConnection(REMOTE_LUA_REPL_PORT);
#endif

	// fire event before the first frame
	LuaEvent::Queue("onGameStart");
	LuaEvent::Emit();

	frame_stat = 0;
	phys_stat = 0;
	accumulator = Pi::game->GetTimeStep();
	time_player_died = 0.0;

	MAX_PHYSICS_TICKS = Pi::config->Int("MaxPhysicsCyclesPerRender");
	if (MAX_PHYSICS_TICKS <= 0)
		MAX_PHYSICS_TICKS = 4;

	Pi::SetGameTickAlpha(0);
	// If we have a tombstone loop, we will SetNextLifecycle() so it runs before
	// we jump back to the main menu
	Pi::GetApp()->QueueLifecycle(Pi::GetApp()->m_mainMenu);

	profile_startup_ms = Clamp(Pi::config->Int("ProfileStartupMs", 0), 0, 10000);
	startup_ticks = SDL_GetTicks();
	SetProfilerAccumulate(profile_startup_ms > 0);
}

void GameLoop::Update(float deltaTime)
{
	PROFILE_SCOPED()
	perfTimer.SoftReset();			   // Reset() + Start()
	frame_time_real = deltaTime * 1e3; // convert to ms
	frame_stat++;

	// Read events into internal structures and into imgui structures,
	// dispatch will be performed after the imgui frame, so that imgui can add
	// something based on clicks on widgets
	Pi::GetApp()->PollEvents();

#ifdef ENABLE_SERVER_AGENT
	Pi::serverAgent->ProcessResponses();
#endif

	// TODO: is it necessary to limit frame delta to 1/4th second?
	// Presumably if we're rendering < 4 FPS, we don't care about physics error either
	// if (Pi::frameTime > 0.25) Pi::frameTime = 0.25;

	accumulator += deltaTime * Pi::game->GetTimeAccelRate();

	const float step = Pi::game->GetTimeStep();
	if (step > 0.0f) {
		PROFILE_SCOPED_RAW("Physics Update [unpaused]")
		int phys_ticks = 0;
		while (accumulator >= step) {
			if (++phys_ticks >= MAX_PHYSICS_TICKS) {
				accumulator = 0.0;
				break;
			}

			Pi::game->TimeStep(step);
			BaseSphere::UpdateAllBaseSphereDerivatives();

			accumulator -= step;
		}

		// rendering interpolation between frames: don't use when docked
		// FIXME: this is the player's concern, the player should be responsible for calling Pi::SetInterpolation(false) when docked
		int pstate = Pi::game->GetPlayer()->GetFlightState();
		if (pstate == Ship::DOCKED || pstate == Ship::DOCKING || pstate == Ship::UNDOCKING)
			Pi::SetGameTickAlpha(1.0);
		else
			Pi::SetGameTickAlpha(accumulator / step);

		phys_stat += phys_ticks;
	} else {
		// paused
		PROFILE_SCOPED_RAW("Physics Update [paused]")
		BaseSphere::UpdateAllBaseSphereDerivatives();
	}

	// Record physics timestep but keep information about current frame timing.
	perfTimer.SoftStop();
	// store the physics time until the end of the frame
	phys_time = perfTimer.milliseconds() / 1.e3;

	// did the player die?
	if (Pi::game->GetPlayer()->IsDead()) {
		// FIXME: this is NOT Pi's concern at all! At the very minimum, this should go in Game.cpp
		if (!(time_player_died > 0.0)) {
			Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
			Pi::game->GetDeathView()->Init();
			Pi::SetView(Pi::game->GetDeathView());
			time_player_died = Pi::game->GetTime();
		} else if (Pi::game->GetTime() - time_player_died > 8.0) {
			// This also shouldn't go here, though we should evaluate to what degree
			// Pi.cpp is involved in queuing the tombstone loop.
			SetNextLifecycle(RefCountedPtr<TombstoneLoop>(new TombstoneLoop()));
			RequestEndLifecycle();
		}
	}

	Pi::renderer->SetTransform(matrix4x4f::Identity());

	/* Calculate position for this rendered frame (interpolated between two physics ticks */
	// XXX should this be here? what is this anyway?
	for (Body *b : Pi::game->GetSpace()->GetBodies()) {
		b->UpdateInterpTransform(Pi::GetGameTickAlpha());
	}
	Frame::GetFrame(Pi::game->GetSpace()->GetRootFrame())->UpdateInterpTransform(Pi::GetGameTickAlpha());

	Pi::GetView()->Update();
	Pi::GetView()->Draw3D();

	// Kick rendering in the background to avoid write->delete->read issues with UI command processing
	// This may cause future issues if graphic resources are deleted while in-flight, but OpenGL is
	// capable of handling that eventuality and it prevents application-scope crashes
	Pi::renderer->FlushCommandBuffers();

#ifdef REMOTE_LUA_REPL
	Pi::luaConsole->HandleTCPDebugConnections();
#endif

	// Ask ImGui to hide OS cursor if we're capturing it for input:
	// it will do this if GetMouseCursor == ImGuiMouseCursor_None.
	if (Pi::input->IsCapturingMouse()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	}

	// TODO: the escape menu depends on HandleEvents() being called before NewFrame()
	// Move HandleEvents to either the end of the loop or the very start of the loop
	// The goal is to be able to call imgui functions for debugging inside C++ code
	perfTimer.SoftReset();
	Pi::pigui->NewFrame();

	if (Pi::game && !Pi::player->IsDead()) {
		// TODO: this mechanism still isn't perfect, but it gets us out of newUI
		if (Pi::luaConsole->IsActive())
			Pi::luaConsole->Draw();
		else {
			Pi::GetView()->DrawPiGui();
			PiGui::EmitEvents();
			PiGui::RunHandler(deltaTime, "game");
		}
	}

	// Render this even when we're dead.
	if (Pi::showDebugInfo) {
		Pi::pigui->SetDebugStyle();
		perfInfoDisplay->Draw();
		Pi::pigui->SetNormalStyle();
	}

	Pi::GetApp()->DispatchEvents();

	// Reset the depth buffer so our UI can get drawn right overtop
	Pi::renderer->ClearDepthBuffer();
	Pi::pigui->Render();

	perfTimer.SoftStop();
	pigui_time = perfTimer.milliseconds() / 1.e3;

	if (Pi::game->UpdateTimeAccel())
		accumulator = 0; // fix for huge pauses 10000x -> 1x

	if (!Pi::player->IsDead()) {
		// XXX should this really be limited to while the player is alive?
		// this is something we need not do every turn...
		if (!Pi::config->Int("DisableSound")) AmbientSounds::Update();
	}

	Pi::GetMusicPlayer().Update();

	perfInfoDisplay->Update(deltaTime);
	perfInfoDisplay->UpdateCounter(PiGui::PerfInfo::COUNTER_PHYS, phys_time);
	perfInfoDisplay->UpdateCounter(PiGui::PerfInfo::COUNTER_PIGUI, pigui_time);

	// XXX: profile game startup
	if (GetProfilerAccumulate() && (SDL_GetTicks() - startup_ticks) >= profile_startup_ms) {
		SetProfilerAccumulate(false);
		Pi::GetApp()->RequestProfileFrame();
	}

	if (Pi::showDebugInfo && SDL_GetTicks() - last_stats >= 1000) {
		perfInfoDisplay->UpdateFrameInfo(frame_stat, phys_stat);
		frame_stat = 0;
		phys_stat = 0;
		if (SDL_GetTicks() - last_stats > 1200)
			last_stats = SDL_GetTicks();
		else
			last_stats += 1000;
	}
	Pi::statSceneTris = 0;
	Pi::statNumPatches = 0;

#if 0 // FIXME: decouple video recording from Pi
	if (Pi::isRecordingVideo && (Pi::ffmpegFile != nullptr)) {
		Graphics::ScreendumpState sd;
		Pi::renderer->FrameGrab(sd);
		// note: FrameGrab looked slightly buggy and now Screendump will do the same thing but without alpha channel
		// so it might need some work to ressurect this and add back in the alpha channel.
		// Also worth noting, this possibly isn't the right way to do things as it can introduce a stall of the GPU/CPU
		// interaction.  Much better to write/copy using GPU to a chain of offscreen render targets and pull data back from them
		// meaning what we write to the video may be a frame or two old, but that's acceptable for better performance.
		fwrite(sd.pixels.get(), sizeof(uint32_t) * Pi::renderer->GetWindowWidth() * Pi::renderer->GetWindowHeight(), 1, Pi::ffmpegFile);
	}
#endif
}

void GameLoop::End()
{
	// Process any pending UI events
	PiGui::EmitEvents();

	// When Pi::game goes, so too goes the death view.
	Pi::SetView(0);

	// Clean up any left-over mouse state
	Pi::input->SetCapturingMouse(false);

#ifdef REMOTE_LUA_REPL
	Pi::luaConsole->CloseTCPDebugConnection();
#endif

	// we have to make sure to autosave the game before the end game process starts
	LuaEvent::Queue("onAutoSaveBeforeGameEnds");
	LuaEvent::Emit();

	// final event
	LuaEvent::Queue("onGameEnd");
	LuaEvent::Emit();

	Pi::luaTimer->RemoveAll();

	Lua::manager->CollectGarbage();

	if (!Pi::config->Int("DisableSound")) AmbientSounds::Uninit();
	Sound::DestroyAllEventsExceptMusic();

	assert(Pi::game);
	delete Pi::game;
	Pi::game = nullptr;
	Pi::player = nullptr;
}

/*
===============================================================================
	TOMBSTONE LOOP
===============================================================================
*/

void TombstoneLoop::Start()
{
	// TODO: just calculate this at draw time inside Tombstone
	tombstone.reset(new Tombstone(Pi::renderer, Pi::renderer->GetWindowWidth(), Pi::renderer->GetWindowHeight()));
	startTime = Pi::GetApp()->GetTime();
}

void TombstoneLoop::Update(float deltaTime)
{
	Pi::GetApp()->HandleEvents();

	// TODO: improve Tombstone, add pigui drawing, etc.
	tombstone->Draw(accumTime);
	accumTime += deltaTime;

	bool hasInput = Pi::input->MouseButtonState(SDL_BUTTON_LEFT) || Pi::input->MouseButtonState(SDL_BUTTON_RIGHT) || Pi::input->KeyState(SDLK_SPACE);

	if ((accumTime > 2.0 && hasInput) || accumTime > 8.0) {
		RequestEndLifecycle();
		Pi::GetMusicPlayer().Stop();
		Sound::DestroyAllEvents();
	}
}

/*
===============================================================================
	MISCELLANEOUS GARBAGE THAT OUGHT NOT TO BE IN THIS CLASS
===============================================================================
*/

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

// request that the game is ended as soon as safely possible
void Pi::RequestEndGame()
{
	GetApp()->internalRequests.push_back(App::InternalRequests::END_GAME);
}

void Pi::RequestQuit()
{
	GetApp()->internalRequests.push_back(App::InternalRequests::QUIT_GAME);
}

/*
	GARBAGE THAT ESPECIALLY SHOULD NOT BE HERE
*/

void Pi::SetView(View *v)
{
	if (currentView) currentView->Detach();
	currentView = v;
	if (currentView) currentView->Attach();
	LuaEvent::Queue("onViewChanged");
}

void Pi::OnChangeDetailLevel()
{
	Pi::GetApp()->internalRequests.push_back(App::InternalRequests::DETAIL_LEVEL_CHANGED);
}

static void OnPlayerDockOrUndock()
{
	Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
	Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
}

// This absolutely ought not to be part of the Pi class
#if 0
static void SetVideoRecording(bool enabled)
{
	Pi::isRecordingVideo = enabled;
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
}
#endif

#if 0
void printShipStats()
{
	// test code to produce list of ship stats

	FILE *pStatFile = fopen("shipstat.csv", "wt");
	if (pStatFile) {
		fprintf(pStatFile, "name,modelname,hullmass,capacity,fakevol,rescale,xsize,ysize,zsize,facc,racc,uacc,sacc,aacc,exvel\n");
		for (auto iter : ShipType::types) {
			const ShipType *shipdef = &(iter.second);
			SceneGraph::Model *model = Pi::FindModel(shipdef->modelName, false);

			double hullmass = shipdef->hullMass;
			double capacity = shipdef->capacity;

			double xsize = 0.0, ysize = 0.0, zsize = 0.0, fakevol = 0.0, rescale = 0.0, brad = 0.0;
			if (model) {
				std::unique_ptr<SceneGraph::Model> inst(model->MakeInstance());
				model->CreateCollisionMesh();
				Aabb aabb = model->GetCollisionMesh()->GetAabb();
				xsize = aabb.max.x - aabb.min.x;
				ysize = aabb.max.y - aabb.min.y;
				zsize = aabb.max.z - aabb.min.z;
				fakevol = xsize * ysize * zsize;
				brad = aabb.GetRadius();
				rescale = pow(fakevol / (100 * (hullmass + capacity)), 0.3333333333);
			}

			double simass = (hullmass + capacity) * 1000.0;
			double angInertia = (2 / 5.0) * simass * brad * brad;
			double acc1 = shipdef->linThrust[Thruster::THRUSTER_FORWARD] / (9.81 * simass);
			double acc2 = shipdef->linThrust[Thruster::THRUSTER_REVERSE] / (9.81 * simass);
			double acc3 = shipdef->linThrust[Thruster::THRUSTER_UP] / (9.81 * simass);
			double acc4 = shipdef->linThrust[Thruster::THRUSTER_RIGHT] / (9.81 * simass);
			double acca = shipdef->angThrust / angInertia;
			double exvel = shipdef->effectiveExhaustVelocity;

			fprintf(pStatFile, "%s,%s,%.1f,%.1f,%.1f,%.3f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%f,%.1f\n",
				shipdef->name.c_str(), shipdef->modelName.c_str(), hullmass, capacity,
				fakevol, rescale, xsize, ysize, zsize, acc1, acc2, acc3, acc4, acca, exvel);
		}
		fclose(pStatFile);
	}
}
#endif
