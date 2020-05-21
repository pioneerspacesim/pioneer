// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "buildopts.h"

#include "Pi.h"

#include "BaseSphere.h"
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
#include "Intro.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "Missile.h"
#include "ModManager.h"
#include "ModelCache.h"
#include "NavLights.h"
#include "core/GuiApplication.h"
#include "core/Log.h"
#include "core/OS.h"
#include "graphics/opengl/RendererGL.h"
#include "lua/Lua.h"
#include "lua/LuaConsole.h"
#include "lua/LuaEvent.h"
#include "lua/LuaPiGui.h"
#include "lua/LuaTimer.h"
#include "profiler/Profiler.h"
#include "sound/AmbientSounds.h"
#if WITH_OBJECTVIEWER
#include "ObjectViewerView.h"
#endif
#include "Beam.h"
#include "Player.h"
#include "Projectile.h"
#include "SectorView.h"
#include "Sfx.h"
#include "Shields.h"
#include "ShipCpanel.h"
#include "ShipType.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Star.h"
#include "StringF.h"
#include "SystemInfoView.h"
#include "SystemView.h"
#include "Tombstone.h"
#include "UIView.h"
#include "WorldView.h"
#include "galaxy/GalaxyGenerator.h"
#include "gameui/Lua.h"
#include "libs.h"
#include "pigui/PerfInfo.h"
#include "pigui/PiGui.h"
#include "pigui/PiGuiLua.h"
#include "ship/PlayerShipController.h"
#include "ship/ShipViewController.h"
#include "sound/Sound.h"
#include "sound/SoundMusic.h"

#include "graphics/Renderer.h"

#if WITH_DEVKEYS
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Stats.h"
#endif // WITH_DEVKEYS

#include "scenegraph/Lua.h"
#include "versioningInfo.h"

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
sigc::signal<void> Pi::onPlayerChangeTarget;
sigc::signal<void> Pi::onPlayerChangeFlightControlState;
LuaSerializer *Pi::luaSerializer;
LuaTimer *Pi::luaTimer;
LuaNameGen *Pi::luaNameGen;
#ifdef ENABLE_SERVER_AGENT
ServerAgent *Pi::serverAgent;
#endif
Input *Pi::input;
Player *Pi::player;
View *Pi::currentView;
TransferPlanner *Pi::planner;
LuaConsole *Pi::luaConsole;
Game *Pi::game;
Random Pi::rng;
float Pi::frameTime;
bool Pi::doingMouseGrab;
bool Pi::showDebugInfo = false;
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
PiGui::Instance *Pi::pigui = nullptr;
ModelCache *Pi::modelCache;
Intro *Pi::intro;
SDLGraphics *Pi::sdl;
Graphics::RenderTarget *Pi::renderTarget;
RefCountedPtr<Graphics::Texture> Pi::renderTexture;
std::unique_ptr<Graphics::Drawables::TexturedQuad> Pi::renderQuad;
Graphics::RenderState *Pi::quadRenderState = nullptr;
bool Pi::isRecordingVideo = false;
FILE *Pi::ffmpegFile = nullptr;

Pi::App *Pi::m_instance = nullptr;

Sound::MusicPlayer Pi::musicPlayer;
std::unique_ptr<AsyncJobQueue> Pi::asyncJobQueue;
std::unique_ptr<SyncJobQueue> Pi::syncJobQueue;

class LoadStep : public Application::Lifecycle {
public:
	LoadStep() :
		Lifecycle(true)
	{}

protected:
	struct LoaderStep {
		// TODO: use a lighter-weight wrapper over lambdas instead of std::function
		std::function<void()> fn;
		std::string name;
	};

	std::vector<LoaderStep> m_loaders;
	size_t m_currentLoader = 0;

	template <typename T>
	void AddStep(std::string name, T fn)
	{
		m_loaders.push_back(LoaderStep{ fn, name });
	}

	Profiler::Clock m_loadTimer;

	void Start() override;
	void Update(float) override;
};

// FIXME: this is a hack, this class should have its lifecycle managed elsewhere
// Ideally an application framework class handles this (as well as the rest of the main loop)
// but for now this is the best we have.
std::unique_ptr<PiGUI::PerfInfo> perfInfoDisplay;

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

	int frame_stat;
	int phys_stat;

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
	Pi::config = new GameConfig(options);
	m_instance = new Pi::App();

	GetApp()->m_loader = std::make_shared<LoadStep>();
	GetApp()->m_mainMenu = std::make_shared<MainMenu>();
	GetApp()->m_gameLoop = std::make_shared<GameLoop>();

	m_instance->m_noGui = no_gui;
}

void Pi::App::SetStartPath(const SystemPath &startPath)
{
	static_cast<MainMenu *>(m_mainMenu.get())->SetStartPath(startPath);
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

	ShipViewController::InputBindings.RegisterBindings();

	WorldView::RegisterInputBindings();
}

void Pi::App::Startup()
{
	PROFILE_SCOPED()
	Profiler::Clock startupTimer;
	startupTimer.Start();

	Application::Startup();
#if PIONEER_PROFILER
	Pi::profilerPath = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
#endif

	Log::GetLog()->SetLogFile("output.txt");

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

	// FIXME: move these out of the Pi namespace
	// TODO: add a better configuration interface for this kind of thing
	Pi::SetAmountBackgroundStars(config->Float("AmountOfBackgroundStars"));
	Pi::detail.planets = config->Int("DetailPlanets");
	Pi::detail.cities = config->Int("DetailCities");

	Graphics::RendererOGL::RegisterRenderer();
	Pi::renderer = StartupRenderer(Pi::config);

	Pi::rng.IncRefCount(); // so nothing tries to free it
	Pi::rng.seed(time(0));

	Pi::input = StartupInput(config);
	Pi::input->onKeyPress.connect(sigc::ptr_fun(&Pi::HandleKeyDown));

	// we can only do bindings once joysticks are initialised.
	if (!m_noGui) // This re-saves the config file. With no GUI we want to allow multiple instances in parallel.
		KeyBindings::InitBindings();

	RegisterInputBindings();

	Pi::pigui = StartupPiGui();

	// FIXME: move these into the appropriate class!
	navTunnelDisplayed = (config->Int("DisplayNavTunnel")) ? true : false;
	speedLinesDisplayed = (config->Int("SpeedLines")) ? true : false;
	hudTrailsDisplayed = (config->Int("HudTrails")) ? true : false;

	TestGPUJobsSupport();

	EnumStrings::Init();

	Profiler::Clock threadTimer;
	threadTimer.Start();

	// get threads up
	Uint32 numThreads = config->Int("WorkerThreads");
	numThreads = numThreads ? numThreads : std::max(OS::GetNumCores() - 1, 1U);
	Pi::asyncJobQueue.reset(new AsyncJobQueue(numThreads));
	Pi::syncJobQueue.reset(new SyncJobQueue);

	threadTimer.Stop();
	Output("started %d worker threads in %.2fms\n", numThreads, threadTimer.milliseconds());

#ifdef PIONEER_PROFILER
	Profiler::dumphtml(profilerPath.c_str());
#endif

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
void Pi::App::Shutdown()
{
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
	delete Pi::luaConsole;
	NavLights::Uninit();
	Shields::Uninit();
	SfxManager::Uninit();
	Sound::Uninit();
	CityOnPlanet::Uninit();
	BaseSphere::Uninit();
	FaceParts::Uninit();
	Graphics::Uninit();

	PiGUI::Lua::Uninit();
	ShutdownPiGui();
	Pi::pigui = nullptr;
	Pi::ui.Reset(0);
	Lua::UninitModules();
	Lua::Uninit();
	Gui::Uninit();

	delete Pi::modelCache;

	GalaxyGenerator::Uninit();

	ShutdownRenderer();
	Pi::renderer = nullptr;

	ShutdownInput();
	Pi::input = nullptr;

	delete Pi::config;
	delete Pi::planner;
	asyncJobQueue.reset();
	syncJobQueue.reset();

	Application::Shutdown();

	SDL_Quit();
	delete Pi::m_instance;
	exit(0);
}

/*
===============================================================================
	LOADING
===============================================================================
*/

// TODO: investigate constructing a DAG out of Init functions and dependencies
void LoadStep::Start()
{
	Output("LoadStep::Start()\n");
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
	PiGUI::Lua::Init();

	// Don't render the first frame, just make sure all of our fonts are loaded
	Pi::pigui->NewFrame();
	PiGUI::RunHandler(0.01, "INIT");
	Pi::pigui->EndFrame();

	AddStep("UI::AddContext", []() {
		float ui_scale = Pi::config->Float("UIScaleFactor", 1.0f);
		if (Graphics::GetScreenHeight() < 768) {
			ui_scale = float(Graphics::GetScreenHeight()) / 768.0f;
		}

		Pi::ui.Reset(new UI::Context(
			Lua::manager,
			Pi::renderer,
			Graphics::GetScreenWidth(),
			Graphics::GetScreenHeight(),
			ui_scale));
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

	AddStep("Gui::Init()", []() {
		Gui::Init(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), 800, 600);
	});

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

	AddStep("PostLoad", []() {
		Pi::luaConsole = new LuaConsole();
		KeyBindings::toggleLuaConsole.onPress.connect(sigc::mem_fun(Pi::luaConsole, &LuaConsole::Toggle));

		Pi::planner = new TransferPlanner();

		perfInfoDisplay.reset(new PiGUI::PerfInfo());
	});
}

void LoadStep::Update(float deltaTime)
{
	if (m_currentLoader < m_loaders.size()) {
		LoaderStep &loader = m_loaders[m_currentLoader++];
		float progress = (m_currentLoader) / float(m_loaders.size());
		Output("Loading [%02.f%%]: %s started\n", progress * 100., loader.name.c_str());

		Profiler::Clock timer;
		timer.Start();

		loader.fn();

		timer.Stop();
		Output("Loading [%02.f%%]: %s took %.2fms\n", progress * 100.,
			loader.name.c_str(), timer.milliseconds());

		Pi::pigui->NewFrame();
		PiGUI::RunHandler(progress, "INIT");
		Pi::pigui->Render();

	} else {
		OS::NotifyLoadEnd();
		RequestEndLifecycle();

		m_loadTimer.Stop();
		Output("\n\nPioneer loading took %.2fms\n", m_loadTimer.milliseconds());
	}
}

/*
===============================================================================
	MAIN MENU
===============================================================================
*/

void MainMenu::Start()
{
	Pi::intro = new Intro(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
	if (m_skipMenu) {
		Output("Loading new game immediately!\n");
		Pi::StartGame(new Game(m_startPath, 0.0));
		m_skipMenu = false; // Show the main menu once we're done here.
	}

	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(51, 51, 51, 255));
}

void MainMenu::Update(float deltaTime)
{
	Pi::GetApp()->HandleEvents();

	Pi::intro->Draw(deltaTime);

	Pi::pigui->NewFrame();
	PiGUI::RunHandler(deltaTime, "MAINMENU");

	perfInfoDisplay->Update(deltaTime * 1e3, 0.0);
	if (Pi::showDebugInfo) {
		perfInfoDisplay->Draw();
	}

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

// FIXME: move out of Pi.cpp into a proper debug interface!
static void SpawnTestObjects();

void Pi::HandleKeyDown(SDL_Keysym *key)
{
	if (key->sym == SDLK_ESCAPE) {
		if (Pi::game) {
			// only accessible once game started
			HandleEscKey();
		}
		return;
	}

	const bool CTRL = input->KeyState(SDLK_LCTRL) || input->KeyState(SDLK_RCTRL);
	if (!CTRL) {
		return;
	}

	// special keys: CTRL+[KEY].
	switch (key->sym) {
	case SDLK_q: // Quit
		Pi::RequestQuit();
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
			Pi::doProfileOne = true;
		else {
			Pi::doProfileSlow = !Pi::doProfileSlow;
			Output("slow frame profiling %s\n", Pi::doProfileSlow ? "enabled" : "disabled");
		}
		break;
#endif

	case SDLK_F11: // Reload shaders
		renderer->ReloadShaders();
		break;

	case SDLK_F12: // Spawn
	{
		if (Pi::game)
			SpawnTestObjects();

		break;
	}
#endif /* DEVKEYS */

#if WITH_OBJECTVIEWER
	case SDLK_F10:
		Pi::SetView(Pi::game->GetObjectViewerView());
		break;
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

void Pi::HandleEscKey()
{
	if (currentView == 0)
		return;

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

// Return true if the event has been handled and shouldn't be passed through
// to the normal input system.
bool Pi::App::HandleEvent(SDL_Event &event)
{
	PROFILE_SCOPED()

	// HACK for most keypresses SDL will generate KEYUP/KEYDOWN and TEXTINPUT
	// events. keybindings run off KEYUP/KEYDOWN. the console is opened/closed
	// via keybinding. the console TextInput widget uses TEXTINPUT events. thus
	// after switching the console, the stray TEXTINPUT event causes the
	// console key (backtick) to appear in the text entry field. we hack around
	// this by setting this flag if the console was switched. if its set, we
	// swallow the TEXTINPUT event this hack must remain until we have a
	// unified input system
	// This is safely able to be removed once GUI and newUI are gone
	static bool skipTextInput = false;

	if (skipTextInput && event.type == SDL_TEXTINPUT) {
		skipTextInput = false;
		return true;
	}

	if (ui->DispatchSDLEvent(event))
		return true;

	bool consoleActive = Pi::IsConsoleActive();
	if (!consoleActive) {
		KeyBindings::DispatchSDLEvent(&event);
		if (currentView)
			currentView->HandleSDLEvent(event);
	} else {
		KeyBindings::toggleLuaConsole.CheckSDLEventAndDispatch(&event);
	}

	if (consoleActive != Pi::IsConsoleActive()) {
		skipTextInput = true;
		return true;
	}

	if (Pi::IsConsoleActive())
		return true;

	Gui::HandleSDLEvent(&event);

	return false;
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
	Pi::frameTime = DeltaTime();

	GuiApplication::PreUpdate();
}

void Pi::App::PostUpdate()
{
	GuiApplication::PostUpdate();

	HandleRequests();
}

void Pi::App::RunJobs()
{
	Pi::syncJobQueue->RunJobs(SYNC_JOBS_PER_LOOP);
	Pi::asyncJobQueue->FinishJobs();
	Pi::syncJobQueue->FinishJobs();
}

// FIXME: delete/move this function out of Pi.cpp
static void OnPlayerDockOrUndock();

void GameLoop::Start()
{
	// this is a bit brittle. skank may be forgotten and survive between
	// games
	Pi::input->InitGame();

	if (!Pi::config->Int("DisableSound")) AmbientSounds::Init();

	LuaEvent::Clear();

	Pi::player->onDock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onUndock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onLanded.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::game->GetCpan()->ShowAll();
	Pi::DrawGUI = true;
	Pi::SetView(Pi::game->GetWorldView());

#ifdef REMOTE_LUA_REPL
#ifndef REMOTE_LUA_REPL_PORT
#define REMOTE_LUA_REPL_PORT 12345
#endif
	luaConsole->OpenTCPDebugConnection(REMOTE_LUA_REPL_PORT);
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
}

void GameLoop::Update(float deltaTime)
{
	PROFILE_SCOPED()
	perfTimer.SoftReset();			   // Reset() + Start()
	frame_time_real = deltaTime * 1e3; // convert to ms
	frame_stat++;

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
	phys_time = perfTimer.milliseconds();

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
			SetNextLifecycle(std::make_shared<TombstoneLoop>());
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

	// FIXME: HandleEvents at the moment must be after view->Draw3D and before
	// Gui::Draw so that labels drawn to screen can have mouse events correctly
	// detected. Gui::Draw wipes memory of label positions.
	Pi::GetApp()->HandleEvents();

#ifdef REMOTE_LUA_REPL
	Pi::luaConsole->HandleTCPDebugConnections();
#endif

	// Reset the depth buffer so our UI can get drawn right overtop
	Pi::renderer->ClearDepthBuffer();

	// FIXME: GUI must die!
	if (Pi::DrawGUI) {
		Gui::Draw();
	}

	// FIXME: newUI must die!
	// ...also we need better handling of death states
	// XXX don't draw the UI during death obviously a hack, and still
	// wrong, because we shouldn't this when the HUD is disabled, but
	// probably sure draw it if they switch to eg infoview while the HUD is
	// disabled so we need much smarter control for all this rubbish
	if ((!Pi::game || Pi::GetView() != Pi::game->GetDeathView()) && Pi::DrawGUI) {
		Pi::ui->Update();
		Pi::ui->Draw();
	}

	// Ask ImGui to hide OS cursor if we're capturing it for input:
	// it will do this if GetMouseCursor == ImGuiMouseCursor_None.
	if (Pi::input->IsCapturingMouse()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	}

	// TODO: the escape menu depends on HandleEvents() being called before NewFrame()
	// Move HandleEvents to either the end of the loop or the very start of the loop
	// The goal is to be able to call imgui functions for debugging inside C++ code
	Pi::pigui->NewFrame();

	if (Pi::game && !Pi::player->IsDead()) {
		// FIXME: Always begin a camera frame because WorldSpaceToScreenSpace
		// requires it and is exposed to pigui.
		Pi::game->GetWorldView()->BeginCameraFrame();
		// FIXME: major hack to work around the fact that the console is in newUI and not pigui
		if (!Pi::IsConsoleActive())
			PiGUI::RunHandler(deltaTime, "GAME");
		Pi::game->GetWorldView()->EndCameraFrame();
	}

	// Render this even when we're dead.
	if (Pi::showDebugInfo) {
		perfInfoDisplay->Draw();
	}

	Pi::pigui->Render();

	if (Pi::game->UpdateTimeAccel())
		accumulator = 0; // fix for huge pauses 10000x -> 1x

	if (!Pi::player->IsDead()) {
		// XXX should this really be limited to while the player is alive?
		// this is something we need not do every turn...
		if (!Pi::config->Int("DisableSound")) AmbientSounds::Update();
	}

	Pi::GetMusicPlayer().Update();

	// FIXME: oldUI must DIEEEEE!
	Pi::game->GetCpan()->Update();

	Pi::GetApp()->RunJobs();

	perfInfoDisplay->Update(frame_time_real, phys_time);
	if (Pi::showDebugInfo && SDL_GetTicks() - last_stats >= 1000) {
		perfInfoDisplay->UpdateFrameInfo(frame_stat, phys_stat);
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
	if (Pi::doProfileOne || (Pi::doProfileSlow && (deltaTime > 0.1))) { // slow: < ~10fps
		Output("dumping profile data\n");
		Profiler::dumphtml(Pi::profilerPath.c_str());
		Pi::doProfileOne = false;
	}
#endif

#if 0 // FIXME: decouple video recording from Pi
	if (Pi::isRecordingVideo && (Pi::ffmpegFile != nullptr)) {
		Graphics::ScreendumpState sd;
		Pi::renderer->FrameGrab(sd);
		fwrite(sd.pixels.get(), sizeof(uint32_t) * Pi::renderer->GetWindowWidth() * Pi::renderer->GetWindowHeight(), 1, Pi::ffmpegFile);
	}
#endif
}

void GameLoop::End()
{
	// When Pi::game goes, so too goes the death view.
	Pi::SetView(0);

	// Clean up any left-over mouse state
	Pi::input->SetCapturingMouse(false);

	Pi::SetMouseGrab(false);

	Pi::GetMusicPlayer().Stop();
	Sound::DestroyAllEvents();

	// final event
	LuaEvent::Queue("onGameEnd");
	LuaEvent::Emit();

	Pi::luaTimer->RemoveAll();

	Lua::manager->CollectGarbage();

	if (!Pi::config->Int("DisableSound")) AmbientSounds::Uninit();
	Sound::DestroyAllEvents();

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
	tombstone.reset(new Tombstone(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight()));
	startTime = Pi::GetApp()->GetTime();
}

void TombstoneLoop::Update(float deltaTime)
{
	Pi::GetApp()->HandleEvents();

	// TODO: improve Tombstone, add pigui drawing, etc.
	tombstone->Draw(accumTime);
	accumTime += deltaTime;

	bool hasInput = Pi::input->MouseButtonState(SDL_BUTTON_LEFT) || Pi::input->MouseButtonState(SDL_BUTTON_RIGHT) || Pi::input->KeyState(SDLK_SPACE);

	if ((accumTime > 2.0 && hasInput) || accumTime > 8.0)
		RequestEndLifecycle();
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

bool Pi::IsConsoleActive()
{
	return luaConsole && luaConsole->IsActive();
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

static void OnPlayerDockOrUndock()
{
	Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
	Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
}

float Pi::GetMoveSpeedShiftModifier()
{
	// Suggestion: make x1000 speed on pressing both keys?
	if (Pi::input->KeyState(SDLK_LSHIFT)) return 100.f;
	if (Pi::input->KeyState(SDLK_RSHIFT)) return 10.f;
	return 1;
}

void Pi::SetMouseGrab(bool on)
{
	if (!doingMouseGrab && on) {
		Pi::input->SetCapturingMouse(true);
		Pi::ui->SetMousePointerEnabled(false);
		doingMouseGrab = true;
	} else if (doingMouseGrab && !on) {
		Pi::input->SetCapturingMouse(false);
		Pi::ui->SetMousePointerEnabled(true);
		doingMouseGrab = false;
	}
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

static void SpawnTestObjects()
{
	vector3d dir = -Pi::player->GetOrient().VectorZ();
	/* add test object */
	if (Pi::input->KeyState(SDLK_RSHIFT)) {
		Missile *missile =
			new Missile(ShipType::MISSILE_GUIDED, Pi::player);
		missile->SetOrient(Pi::player->GetOrient());
		missile->SetFrame(Pi::player->GetFrame());
		missile->SetPosition(Pi::player->GetPosition() + 50.0 * dir);
		missile->SetVelocity(Pi::player->GetVelocity());
		Pi::game->GetSpace()->AddBody(missile);
		missile->AIKamikaze(Pi::player->GetCombatTarget());
	} else if (Pi::input->KeyState(SDLK_LSHIFT)) {
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
				Pi::game->GetSpace()->AddBody(ship);
			} else {
				delete ship;
				Output("No docking ports free dude\n");
			}
		} else {
			Output("Select a space station...\n");
		}
	} else {
		Ship *ship = new Ship(ShipType::POLICE);
		if (!Pi::input->KeyState(SDLK_LALT)) { //Left ALT = no AI
			if (!Pi::input->KeyState(SDLK_LCTRL))
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
		Pi::game->GetSpace()->AddBody(ship);
	}
}

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
