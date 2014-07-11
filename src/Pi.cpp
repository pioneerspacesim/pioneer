// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "libs.h"
#include "AmbientSounds.h"
#include "CargoBody.h"
#include "CityOnPlanet.h"
#include "DeathView.h"
#include "FaceGenManager.h"
#include "Factions.h"
#include "FileSystem.h"
#include "Frame.h"
#include "GalacticView.h"
#include "Game.h"
#include "BaseSphere.h"
#include "Intro.h"
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
#include "LuaLang.h"
#include "LuaManager.h"
#include "LuaMissile.h"
#include "LuaMusic.h"
#include "LuaNameGen.h"
#include "LuaRef.h"
#include "LuaShipDef.h"
#include "LuaSpace.h"
#include "LuaTimer.h"
#include "Missile.h"
#include "ModelCache.h"
#include "ModManager.h"
#include "NavLights.h"
#include "Shields.h"
#include "ObjectViewerView.h"
#include "OS.h"
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include "SDLWrappers.h"
#include "SectorView.h"
#include "Serializer.h"
#include "Sfx.h"
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
#include "KeyBindings.h"
#include "EnumStrings.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "gameui/Lua.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"
#include "gui/Gui.h"
#include "scenegraph/Model.h"
#include "scenegraph/Lua.h"
#include "ui/Context.h"
#include "ui/Lua.h"
#include <algorithm>
#include <sstream>

float Pi::gameTickAlpha;
sigc::signal<void, SDL_Keysym*> Pi::onKeyPress;
sigc::signal<void, SDL_Keysym*> Pi::onKeyRelease;
sigc::signal<void, int, int, int> Pi::onMouseButtonUp;
sigc::signal<void, int, int, int> Pi::onMouseButtonDown;
sigc::signal<void, bool> Pi::onMouseWheel;
sigc::signal<void> Pi::onPlayerChangeTarget;
sigc::signal<void> Pi::onPlayerChangeFlightControlState;
LuaSerializer *Pi::luaSerializer;
LuaTimer *Pi::luaTimer;
LuaNameGen *Pi::luaNameGen;
int Pi::keyModState;
std::map<SDL_Keycode,bool> Pi::keyState; // XXX SDL2 SDLK_LAST
char Pi::mouseButton[6];
int Pi::mouseMotion[2];
bool Pi::doingMouseGrab = false;
bool Pi::warpAfterMouseGrab = false;
int Pi::mouseGrabWarpPos[2];
Player *Pi::player;
View *Pi::currentView;
WorldView *Pi::worldView;
DeathView *Pi::deathView;
UIView *Pi::spaceStationView;
UIView *Pi::infoView;
SectorView *Pi::sectorView;
GalacticView *Pi::galacticView;
UIView *Pi::settingsView;
SystemView *Pi::systemView;
SystemInfoView *Pi::systemInfoView;
ShipCpanel *Pi::cpan;
LuaConsole *Pi::luaConsole;
Game *Pi::game;
Random Pi::rng;
float Pi::frameTime;
#if WITH_DEVKEYS
bool Pi::showDebugInfo = false;
#endif
#if PIONEER_PROFILER
std::string Pi::profilerPath;
bool Pi::doProfileSlow = false;
bool Pi::doProfileOne = false;
#endif
int Pi::statSceneTris;
GameConfig *Pi::config;
struct DetailLevel Pi::detail = { 0, 0 };
bool Pi::joystickEnabled;
bool Pi::mouseYInvert;
std::map<SDL_JoystickID,Pi::JoystickState> Pi::joysticks;
bool Pi::navTunnelDisplayed;
bool Pi::speedLinesDisplayed = false;
bool Pi::hudTrailsDisplayed = false;
Gui::Fixed *Pi::menu;
bool Pi::DrawGUI = true;
Graphics::Renderer *Pi::renderer;
RefCountedPtr<UI::Context> Pi::ui;
ModelCache *Pi::modelCache;
Intro *Pi::intro;
SDLGraphics *Pi::sdl;
Graphics::RenderTarget *Pi::renderTarget;
RefCountedPtr<Graphics::Texture> Pi::renderTexture;
std::unique_ptr<Graphics::Drawables::TexturedQuad> Pi::renderQuad;
Graphics::RenderState *Pi::quadRenderState = nullptr;

#if WITH_OBJECTVIEWER
ObjectViewerView *Pi::objectViewerView;
#endif

Sound::MusicPlayer Pi::musicPlayer;
std::unique_ptr<AsyncJobQueue> Pi::asyncJobQueue;
std::unique_ptr<SyncJobQueue> Pi::syncJobQueue;

Galaxy* Pi::s_galaxy = nullptr;

// XXX enabling this breaks UI gauge rendering. see #2627
#define USE_RTT 0

//static
void Pi::CreateRenderTarget(const Uint16 width, const Uint16 height) {
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
	rsd.depthTest  = false;
	rsd.depthWrite = false;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	quadRenderState = Pi::renderer->CreateRenderState(rsd);

	Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888,
		vector2f(width, height),
		Graphics::LINEAR_CLAMP, false, false, 0);
	Pi::renderTexture.Reset(Pi::renderer->CreateTexture(texDesc));
	Pi::renderQuad.reset(new Graphics::Drawables::TexturedQuad(
		Pi::renderer, Pi::renderTexture.Get(), 
		vector2f(0.0f,0.0f), vector2f(float(Graphics::GetScreenWidth()), float(Graphics::GetScreenHeight())), 
		quadRenderState));

	// Complete the RT description so we can request a buffer.
	// NB: we don't want it to create use a texture because we share it with the textured quad created above.
	Graphics::RenderTargetDesc rtDesc(
		width,
		height,
		Graphics::TEXTURE_NONE,		// don't create a texture
		Graphics::TEXTURE_DEPTH,
		false);
	Pi::renderTarget = Pi::renderer->CreateRenderTarget(rtDesc);

	Pi::renderTarget->SetColorTexture(Pi::renderTexture.Get());
#endif
}

//static
void Pi::DrawRenderTarget() {
#if USE_RTT
	Pi::renderer->BeginFrame();
	Pi::renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());	
	Pi::renderer->SetTransform(matrix4x4f::Identity());

	//Gui::Screen::EnterOrtho();
	{
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::PROJECTION);
		Pi::renderer->PushMatrix();
		Pi::renderer->SetOrthographicProjection(0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), 0, -1, 1);
		Pi::renderer->SetMatrixMode(Graphics::MatrixMode::MODELVIEW);
		Pi::renderer->PushMatrix();
		Pi::renderer->LoadIdentity();
	}
	
	Pi::renderQuad->Draw( Pi::renderer );

	//Gui::Screen::LeaveOrtho();
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
void Pi::BeginRenderTarget() {
#if USE_RTT
	Pi::renderer->SetRenderTarget(Pi::renderTarget);
#endif
}

//static
void Pi::EndRenderTarget() {
#if USE_RTT
	Pi::renderer->SetRenderTarget(nullptr);
#endif
}

static void draw_progress(UI::Gauge *gauge, UI::Label *label, float progress)
{
	gauge->SetValue(progress);
	label->SetText(stringf(Lang::SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS, formatarg("age", progress * 13.7f)));

	Pi::renderer->ClearScreen();
	Pi::ui->Update();
	Pi::ui->Draw();
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
	LuaFileSystem::Register();
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

	// XXX load everything. for now, just modules
	lua_State *l = Lua::manager->GetLuaState();
	pi_lua_dofile(l, "libs/autoload.lua");
	pi_lua_dofile_recursive(l, "ui");
	pi_lua_dofile_recursive(l, "modules");

	Pi::luaNameGen = new LuaNameGen(Lua::manager);
}

static void LuaUninit() {
	delete Pi::luaNameGen;

	delete Pi::luaSerializer;
	delete Pi::luaTimer;

	Lua::Uninit();
}

static void LuaInitGame() {
	LuaEvent::Clear();
}

SceneGraph::Model *Pi::FindModel(const std::string &name, bool allowPlaceholder)
{
	SceneGraph::Model *m = 0;
	try {
		m = Pi::modelCache->FindModel(name);
	} catch (ModelCache::ModelNotFoundException) {
		Output("Could not find model: %s\n", name.c_str());
		if (allowPlaceholder) {
			try {
				m = Pi::modelCache->FindModel("error");
			} catch (ModelCache::ModelNotFoundException) {
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

void Pi::Init(const std::map<std::string,std::string> &options, bool no_gui)
{
#ifdef PIONEER_PROFILER
	Profiler::reset();
#endif

	Profiler::Timer timer;
	timer.Start();

	OS::NotifyLoadBegin();

	FileSystem::Init();
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
#ifdef PIONEER_PROFILER
	FileSystem::userFiles.MakeDirectory("profiler");
	profilerPath = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler");
#endif

	Pi::config = new GameConfig(options);
	if (!no_gui) // This re-saves the config file. With no GUI we want to allow multiple instances in parallel.
		KeyBindings::InitBindings();

	if (config->Int("RedirectStdio"))
		OS::RedirectStdio();

	std::string version(PIONEER_VERSION);
	if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
	const char* platformName = SDL_GetPlatform();
	if(platformName)
		Output("ver %s on: %s\n\n", version.c_str(), platformName);
	else
		Output("ver %s but could not detect platform name.\n\n", version.c_str());

	Output("%s\n", OS::GetOSInfoString().c_str());

	ModManager::Init();

	Lang::Resource res(Lang::GetResource("core", config->String("Lang")));
	Lang::MakeCore(res);

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

	// Do rest of SDL video initialization and create Renderer
	Graphics::Settings videoSettings = {};
	videoSettings.width = config->Int("ScrWidth");
	videoSettings.height = config->Int("ScrHeight");
	videoSettings.fullscreen = (config->Int("StartFullscreen") != 0);
	videoSettings.hidden = no_gui;
	videoSettings.requestedSamples = config->Int("AntiAliasingMode");
	videoSettings.vsync = (config->Int("VSync") != 0);
	videoSettings.useTextureCompression = (config->Int("UseTextureCompression") != 0);
	videoSettings.enableDebugMessages = (config->Int("EnableGLDebug") != 0);
	videoSettings.iconFile = OS::GetIconFilename();
	videoSettings.title = "Pioneer";

	Pi::renderer = Graphics::Init(videoSettings);
	{
		std::ostringstream buf;
		renderer->PrintDebugInfo(buf);

		FILE *f = FileSystem::userFiles.OpenWriteStream("opengl.txt", FileSystem::FileSourceFS::WRITE_TEXT);
		if (!f)
			Output("Could not open 'opengl.txt'\n");
		const std::string &s = buf.str();
		fwrite(s.c_str(), 1, s.size(), f);
		fclose(f);
	}

	Pi::CreateRenderTarget(videoSettings.width, videoSettings.height);
	Pi::rng.IncRefCount(); // so nothing tries to free it
	Pi::rng.seed(time(0));

	InitJoysticks();
	joystickEnabled = (config->Int("EnableJoystick")) ? true : false;
	mouseYInvert = (config->Int("InvertMouseY")) ? true : false;

	navTunnelDisplayed = (config->Int("DisplayNavTunnel")) ? true : false;
	speedLinesDisplayed = (config->Int("SpeedLines")) ? true : false;
	hudTrailsDisplayed = (config->Int("HudTrails")) ? true : false;

	EnumStrings::Init();

	// get threads up
	Uint32 numThreads = config->Int("WorkerThreads");
	const int numCores = OS::GetNumCores();
	assert(numCores > 0);
	if (numThreads == 0) numThreads = std::max(Uint32(numCores) - 1, 1U);
	asyncJobQueue.reset(new AsyncJobQueue(numThreads));
	Output("started %d worker threads\n", numThreads);
	syncJobQueue.reset(new SyncJobQueue);

	// XXX early, Lua init needs it
	ShipType::Init();

	// XXX UI requires Lua  but Pi::ui must exist before we start loading
	// templates. so now we have crap everywhere :/
	Lua::Init();

	Pi::ui.Reset(new UI::Context(Lua::manager, Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight()));

	LuaInit();

	// Gui::Init shouldn't initialise any VBOs, since we haven't tested
	// that the capability exists. (Gui does not use VBOs so far)
	Gui::Init(renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), 800, 600);

	UI::Box *box = Pi::ui->VBox(5);
	UI::Label *label = Pi::ui->Label("");
	label->SetFont(UI::Widget::FONT_HEADING_NORMAL);
	UI::Gauge *gauge = Pi::ui->Gauge();
	Pi::ui->GetTopLayer()->SetInnerWidget(
		Pi::ui->Margin(10, UI::Margin::HORIZONTAL)->SetInnerWidget(
			Pi::ui->Expand()->SetInnerWidget(
				Pi::ui->Align(UI::Align::MIDDLE)->SetInnerWidget(
					box->PackEnd(UI::WidgetSet(
						label,
						gauge
					))
				)
			)
		)
    );

	draw_progress(gauge, label, 0.1f);

	s_galaxy = new Galaxy;

	draw_progress(gauge, label, 0.2f);

	s_galaxy->Init();

	draw_progress(gauge, label, 0.3f);

	FaceGenManager::Init();

	draw_progress(gauge, label, 0.4f);

	// Reload home sector, they might have changed, due to custom systems
	// Sectors might be changed in game, so have to re-create them again once we have a Game.
	draw_progress(gauge, label, 0.45f);

	modelCache = new ModelCache(Pi::renderer);
	Shields::Init(Pi::renderer);
	draw_progress(gauge, label, 0.5f);

//unsigned int control_word;
//_clearfp();
//_controlfp_s(&control_word, _EM_INEXACT | _EM_UNDERFLOW | _EM_ZERODIVIDE, _MCW_EM);
//double fpexcept = Pi::timeAccelRates[1] / Pi::timeAccelRates[0];

	draw_progress(gauge, label, 0.6f);

	BaseSphere::Init();
	draw_progress(gauge, label, 0.7f);

	CityOnPlanet::Init();
	draw_progress(gauge, label, 0.8f);

	SpaceStation::Init();
	draw_progress(gauge, label, 0.9f);

	NavLights::Init(Pi::renderer);
	Sfx::Init(Pi::renderer);
	draw_progress(gauge, label, 0.95f);

	if (!no_gui && !config->Int("DisableSound")) {
		Sound::Init();
		Sound::SetMasterVolume(config->Float("MasterVolume"));
		Sound::SetSfxVolume(config->Float("SfxVolume"));
		GetMusicPlayer().SetVolume(config->Float("MusicVolume"));

		Sound::Pause(0);
		if (config->Int("MasterMuted")) Sound::Pause(1);
		if (config->Int("SfxMuted")) Sound::SetSfxVolume(0.f);
		if (config->Int("MusicMuted")) GetMusicPlayer().SetEnabled(false);
	}
	draw_progress(gauge, label, 1.0f);

	OS::NotifyLoadEnd();

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
		for (std::map<std::string, ShipType>::iterator i = ShipType::types.begin();
				i != ShipType::types.end(); ++i)
		{
			const ShipType *shipdef = &(i->second);
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
			double acc1 = shipdef->linThrust[ShipType::THRUSTER_FORWARD] / (9.81*simass);
			double acc2 = shipdef->linThrust[ShipType::THRUSTER_REVERSE] / (9.81*simass);
			double acc3 = shipdef->linThrust[ShipType::THRUSTER_UP] / (9.81*simass);
			double acc4 = shipdef->linThrust[ShipType::THRUSTER_RIGHT] / (9.81*simass);
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

	timer.Stop();
	Output("\n\nLoading took: %lf milliseconds\n", timer.millicycles());
}

bool Pi::IsConsoleActive()
{
	return luaConsole && luaConsole->IsActive();
}

void Pi::Quit()
{
	Projectile::FreeModel();
	delete Pi::intro;
	delete Pi::luaConsole;
	NavLights::Uninit();
	Shields::Uninit();
	Sfx::Uninit();
	Sound::Uninit();
	SpaceStation::Uninit();
	CityOnPlanet::Uninit();
	BaseSphere::Uninit();
	FaceGenManager::Destroy();
	Graphics::Uninit();
	Pi::ui.Reset(0);
	LuaUninit();
	Gui::Uninit();
	delete Pi::modelCache;
	delete Pi::renderer;
	delete Pi::config;
	delete Pi::s_galaxy;
	SDL_Quit();
	FileSystem::Uninit();
	asyncJobQueue.reset();
	syncJobQueue.reset();
	exit(0);
}

void Pi::FlushCaches()
{
	s_galaxy->FlushCaches();
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

	Pi::mouseMotion[0] = Pi::mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			if (Pi::game)
				Pi::EndGame();
			Pi::Quit();
		}

		if (skipTextInput && event.type == SDL_TEXTINPUT) {
			skipTextInput = false;
			continue;
		}
		if (ui->DispatchSDLEvent(event))
			continue;

		bool consoleActive = Pi::IsConsoleActive();
		if (!consoleActive)
			KeyBindings::DispatchSDLEvent(&event);
		else
			KeyBindings::toggleLuaConsole.CheckSDLEventAndDispatch(&event);
		if (consoleActive != Pi::IsConsoleActive()) {
			skipTextInput = true;
			continue;
		}

		if (Pi::IsConsoleActive())
			continue;

		Gui::HandleSDLEvent(&event);

		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					if (Pi::game) {
						// only accessible once game started
						if (currentView != 0) {
							if (currentView != settingsView) {
								Pi::game->SetTimeAccel(Game::TIMEACCEL_PAUSED);
								SetView(settingsView);
							}
							else {
								Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
								SetView(Pi::player->IsDead()
										? static_cast<View*>(deathView)
										: static_cast<View*>(worldView));
							}
						}
					}
					break;
				}
				// special keys. LCTRL+turd
				if ((KeyState(SDLK_LCTRL) || (KeyState(SDLK_RCTRL)))) {
					switch (event.key.keysym.sym) {
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
							Screendump(buf, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
							break;
						}
#if WITH_DEVKEYS
						case SDLK_i: // Toggle Debug info
							Pi::showDebugInfo = !Pi::showDebugInfo;
							break;

#ifdef PIONEER_PROFILER
						case SDLK_p: // alert it that we want to profile
							if (KeyState(SDLK_LSHIFT) || KeyState(SDLK_RSHIFT))
								Pi::doProfileOne = true;
							else {
								Pi::doProfileSlow = !Pi::doProfileSlow;
								Output("slow frame profiling %s\n", Pi::doProfileSlow ? "enabled" : "disabled");
							}
							break;
#endif

						case SDLK_F12:
						{
							if(Pi::game) {
								vector3d dir = -Pi::player->GetOrient().VectorZ();
								/* add test object */
								if (KeyState(SDLK_RSHIFT)) {
									Missile *missile =
										new Missile(ShipType::MISSILE_GUIDED, Pi::player);
									missile->SetOrient(Pi::player->GetOrient());
									missile->SetFrame(Pi::player->GetFrame());
									missile->SetPosition(Pi::player->GetPosition()+50.0*dir);
									missile->SetVelocity(Pi::player->GetVelocity());
									game->GetSpace()->AddBody(missile);
									missile->AIKamikaze(Pi::player->GetCombatTarget());
								} else if (KeyState(SDLK_LSHIFT)) {
									SpaceStation *s = static_cast<SpaceStation*>(Pi::player->GetNavTarget());
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
									if( KeyState(SDLK_LCTRL) )
										ship->AIFlyTo(Pi::player);	// a less lethal option
									else
										ship->AIKill(Pi::player);	// a really lethal option!
									lua_State *l = Lua::manager->GetLuaState();
									pi_lua_import(l, "Equipment");
									LuaTable equip(l, -1);
									LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("laser").Sub("pulsecannon_dual_1mw"));
									LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("misc").Sub("laser_cooling_booster"));
									LuaObject<Ship>::CallMethod<>(ship, "AddEquip", equip.Sub("misc").Sub("atmospheric_shielding"));
									lua_pop(l, 5);
									ship->SetFrame(Pi::player->GetFrame());
									ship->SetPosition(Pi::player->GetPosition()+100.0*dir);
									ship->SetVelocity(Pi::player->GetVelocity());
									ship->UpdateStats();
									game->GetSpace()->AddBody(ship);
								}
							}
							break;
						}
#endif /* DEVKEYS */
#if WITH_OBJECTVIEWER
						case SDLK_F10:
							Pi::SetView(Pi::objectViewerView);
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
							if(Pi::game) {
								if (Pi::game->IsHyperspace())
									Pi::cpan->MsgLog()->Message("", Lang::CANT_SAVE_IN_HYPERSPACE);

								else {
									const std::string name = "_quicksave";
									const std::string path = FileSystem::JoinPath(GetSaveDir(), name);
									try {
										Game::SaveGame(name, Pi::game);
										Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVED_TO + path);
									} catch (CouldNotOpenFileException) {
										Pi::cpan->MsgLog()->Message("", stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path)));
									}
									catch (CouldNotWriteToFileException) {
										Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVE_CANNOT_WRITE);
									}
								}
							}
							break;
						}
						default:
							break; // This does nothing but it stops the compiler warnings
					}
				}
				Pi::keyState[event.key.keysym.sym] = true;
				Pi::keyModState = event.key.keysym.mod;
				Pi::onKeyPress.emit(&event.key.keysym);
				break;
			case SDL_KEYUP:
				Pi::keyState[event.key.keysym.sym] = false;
				Pi::keyModState = event.key.keysym.mod;
				Pi::onKeyRelease.emit(&event.key.keysym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button < COUNTOF(Pi::mouseButton)) {
					Pi::mouseButton[event.button.button] = 1;
					Pi::onMouseButtonDown.emit(event.button.button,
							event.button.x, event.button.y);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button < COUNTOF(Pi::mouseButton)) {
					Pi::mouseButton[event.button.button] = 0;
					Pi::onMouseButtonUp.emit(event.button.button,
							event.button.x, event.button.y);
				}
				break;
			case SDL_MOUSEWHEEL:
				Pi::onMouseWheel.emit(event.wheel.y > 0); // true = up
				break;
			case SDL_MOUSEMOTION:
				Pi::mouseMotion[0] += event.motion.xrel;
				Pi::mouseMotion[1] += event.motion.yrel;
		//		SDL_GetRelativeMouseState(&Pi::mouseMotion[0], &Pi::mouseMotion[1]);
				break;
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
		}
	}
}

void Pi::TombStoneLoop()
{
	std::unique_ptr<Tombstone> tombstone(new Tombstone(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight()));
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	do {
		Pi::HandleEvents();
		Pi::renderer->GetWindow()->SetGrab(false);

		// render the scene
		Pi::BeginRenderTarget();
		Pi::renderer->BeginFrame();
		tombstone->Draw(_time);
		Pi::renderer->EndFrame();
		Gui::Draw();
		Pi::EndRenderTarget();

		Pi::DrawRenderTarget();
		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	} while (!((_time > 2.0) && ((Pi::MouseButtonState(SDL_BUTTON_LEFT)) || Pi::KeyState(SDLK_SPACE)) ));
}

void Pi::InitGame()
{
	// this is a bit brittle. skank may be forgotten and survive between
	// games

	//reset input states
	keyState.clear();
	keyModState = 0;
	std::fill(mouseButton, mouseButton + COUNTOF(mouseButton), 0);
	std::fill(mouseMotion, mouseMotion + COUNTOF(mouseMotion), 0);
	for (std::map<SDL_JoystickID,JoystickState>::iterator stick = joysticks.begin(); stick != joysticks.end(); ++stick) {
		JoystickState &state = stick->second;
		std::fill(state.buttons.begin(), state.buttons.end(), false);
		std::fill(state.hats.begin(), state.hats.end(), 0);
		std::fill(state.axes.begin(), state.axes.end(), 0.f);
	}

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
	cpan->ShowAll();
	DrawGUI = true;
	cpan->SetAlertState(Ship::ALERT_NONE);
	SetView(worldView);

	// fire event before the first frame
	LuaEvent::Queue("onGameStart");
	LuaEvent::Emit();
}

void Pi::Start()
{
	Pi::intro = new Intro(Pi::renderer, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());

	ui->DropAllLayers();
	ui->GetTopLayer()->SetInnerWidget(ui->CallTemplate("MainMenu"));

	Pi::ui->SetMousePointer("icons/cursors/mouse_cursor_2.png", UI::Point(15, 8));

	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(51, 51, 51, 255));

	ui->Layout();

	Uint32 last_time = SDL_GetTicks();
	float _time = 0;

	while (!Pi::game) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				Pi::Quit();
			else
				ui->DispatchSDLEvent(event);

			// XXX hack
			// if we hit our exit conditions then ignore further queued events
			// protects against eg double-click during game generation
			if (Pi::game)
				while (SDL_PollEvent(&event)) {}
		}

		Pi::BeginRenderTarget();
		Pi::renderer->BeginFrame();
		intro->Draw(_time);
		Pi::renderer->EndFrame();

		ui->Update();
		ui->Draw();
		Pi::EndRenderTarget();

		// render the rendertarget texture
		Pi::DrawRenderTarget();
		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	}

	ui->DropAllLayers();
	ui->Layout(); // UI does important things on layout, like updating keyboard shortcuts

	delete Pi::intro; Pi::intro = 0;

	InitGame();
	StartGame();
	MainLoop();
}

void Pi::EndGame()
{
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

	FlushCaches();
	//Faction::SetHomeSectors(); // We might need them to start a new game
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
	char fps_readout[256];
	memset(fps_readout, 0, sizeof(fps_readout));
#endif

	int MAX_PHYSICS_TICKS = Pi::config->Int("MaxPhysicsCyclesPerRender");
	if (MAX_PHYSICS_TICKS <= 0)
		MAX_PHYSICS_TICKS = 4;

	double currentTime = 0.001 * double(SDL_GetTicks());
	double accumulator = Pi::game->GetTimeStep();
	Pi::gameTickAlpha = 0;

	while (Pi::game) {
		PROFILE_SCOPED()

#ifdef PIONEER_PROFILER
		Profiler::reset();
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
			if (pstate == Ship::DOCKED || pstate == Ship::DOCKING) Pi::gameTickAlpha = 1.0;
			else Pi::gameTickAlpha = accumulator / step;

#if WITH_DEVKEYS
			phys_stat += phys_ticks;
#endif
		} else {
			// paused
			PROFILE_SCOPED_RAW("paused")
			BaseSphere::UpdateAllBaseSphereDerivatives();
		}
		frame_stat++;

		// fuckadoodledoo, did the player die?
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
				Pi::deathView->Init();
				Pi::SetView(Pi::deathView);
				time_player_died = Pi::game->GetTime();
			}
		}

		Pi::BeginRenderTarget();

		Pi::renderer->BeginFrame();
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		/* Calculate position for this rendered frame (interpolated between two physics ticks */
        // XXX should this be here? what is this anyway?
		for (Body* b : game->GetSpace()->GetBodies()) {
			b->UpdateInterpTransform(Pi::GetGameTickAlpha());
		}
		game->GetSpace()->GetRootFrame()->UpdateInterpTransform(Pi::GetGameTickAlpha());

		currentView->Update();
		currentView->Draw3D();
		// XXX HandleEvents at the moment must be after view->Draw3D and before
		// Gui::Draw so that labels drawn to screen can have mouse events correctly
		// detected. Gui::Draw wipes memory of label positions.
		Pi::HandleEvents();
		// hide cursor for ship control.

		SetMouseGrab(Pi::MouseButtonState(SDL_BUTTON_RIGHT));

		Pi::renderer->EndFrame();
		if( DrawGUI ) {
			Gui::Draw();
		} else if (game && game->IsNormalSpace()) {
			if (config->Int("DisableScreenshotInfo")==0) {
				const RefCountedPtr<StarSystem> sys = game->GetSpace()->GetStarSystem();
				const SystemPath sp = sys->GetPath();
				std::ostringstream pathStr;

				// fill in pathStr from sp values and sys->GetName()
				static const std::string comma(", ");
				pathStr << Pi::player->GetFrame()->GetLabel() << comma << sys->GetName() << " (" << sp.sectorX << comma << sp.sectorY << comma << sp.sectorZ << ")";

				// display pathStr
				Gui::Screen::EnterOrtho();
				Gui::Screen::PushFont("ConsoleFont");
				Gui::Screen::RenderString(pathStr.str(), 0, 0);
				Gui::Screen::PopFont();
				Gui::Screen::LeaveOrtho();
			}
		}

		// XXX don't draw the UI during death obviously a hack, and still
		// wrong, because we shouldn't this when the HUD is disabled, but
		// probably sure draw it if they switch to eg infoview while the HUD is
		// disabled so we need much smarter control for all this rubbish
		if (Pi::GetView() != Pi::deathView) {
			Pi::ui->Update();
			Pi::ui->Draw();
		}

#if WITH_DEVKEYS
		if (Pi::showDebugInfo) {
			Gui::Screen::EnterOrtho();
			Gui::Screen::PushFont("ConsoleFont");
			Gui::Screen::RenderString(fps_readout, 0, 0);
			Gui::Screen::PopFont();
			Gui::Screen::LeaveOrtho();
		}
#endif

		Pi::EndRenderTarget();
		Pi::DrawRenderTarget();
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
		cpan->Update();
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

			snprintf(
				fps_readout, sizeof(fps_readout),
				"%d fps (%.1f ms/f), %d phys updates, %d triangles, %.3f M tris/sec, %d glyphs/sec\n"
				"Lua mem usage: %d MB + %d KB + %d bytes (stack top: %d)",
				frame_stat, (1000.0/frame_stat), phys_stat, Pi::statSceneTris, Pi::statSceneTris*frame_stat*1e-6,
				Text::TextureFont::GetGlyphCount(),
				lua_memMB, lua_memKB, lua_memB, lua_gettop(Lua::manager->GetLuaState())
			);
			frame_stat = 0;
			phys_stat = 0;
			Text::TextureFont::ClearGlyphCount();
			if (SDL_GetTicks() - last_stats > 1200) last_stats = SDL_GetTicks();
			else last_stats += 1000;
		}
		Pi::statSceneTris = 0;

#ifdef PIONEER_PROFILER
		const Uint32 profTicks = SDL_GetTicks();
		if (Pi::doProfileOne || (Pi::doProfileSlow && (profTicks-newTicks) > 100)) { // slow: < ~10fps
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
	}
}

void Pi::Message(const std::string &message, const std::string &from, enum MsgLevel level)
{
	if (level == MSG_IMPORTANT) {
		Pi::cpan->MsgLog()->ImportantMessage(from, message);
	} else {
		Pi::cpan->MsgLog()->Message(from, message);
	}
}

void Pi::InitJoysticks() {
	int joy_count = SDL_NumJoysticks();
	for (int n = 0; n < joy_count; n++) {
		JoystickState state;

		state.joystick = SDL_JoystickOpen(n);
		if (!state.joystick) {
			Output("SDL_JoystickOpen(%i): %s\n", n, SDL_GetError());
			continue;
		}
		state.axes.resize(SDL_JoystickNumAxes(state.joystick));
		state.buttons.resize(SDL_JoystickNumButtons(state.joystick));
		state.hats.resize(SDL_JoystickNumHats(state.joystick));

		SDL_JoystickID joyID = SDL_JoystickInstanceID(state.joystick);
		joysticks[joyID] = state;
	}
}

int Pi::JoystickButtonState(int joystick, int button) {
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(joysticks.size()))
		return 0;

	if (button < 0 || button >= int(joysticks[joystick].buttons.size()))
		return 0;

	return joysticks[joystick].buttons[button];
}

int Pi::JoystickHatState(int joystick, int hat) {
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(joysticks.size()))
		return 0;

	if (hat < 0 || hat >= int(joysticks[joystick].hats.size()))
		return 0;

	return joysticks[joystick].hats[hat];
}

float Pi::JoystickAxisState(int joystick, int axis) {
	if (!joystickEnabled) return 0;
	if (joystick < 0 || joystick >= int(joysticks.size()))
		return 0;

	if (axis < 0 || axis >= int(joysticks[joystick].axes.size()))
		return 0;

	return joysticks[joystick].axes[axis];
}

void Pi::SetMouseGrab(bool on)
{
	if (!doingMouseGrab && on) {
		Pi::renderer->GetWindow()->SetGrab(true);
		Pi::ui->SetMousePointerEnabled(false);
		doingMouseGrab = true;
	}
	else if(doingMouseGrab && !on) {
		Pi::renderer->GetWindow()->SetGrab(false);
		Pi::ui->SetMousePointerEnabled(true);
		doingMouseGrab = false;
	}
}

float Pi::GetMoveSpeedShiftModifier() {
	// Suggestion: make x1000 speed on pressing both keys?
	if (Pi::KeyState(SDLK_LSHIFT)) return 100.f;
	if (Pi::KeyState(SDLK_RSHIFT)) return 10.f;
	return 1;
}
