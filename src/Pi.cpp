#include "Pi.h"
#include "libs.h"
#include "AmbientSounds.h"
#include "Background.h"
#include "CargoBody.h"
#include "CityOnPlanet.h"
#include "FileSystem.h"
#include "Frame.h"
#include "GalacticView.h"
#include "Game.h"
#include "GameLoaderSaver.h"
#include "GameMenuView.h"
#include "GeoSphere.h"
#include "InfoView.h"
#include "Lang.h"
#include "LmrModel.h"
#include "LuaManager.h"
#include "LuaRef.h"
#include "LuaBody.h"
#include "LuaCargoBody.h"
#include "LuaChatForm.h"
#include "LuaComms.h"
#include "LuaConsole.h"
#include "LuaConstants.h"
#include "LuaEngine.h"
#include "LuaFileSystem.h"
#include "LuaEquipType.h"
#include "LuaFormat.h"
#include "LuaGame.h"
#include "LuaLang.h"
#include "LuaManager.h"
#include "LuaMusic.h"
#include "LuaNameGen.h"
#include "LuaPlanet.h"
#include "LuaPlayer.h"
#include "LuaRand.h"
#include "LuaShip.h"
#include "LuaShipType.h"
#include "LuaSpace.h"
#include "LuaSpace.h"
#include "LuaSpaceStation.h"
#include "LuaStar.h"
#include "LuaStarSystem.h"
#include "LuaSystemBody.h"
#include "LuaSystemPath.h"
#include "LuaTimer.h"
#include "LuaEvent.h"
#include "Missile.h"
#include "ModManager.h"
#include "ObjectViewerView.h"
#include "OS.h"
#include "Planet.h"
#include "Player.h"
#include "Polit.h"
#include "SectorView.h"
#include "Serializer.h"
#include "Sfx.h"
#include "ShipCpanel.h"
#include "ShipType.h"
#include "Sound.h"
#include "SoundMusic.h"
#include "Space.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Star.h"
#include "StringF.h"
#include "SystemInfoView.h"
#include "SystemView.h"
#include "WorldView.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/Light.h"
#include "gui/Gui.h"

#include <fstream>

float Pi::gameTickAlpha;
int Pi::scrWidth;
int Pi::scrHeight;
float Pi::scrAspect;
sigc::signal<void, SDL_keysym*> Pi::onKeyPress;
sigc::signal<void, SDL_keysym*> Pi::onKeyRelease;
sigc::signal<void, int, int, int> Pi::onMouseButtonUp;
sigc::signal<void, int, int, int> Pi::onMouseButtonDown;
sigc::signal<void> Pi::onPlayerChangeTarget;
sigc::signal<void> Pi::onPlayerChangeFlightControlState;
sigc::signal<void> Pi::onPlayerChangeEquipment;
sigc::signal<void, const SpaceStation*> Pi::onDockingClearanceExpired;
LuaSerializer *Pi::luaSerializer;
LuaTimer *Pi::luaTimer;
LuaNameGen *Pi::luaNameGen;
int Pi::keyModState;
char Pi::keyState[SDLK_LAST];
char Pi::mouseButton[6];
int Pi::mouseMotion[2];
bool Pi::doingMouseGrab = false;
Player *Pi::player;
View *Pi::currentView;
WorldView *Pi::worldView;
SpaceStationView *Pi::spaceStationView;
InfoView *Pi::infoView;
SectorView *Pi::sectorView;
GalacticView *Pi::galacticView;
GameMenuView *Pi::gameMenuView;
SystemView *Pi::systemView;
SystemInfoView *Pi::systemInfoView;
ShipCpanel *Pi::cpan;
LuaConsole *Pi::luaConsole;
Game *Pi::game;
MTRand Pi::rng;
float Pi::frameTime;
#if WITH_DEVKEYS
bool Pi::showDebugInfo;
#endif
int Pi::statSceneTris;
GameConfig *Pi::config;
struct DetailLevel Pi::detail = { 0, 0 };
bool Pi::joystickEnabled;
bool Pi::mouseYInvert;
std::vector<Pi::JoystickState> Pi::joysticks;
bool Pi::navTunnelDisplayed;
Gui::Fixed *Pi::menu;
const char * const Pi::combatRating[] = {
	Lang::HARMLESS,
	Lang::MOSTLY_HARMLESS,
	Lang::POOR,
	Lang::AVERAGE,
	Lang::ABOVE_AVERAGE,
	Lang::COMPETENT,
	Lang::DANGEROUS,
	Lang::DEADLY,
	Lang::ELITE
};
Graphics::Renderer *Pi::renderer;

#if WITH_OBJECTVIEWER
ObjectViewerView *Pi::objectViewerView;
#endif

Sound::MusicPlayer Pi::musicPlayer;

int Pi::CombatRating(int kills)
{
	if (kills < 8) return 0;
	if (kills < 16) return 1;
	if (kills < 32) return 2;
	if (kills < 64) return 3;
	if (kills < 128) return 4;
	if (kills < 512) return 5;
	if (kills < 2400) return 6;
	if (kills < 6000) return 7;
	/* nothing better to do with their lives? */
	return 8;
}

static void draw_progress(float progress)
{
	float w, h;
	Pi::renderer->BeginFrame();
	Pi::renderer->EndFrame();
	Gui::Screen::EnterOrtho();
	std::string msg = stringf(Lang::SIMULATING_UNIVERSE_EVOLUTION_N_BYEARS, formatarg("age", progress * 13.7f));
	Gui::Screen::MeasureString(msg, w, h);
	Gui::Screen::RenderString(msg, 0.5f*(Gui::Screen::GetWidth()-w), 0.5f*(Gui::Screen::GetHeight()-h));
	Gui::Screen::LeaveOrtho();
	Pi::renderer->SwapBuffers();
}

static void LuaInit()
{
	Lua::Init();

	lua_State *l = Lua::manager->GetLuaState();

	LuaBody::RegisterClass();
	LuaShip::RegisterClass();
	LuaSpaceStation::RegisterClass();
	LuaPlanet::RegisterClass();
	LuaStar::RegisterClass();
	LuaPlayer::RegisterClass();
	LuaCargoBody::RegisterClass();
	LuaStarSystem::RegisterClass();
	LuaSystemPath::RegisterClass();
	LuaSystemBody::RegisterClass();
	LuaShipType::RegisterClass();
	LuaEquipType::RegisterClass();
	LuaRand::RegisterClass();

	LuaObject<LuaChatForm>::RegisterClass();

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
	LuaMusic::Register();

	LuaConsole::Register();

	// XXX load everything. for now, just modules
	pi_lua_dofile_recursive(l, "libs");
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

std::string Pi::GetSaveDir()
{
	return FileSystem::GetUserDir("savefiles");
}

void Pi::RedirectStdio()
{
	std::string stdout_file = FileSystem::JoinPath(FileSystem::GetUserDir(), "stdout.txt");
	std::string stderr_file = FileSystem::JoinPath(FileSystem::GetUserDir(), "stderr.txt");

	FILE *f;

	f = freopen(stdout_file.c_str(), "w", stdout);
	if (!f)
		f = fopen(stdout_file.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stdout to '%s': %s\n", stdout_file.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stdout = *f;
	}

	f = freopen(stderr_file.c_str(), "w", stderr);
	if (!f)
		f = fopen(stderr_file.c_str(), "w");
	if (!f)
		fprintf(stderr, "ERROR: Couldn't redirect stderr to '%s': %s\n", stderr_file.c_str(), strerror(errno));
	else {
		setvbuf(f, 0, _IOLBF, BUFSIZ);
		*stderr = *f;
	}
}

void Pi::Init()
{
	FileSystem::Init();
	FileSystem::rawFileSystem.MakeDirectory(FileSystem::GetUserDir());

	ModManager::Init();

	Pi::config = new GameConfig(FileSystem::JoinPath(FileSystem::GetUserDir(), "config.ini"));
	KeyBindings::InitBindings();

	if (config->Int("RedirectStdio"))
		RedirectStdio();

	if (!Lang::LoadStrings(config->String("Lang")))
		abort();

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
		OS::Error("SDL initialization failed: %s\n", SDL_GetError());
	}

	// Do rest of SDL video initialization and create Renderer
	Graphics::Settings videoSettings = {};
	videoSettings.width = config->Int("ScrWidth");
	videoSettings.height = config->Int("ScrHeight");
	videoSettings.fullscreen = (config->Int("StartFullscreen") != 0);
	videoSettings.shaders = (config->Int("DisableShaders") == 0);
	videoSettings.requestedSamples = config->Int("AntiAliasingMode");
	videoSettings.vsync = (config->Int("VSync") != 0);

	Pi::renderer = Graphics::Init(videoSettings);
	{
		std::ofstream out;
		out.open((FileSystem::JoinPath(FileSystem::GetUserDir(), "opengl.txt")).c_str());
		renderer->PrintDebugInfo(out);
	}

	OS::LoadWindowIcon();
	SDL_WM_SetCaption("Pioneer","Pioneer");

	Pi::scrWidth = videoSettings.width;
	Pi::scrHeight = videoSettings.height;
	Pi::scrAspect = videoSettings.width / float(videoSettings.height);

	Pi::rng.seed(time(0));

	InitJoysticks();
	joystickEnabled = (config->Int("EnableJoystick")) ? true : false;
	mouseYInvert = (config->Int("InvertMouseY")) ? true : false;

	navTunnelDisplayed = (config->Int("DisplayNavTunnel")) ? true : false;

	Gui::Init(renderer, scrWidth, scrHeight, 800, 600);

	LuaInit();

	draw_progress(0.1f);

	Galaxy::Init();
	draw_progress(0.2f);

	CustomSystem::Init();
	draw_progress(0.4f);

	LmrModelCompilerInit(Pi::renderer);
	LmrNotifyScreenWidth(Pi::scrWidth);
	draw_progress(0.5f);

//unsigned int control_word;
//_clearfp();
//_controlfp_s(&control_word, _EM_INEXACT | _EM_UNDERFLOW | _EM_ZERODIVIDE, _MCW_EM);
//double fpexcept = Pi::timeAccelRates[1] / Pi::timeAccelRates[0];

	ShipType::Init();
	draw_progress(0.6f);

	GeoSphere::Init();
	draw_progress(0.7f);

	CityOnPlanet::Init();
	draw_progress(0.8f);

	SpaceStation::Init();
	draw_progress(0.9f);

	Sfx::Init(Pi::renderer);
	draw_progress(0.95f);

	if (!config->Int("DisableSound")) {
		Sound::Init();
		Sound::SetMasterVolume(config->Float("MasterVolume"));
		Sound::SetSfxVolume(config->Float("SfxVolume"));
		GetMusicPlayer().SetVolume(config->Float("MusicVolume"));

		Sound::Pause(0);
		if (config->Int("MasterMuted")) Sound::Pause(1);
		if (config->Int("SfxMuted")) Sound::SetSfxVolume(0.f);
		if (config->Int("MusicMuted")) GetMusicPlayer().SetEnabled(false);
	}
	draw_progress(1.0f);

#if 0
	// test code to produce list of ship stats

	FILE *pStatFile = fopen("shipstat.csv","wt");
	if (pStatFile)
	{
		fprintf(pStatFile, "name,lmrname,hullmass,capacity,fakevol,rescale,xsize,ysize,zsize,facc,racc,uacc,sacc,aacc\n");
		for (std::map<std::string, ShipType>::iterator i = ShipType::types.begin();
				i != ShipType::types.end(); ++i)
		{
			ShipType *shipdef = &(i->second);
			LmrModel *lmrModel = LmrLookupModelByName(shipdef->lmrModelName.c_str());
			LmrObjParams lmrParams; memset(&lmrParams, 0, sizeof(LmrObjParams));
			LmrCollMesh *collMesh = new LmrCollMesh(lmrModel, &lmrParams);
			Aabb aabb = collMesh->GetAabb();

			double hullmass = shipdef->hullMass;
			double capacity = shipdef->capacity;
			double xsize = aabb.max.x-aabb.min.x;
			double ysize = aabb.max.y-aabb.min.y;
			double zsize = aabb.max.z-aabb.min.z;
			double fakevol = xsize*ysize*zsize;
			double rescale = pow(fakevol/(100 * (hullmass+capacity)), 0.3333333333);
			double brad = aabb.GetBoundingRadius();
			double simass = (hullmass + capacity) * 1000.0;
			double angInertia = (2/5.0)*simass*brad*brad;
			double acc1 = shipdef->linThrust[ShipType::THRUSTER_FORWARD] / (9.81*simass);
			double acc2 = shipdef->linThrust[ShipType::THRUSTER_REVERSE] / (9.81*simass);
			double acc3 = shipdef->linThrust[ShipType::THRUSTER_UP] / (9.81*simass);
			double acc4 = shipdef->linThrust[ShipType::THRUSTER_RIGHT] / (9.81*simass);
			double acca = shipdef->angThrust/angInertia;

			fprintf(pStatFile, "%s,%s,%.1f,%.1f,%.1f,%.3f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%f\n",
				shipdef->name.c_str(), shipdef->lmrModelName.c_str(), hullmass, capacity,
				fakevol, rescale, xsize, ysize, zsize, acc1, acc2, acc3, acc4, acca);
			delete collMesh;
		}
		fclose(pStatFile);
	}
#endif

	luaConsole = new LuaConsole(10);
	KeyBindings::toggleLuaConsole.onPress.connect(sigc::ptr_fun(&Pi::ToggleLuaConsole));

	gameMenuView = new GameMenuView();
	config->Save();
}

bool Pi::IsConsoleActive()
{
	return luaConsole && luaConsole->IsActive();
}

void Pi::ToggleLuaConsole()
{
	if (luaConsole->IsVisible()) {
		luaConsole->Hide();
		if (luaConsole->GetTextEntryField()->IsFocused())
			Gui::Screen::ClearFocus();
		Gui::Screen::RemoveBaseWidget(luaConsole);
	} else {
		// luaConsole is added and removed from the base widget set
		// (rather than just using Show()/Hide())
		// so that it's forced in front of any other base widgets when it opens
		Gui::Screen::AddBaseWidget(luaConsole, 0, 0);
		luaConsole->Show();
		luaConsole->GetTextEntryField()->Show();
	}
}

void Pi::Quit()
{
	delete Pi::gameMenuView;
	delete Pi::luaConsole;
	Sfx::Uninit();
	Sound::Uninit();
	SpaceStation::Uninit();
	CityOnPlanet::Uninit();
	GeoSphere::Uninit();
	LmrModelCompilerUninit();
	Galaxy::Uninit();
	Graphics::Uninit();
	LuaUninit();
	Gui::Uninit();
	delete Pi::renderer;
	StarSystem::ShrinkCache();
	SDL_Quit();
	FileSystem::Uninit();
	exit(0);
}

void Pi::BoinkNoise()
{
	Sound::PlaySfx("Click", 0.3f, 0.3f, false);
}

void Pi::SetView(View *v)
{
	if (cpan)
		cpan->ClearOverlay();
	if (currentView) currentView->HideAll();
	currentView = v;
	if (currentView) {
		currentView->OnSwitchTo();
		currentView->ShowAll();
	}
}

void Pi::OnChangeDetailLevel()
{
	GeoSphere::OnChangeDetailLevel();
}

void Pi::HandleEvents()
{
	SDL_Event event;

	Pi::mouseMotion[0] = Pi::mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		KeyBindings::DispatchSDLEvent(&event);

		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					if (Pi::game) {
						// only accessible once game started
						if (currentView != 0) {
							if (currentView != gameMenuView) {
								Pi::game->SetTimeAccel(Game::TIMEACCEL_PAUSED);
								SetView(gameMenuView);
							}
							else {
								Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
								SetView(worldView);
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
						case SDLK_PRINT:	   // print
						case SDLK_KP_MULTIPLY: // screen
						{
							char buf[256];
							const time_t t = time(0);
							struct tm *_tm = localtime(&t);
							strftime(buf, sizeof(buf), "screenshot-%Y%m%d-%H%M%S.png", _tm);
							Screendump(buf, GetScrWidth(), GetScrHeight());
							break;
						}
#if WITH_DEVKEYS
						case SDLK_i: // Toggle Debug info
							Pi::showDebugInfo = !Pi::showDebugInfo;
							break;
						case SDLK_m:  // Gimme money!
							if(Pi::game) {
								Pi::player->SetMoney(Pi::player->GetMoney() + 10000000);
							}
							break;
						case SDLK_F12:
						{
							if(Pi::game) {
								matrix4x4d m; Pi::player->GetRotMatrix(m);
								vector3d dir = m*vector3d(0,0,-1);
								/* add test object */
								if (KeyState(SDLK_RSHIFT)) {
									Missile *missile =
										new Missile(ShipType::MISSILE_GUIDED, Pi::player, Pi::player->GetCombatTarget());
									missile->SetRotMatrix(m);
									missile->SetFrame(Pi::player->GetFrame());
									missile->SetPosition(Pi::player->GetPosition()+50.0*dir);
									missile->SetVelocity(Pi::player->GetVelocity());
									game->GetSpace()->AddBody(missile);
								} else if (KeyState(SDLK_LSHIFT)) {
									SpaceStation *s = static_cast<SpaceStation*>(Pi::player->GetNavTarget());
									if (s) {
										int port = s->GetFreeDockingPort();
										if (port != -1) {
											printf("Putting ship into station\n");
											// Make police ship intent on killing the player
											Ship *ship = new Ship(ShipType::LADYBIRD);
											ship->AIKill(Pi::player);
											ship->SetFrame(Pi::player->GetFrame());
											ship->SetDockedWith(s, port);
											game->GetSpace()->AddBody(ship);
										} else {
											printf("No docking ports free dude\n");
										}
									} else {
											printf("Select a space station...\n");
									}
								} else {
									Ship *ship = new Ship(ShipType::LADYBIRD);
									ship->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::PULSECANNON_1MW);
									ship->AIKill(Pi::player);
									ship->SetFrame(Pi::player->GetFrame());
									ship->SetPosition(Pi::player->GetPosition()+100.0*dir);
									ship->SetVelocity(Pi::player->GetVelocity());
									ship->m_equipment.Add(Equip::DRIVE_CLASS2);
									ship->m_equipment.Add(Equip::RADAR_MAPPER);
									ship->m_equipment.Add(Equip::SCANNER);
									ship->m_equipment.Add(Equip::SHIELD_GENERATOR);
									ship->m_equipment.Add(Equip::HYDROGEN, 10);
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
									std::string name = FileSystem::JoinPath(GetSaveDir(), "_quicksave");
									GameSaver saver(Pi::game);
									if (saver.SaveToFile(name))
										Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVED_TO+name);
								}
							}
							break;
						}
						default:
							break; // This does nothing but it stops the compiler warnings
					}
				}
				Pi::keyState[event.key.keysym.sym] = 1;
				Pi::keyModState = event.key.keysym.mod;
				Pi::onKeyPress.emit(&event.key.keysym);
				break;
			case SDL_KEYUP:
				Pi::keyState[event.key.keysym.sym] = 0;
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
			case SDL_MOUSEMOTION:
				Pi::mouseMotion[0] += event.motion.xrel;
				Pi::mouseMotion[1] += event.motion.yrel;
		//		SDL_GetRelativeMouseState(&Pi::mouseMotion[0], &Pi::mouseMotion[1]);
				break;
			case SDL_JOYAXISMOTION:
				if (joysticks[event.jaxis.which].joystick == NULL)
					break;
				if (event.jaxis.value == -32768)
					joysticks[event.jaxis.which].axes[event.jaxis.axis] = 1.f;
				else
					joysticks[event.jaxis.which].axes[event.jaxis.axis] = -event.jaxis.value / 32767.f;
				break;
			case SDL_JOYBUTTONUP:
			case SDL_JOYBUTTONDOWN:
				if (joysticks[event.jaxis.which].joystick == NULL)
					break;
				joysticks[event.jbutton.which].buttons[event.jbutton.button] = event.jbutton.state != 0;
				break;
			case SDL_JOYHATMOTION:
				if (joysticks[event.jaxis.which].joystick == NULL)
					break;
				joysticks[event.jhat.which].hats[event.jhat.hat] = event.jhat.value;
				break;
			case SDL_QUIT:
				if (Pi::game)
					Pi::EndGame();
				Pi::Quit();
				break;
		}
	}
}

static void draw_intro(Background::Container *background, float _time)
{
	LmrObjParams params = {
		"ShipAnimation", // animation namespace
		0.0, // time
		{ }, // animation stages
		{ 0.0, 1.0 }, // animation positions
		Lang::PIONEER, // label
		0, // equipment
		Ship::FLYING, // flightState
		{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, // thrust
		{	// pColor[3]
		{ { .2f, .2f, .5f, 1.0f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
		{ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.8f, 0.8f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
	};
	EquipSet equipment;
	// The finest parts that money can buy!
	params.equipment = &equipment;
	equipment.Add(Equip::ECM_ADVANCED, 1);
	equipment.Add(Equip::HYPERCLOUD_ANALYZER, 1);
	equipment.Add(Equip::ATMOSPHERIC_SHIELDING, 1);
	equipment.Add(Equip::FUEL_SCOOP, 1);
	equipment.Add(Equip::SCANNER, 1);
	equipment.Add(Equip::RADAR_MAPPER, 1);
	equipment.Add(Equip::MISSILE_NAVAL, 4);

	// XXX all this stuff will be gone when intro uses a Camera
	// rotate background by time, and a bit extra Z so it's not so flat
	matrix4x4d brot = matrix4x4d::RotateXMatrix(-0.25*_time) * matrix4x4d::RotateZMatrix(0.6);
	background->Draw(Pi::renderer, brot);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	const Color oldSceneAmbientColor = Pi::renderer->GetAmbientColor();
	Pi::renderer->SetAmbientColor(Color(0.1f, 0.1f, 0.1f, 1.f));

	const Color lc(1.f, 1.f, 1.f, 0.f);
	const Graphics::Light light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 1.f, 1.f), lc, lc, lc);
	Pi::renderer->SetLights(1, &light);

	matrix4x4f rot = matrix4x4f::RotateYMatrix(_time) * matrix4x4f::RotateZMatrix(0.6f*_time) *
			matrix4x4f::RotateXMatrix(_time*0.7f);
	rot[14] = -80.0;
	LmrLookupModelByName("lanner_ub")->Render(rot, &params);
	glPopAttrib();
	Pi::renderer->SetAmbientColor(oldSceneAmbientColor);
}

static void draw_tombstone(float _time)
{
	LmrObjParams params = {
		0, // animation namespace
		0.0, // time
		{}, // animation stages
		{}, // animation positions
		Lang::TOMBSTONE_EPITAPH, // label
		0, // equipment
		0, // flightState
		{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },
		{	// pColor[3]
		{ { 1.0f, 1.0f, 1.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.6f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
	};
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	const Color oldSceneAmbientColor = Pi::renderer->GetAmbientColor();
	Pi::renderer->SetAmbientColor(Color(0.1f, 0.1f, 0.1f, 1.f));

	const Color lc(1.f, 1.f, 1.f, 0.f);
	const Graphics::Light light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 1.f, 1.f), lc, lc, lc);
	Pi::renderer->SetLights(1, &light);

	matrix4x4f rot = matrix4x4f::RotateYMatrix(_time*2);
	rot[14] = -std::max(150.0f - 30.0f*_time, 30.0f);
	LmrLookupModelByName("tombstone")->Render(rot, &params);
	glPopAttrib();
	Pi::renderer->SetAmbientColor(oldSceneAmbientColor);
}

void Pi::TombStoneLoop()
{
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	cpan->HideAll();
	currentView->HideAll();
	do {
		Pi::renderer->BeginFrame();
		Pi::renderer->SetPerspectiveProjection(75, Pi::GetScrAspect(), 1.f, 10000.f);
		Pi::renderer->SetTransform(matrix4x4f::Identity());
		Pi::HandleEvents();
		Pi::SetMouseGrab(false);
		draw_tombstone(_time);
		Pi::renderer->EndFrame();
		Gui::Draw();
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

	Polit::Init();

	if (!config->Int("DisableSound")) AmbientSounds::Init();

	LuaInitGame();
}

static void OnPlayerDockOrUndock()
{
	Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
	Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
}

static void OnPlayerChangeEquipment(Equip::Type e)
{
	Pi::onPlayerChangeEquipment.emit();
}

void Pi::StartGame()
{
	Pi::player->onDock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->onUndock.connect(sigc::ptr_fun(&OnPlayerDockOrUndock));
	Pi::player->m_equipment.onChange.connect(sigc::ptr_fun(&OnPlayerChangeEquipment));
	cpan->ShowAll();
	cpan->SetAlertState(Ship::ALERT_NONE);
	OnPlayerChangeEquipment(Equip::NONE);
	SetView(worldView);

	// fire event before the first frame
	LuaEvent::Queue("onGameStart");
	LuaEvent::Emit();
}

bool Pi::menuDone = false;
void Pi::HandleMenuKey(int n)
{
	switch (n) {

		// XXX these assign to Pi::game, which is the correct behaviour. its
		// redundant right now because the Game constructor assigns itself to
		// Pi::game. it only does that as a hack to get the views up and
		// running. one day, when all that is fixed, you can delete this
		// comment

		case 0: // Earth start point
		{
			game = new Game(SystemPath(0,0,0,0,9));  // Los Angeles, Earth
			break;
		}

		case 1: // Epsilon Eridani start point
		{
			game = new Game(SystemPath(1,-1,-1,0,4));  // New Hope, New Hope
			break;
		}

		case 2: // Lave start point
		{
			game = new Game(SystemPath(-2,1,90,0,2));  // Lave Station, Lave
			break;
		}

		case 3: // Debug start point
		{
			game = new Game(SystemPath(1,-1,-1,0,4), vector3d(0,2*EARTH_RADIUS,0));  // somewhere over New Hope

			Ship *enemy = new Ship(ShipType::EAGLE_LRF);
			enemy->SetFrame(player->GetFrame());
			enemy->SetPosition(player->GetPosition()+vector3d(0,0,-9000.0));
			enemy->SetVelocity(vector3d(0,0,0));
			enemy->m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS1);
			enemy->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::PULSECANNON_1MW);
			enemy->m_equipment.Add(Equip::HYDROGEN, 2);
			enemy->m_equipment.Add(Equip::ATMOSPHERIC_SHIELDING);
			enemy->m_equipment.Add(Equip::AUTOPILOT);
			enemy->m_equipment.Add(Equip::SCANNER);
			enemy->UpdateStats();
			enemy->AIKill(player);
			game->GetSpace()->AddBody(enemy);

			player->SetCombatTarget(enemy);

			const ShipType *shipdef;
			double mass, acc1, acc2, acc3;
			printf("Player ship mass = %.0fkg, Enemy ship mass = %.0fkg\n",
				   player->GetMass(), enemy->GetMass());

			shipdef = &player->GetShipType();
			mass = player->GetMass();
			acc1 = shipdef->linThrust[ShipType::THRUSTER_FORWARD] / (9.81*mass);
			acc2 = shipdef->linThrust[ShipType::THRUSTER_REVERSE] / (9.81*mass);
			acc3 = shipdef->linThrust[ShipType::THRUSTER_UP] / (9.81*mass);
			printf("Player ship thrust = %.1fg, %.1fg, %.1fg\n", acc1, acc2, acc3);

			shipdef = &enemy->GetShipType();
			mass = enemy->GetMass();
			acc1 = shipdef->linThrust[ShipType::THRUSTER_FORWARD] / (9.81*mass);
			acc2 = shipdef->linThrust[ShipType::THRUSTER_REVERSE] / (9.81*mass);
			acc3 = shipdef->linThrust[ShipType::THRUSTER_UP] / (9.81*mass);
			printf("Enemy ship thrust = %.1fg, %.1fg, %.1fg\n", acc1, acc2, acc3);

			/*	Frame *stationFrame = new Frame(pframe, "Station frame...");
			 stationFrame->SetRadius(5000);
			 stationFrame->m_sbody = 0;
			 stationFrame->SetPosition(vector3d(0,0,zpos));
			 stationFrame->SetAngVelocity(vector3d(0,0,0.5));

			 for (int i=0; i<4; i++) {
			 Ship *body = new Ship(ShipType::LADYBIRD);
			 char buf[64];
			 snprintf(buf,sizeof(buf),"X%c-0%02d", 'A'+i, i);
			 body->SetLabel(buf);
			 body->SetFrame(stationFrame);
			 body->SetPosition(vector3d(200*(i+1), 0, 2000));
			 Space::AddBody(body);
			 }

			 SpaceStation *station = new SpaceStation(SpaceStation::JJHOOP);
			 station->SetLabel("Poemi-chan's Folly");
			 station->SetFrame(stationFrame);
			 station->SetPosition(vector3d(0,0,0));
			 Space::AddBody(station);

			 SpaceStation *station2 = new SpaceStation(SpaceStation::GROUND_FLAVOURED);
			 station2->SetLabel("Conor's End");
			 station2->SetFrame(*pframe->m_children.begin()); // rotating frame of planet
			 station2->OrientOnSurface(EARTH_RADIUS, M_PI/4, M_PI/4);
			 Space::AddBody(station2);
			 */
			//	player->SetDockedWith(station2, 0);

			break;
		}

		case 4: // Load game
		{
			GameLoader loader;
			loader.DialogMainLoop();
			game = loader.GetGame();
			if (! game) {
				// loading screen was cancelled;
				// return without setting menuDone so the menu is re-displayed
				return;
			}
			break;
		}

		case 5: // Settings
		{
			Gui::Screen::RemoveBaseWidget(menu);
			SetView(Pi::gameMenuView);
			while (Pi::GetView() == Pi::gameMenuView) Gui::MainLoopIteration();
			SetView(0);
			Gui::Screen::AddBaseWidget(menu, 0, 0);
			return;
		}

		default:
			break;
	}

	menuDone = true;
}

void Pi::Start()
{
	Background::Container *background = new Background::Container(Pi::renderer, UNIVERSE_SEED);

	menu = new Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
	Gui::Screen::AddBaseWidget(menu, 0, 0);
	menu->SetTransparency(true);

	static const float badgeWidth = 128;
	float badgeSize[2];
	Gui::Screen::GetCoords2Pixels(badgeSize);
	badgeSize[0] *= badgeWidth; badgeSize[1] *= badgeWidth;
	Gui::Fixed *badge = new Gui::Fixed(badgeSize[0], badgeSize[1]);
	badge->Add(new Gui::Image("icons/badge.png"),0,0);
	menu->Add(badge, 30, Gui::Screen::GetHeight()-badgeSize[1]-30);

	Gui::Screen::PushFont("OverlayFont");

	const float w = Gui::Screen::GetWidth() / 2.0f;
	const float h = Gui::Screen::GetHeight() / 2.0f;
	const int OPTS = 7;
	Gui::SolidButton *opts[OPTS];
	opts[0] = new Gui::SolidButton(); opts[0]->SetShortcut(SDLK_1, KMOD_NONE);
	opts[0]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 0));
	opts[1] = new Gui::SolidButton(); opts[1]->SetShortcut(SDLK_2, KMOD_NONE);
	opts[1]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 1));
	opts[2] = new Gui::SolidButton(); opts[2]->SetShortcut(SDLK_3, KMOD_NONE);
	opts[2]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 2));
	opts[3] = new Gui::SolidButton(); opts[3]->SetShortcut(SDLK_4, KMOD_NONE);
	opts[3]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 3));
	opts[4] = new Gui::SolidButton(); opts[4]->SetShortcut(SDLK_5, KMOD_NONE);
	opts[4]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 4));
	opts[5] = new Gui::SolidButton(); opts[5]->SetShortcut(SDLK_6, KMOD_NONE);
	opts[5]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 5));
	opts[6] = new Gui::SolidButton(); opts[6]->SetShortcut(SDLK_7, KMOD_NONE);
	opts[6]->onClick.connect(sigc::bind(sigc::ptr_fun(&Pi::HandleMenuKey), 6));
	menu->Add(opts[0], w, h-96);
	menu->Add(new Gui::Label(Lang::MM_START_NEW_GAME_EARTH), w+32, h-96);
	menu->Add(opts[1], w, h-64);
	menu->Add(new Gui::Label(Lang::MM_START_NEW_GAME_E_ERIDANI), w+32, h-64);
	menu->Add(opts[2], w, h-32);
	menu->Add(new Gui::Label(Lang::MM_START_NEW_GAME_LAVE), w+32, h-32);
	menu->Add(opts[3], w, h);
	menu->Add(new Gui::Label(Lang::MM_START_NEW_GAME_DEBUG), w+32, h);
	menu->Add(opts[4], w, h+32);
	menu->Add(new Gui::Label(Lang::MM_LOAD_SAVED_GAME), w+32, h+32);
	menu->Add(opts[5], w, h+64);
	menu->Add(new Gui::Label(Lang::MM_SETTINGS), w+32, h+64);
	menu->Add(opts[6], w, h+96);
	menu->Add(new Gui::Label(Lang::MM_QUIT), w+32, h+96);

	std::string version("Pioneer " PIONEER_VERSION);
	if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
	version += "\n";
	version += Pi::renderer->GetName();

	menu->Add(new Gui::Label(version), 30+badgeSize[0]+20, Gui::Screen::GetHeight()-badgeSize[1]-10);

	Gui::Screen::PopFont();

	menu->ShowAll();

	Uint32 last_time = SDL_GetTicks();
	float _time = 0;

	menuDone = false;
	game = 0;

	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(0.2f, 0.2f, 0.2f, 1.f));

	while (!menuDone) {
		Pi::HandleEvents();
		Pi::renderer->BeginFrame();
		Pi::renderer->SetPerspectiveProjection(75, Pi::GetScrAspect(), 1.f, 10000.f);
		Pi::renderer->SetTransform(matrix4x4f::Identity());
		Pi::SetMouseGrab(false);
		draw_intro(background, _time);
		Pi::renderer->EndFrame();
		Gui::Draw();
		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	}
	menu->HideAll();

	Gui::Screen::RemoveBaseWidget(menu);
	delete menu;
	delete background;

	// game is set by HandleMenuKey if any game-starting option (start or
	// load) is selected
	if (game) {
		InitGame();
		StartGame();
		MainLoop();
	}

	// no game means quit was selected, so end things
	else
		Pi::Quit();
}

void Pi::EndGame()
{
	Pi::musicPlayer.Stop();
	Sound::DestroyAllEvents();

	// final event
	LuaEvent::Queue("onGameEnd");
	LuaEvent::Emit();

	Lua::manager->CollectGarbage();

	if (!config->Int("DisableSound")) AmbientSounds::Uninit();
	Sound::DestroyAllEvents();

	assert(game);
	delete game;
	game = 0;
	player = 0;

	StarSystem::ShrinkCache();
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
		double newTime = 0.001 * double(SDL_GetTicks());
		Pi::frameTime = newTime - currentTime;
		if (Pi::frameTime > 0.25) Pi::frameTime = 0.25;
		currentTime = newTime;
		accumulator += Pi::frameTime * Pi::game->GetTimeAccelRate();

		const float step = Pi::game->GetTimeStep();
		if (step > 0.0f) {
			int phys_ticks = 0;
			while (accumulator >= step) {
				if (++phys_ticks >= MAX_PHYSICS_TICKS) {
					accumulator = 0.0;
					break;
				}
				game->TimeStep(step);

				accumulator -= step;
			}
			Pi::gameTickAlpha = accumulator / step;

#if WITH_DEVKEYS
			phys_stat += phys_ticks;
#endif
		} else {
			// paused
		}
		frame_stat++;

		Pi::renderer->BeginFrame();
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		/* Calculate position for this rendered frame (interpolated between two physics ticks */
        // XXX should this be here? what is this anyway?
		for (Space::BodyIterator i = game->GetSpace()->BodiesBegin(); i != game->GetSpace()->BodiesEnd(); ++i) {
			(*i)->UpdateInterpolatedTransform(Pi::GetGameTickAlpha());
		}
		game->GetSpace()->GetRootFrame()->UpdateInterpolatedTransform(Pi::GetGameTickAlpha());

		currentView->Update();
		currentView->Draw3D();
		// XXX HandleEvents at the moment must be after view->Draw3D and before
		// Gui::Draw so that labels drawn to screen can have mouse events correctly
		// detected. Gui::Draw wipes memory of label positions.
		Pi::HandleEvents();
		// hide cursor for ship control.

		SetMouseGrab(Pi::MouseButtonState(SDL_BUTTON_RIGHT));

		Pi::renderer->EndFrame();
		Gui::Draw();

#if WITH_DEVKEYS
		if (Pi::showDebugInfo) {
			Gui::Screen::EnterOrtho();
			Gui::Screen::PushFont("ConsoleFont");
			Gui::Screen::RenderString(fps_readout, 0, 0);
			Gui::Screen::PopFont();
			Gui::Screen::LeaveOrtho();
		}
#endif

		Pi::renderer->SwapBuffers();

		// game exit or failed load from GameMenuView will have cleared
		// Pi::game. we can't continue.
		if (!Pi::game)
			return;

		if (Pi::game->UpdateTimeAccel())
			accumulator = 0;				// fix for huge pauses 10000x -> 1x

		// fuckadoodledoo, did the player die?
		if (Pi::player->IsDead()) {
			if (time_player_died > 0.0) {
				if (Pi::game->GetTime() - time_player_died > 8.0) {
					Pi::TombStoneLoop();
					Pi::EndGame();
					break;
				}
			} else {
				Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
				Pi::cpan->HideAll();
				Pi::SetView(static_cast<View*>(Pi::worldView));
				Pi::player->Disable();
				time_player_died = Pi::game->GetTime();
			}
		} else {
			// this is something we need not do every turn...
			if (!config->Int("DisableSound")) AmbientSounds::Update();
			StarSystem::ShrinkCache();
		}
		cpan->Update();
		musicPlayer.Update();

#if WITH_DEVKEYS
		if (Pi::showDebugInfo && SDL_GetTicks() - last_stats > 1000) {
			size_t lua_mem = Lua::manager->GetMemoryUsage();
			int lua_memB = int(lua_mem & ((1u << 10) - 1));
			int lua_memKB = int(lua_mem >> 10) % 1024;
			int lua_memMB = int(lua_mem >> 20);

			Pi::statSceneTris += LmrModelGetStatsTris();

			snprintf(
				fps_readout, sizeof(fps_readout),
				"%d fps (%.1f ms/f), %d phys updates, %d triangles, %.3f M tris/sec, %d terrain vtx/sec, %d glyphs/sec\n"
				"Lua mem usage: %d MB + %d KB + %d bytes",
				frame_stat, (1000.0/frame_stat), phys_stat, Pi::statSceneTris, Pi::statSceneTris*frame_stat*1e-6,
				GeoSphere::GetVtxGenCount(), Text::TextureFont::GetGlyphCount(),
				lua_memMB, lua_memKB, lua_memB
			);
			frame_stat = 0;
			phys_stat = 0;
			Text::TextureFont::ClearGlyphCount();
			GeoSphere::ClearVtxGenCount();
			if (SDL_GetTicks() - last_stats > 1200) last_stats = SDL_GetTicks();
			else last_stats += 1000;
		}
		Pi::statSceneTris = 0;
		LmrModelClearStatsTris();
#endif

#ifdef MAKING_VIDEO
		if (SDL_GetTicks() - last_screendump > 50) {
			last_screendump = SDL_GetTicks();
			std::string fname = stringf(Lang::SCREENSHOT_FILENAME_TEMPLATE, formatarg("index", dumpnum++));
			Screendump(fname.c_str(), GetScrWidth(), GetScrHeight());
		}
#endif /* MAKING_VIDEO */
	}
}

float Pi::CalcHyperspaceRange(int hyperclass, int total_mass_in_tonnes)
{
	// for the sake of hyperspace range, we count ships mass as 60% of original.
	// Brian: "The 60% value was arrived at through trial and error,
	// to scale the entire jump range calculation after things like ship mass,
	// cargo mass, hyperdrive class, fuel use and fun were factored in."
	return 200.0f * hyperclass * hyperclass / (total_mass_in_tonnes * 0.6f);
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
		JoystickState *state;
		joysticks.push_back(JoystickState());
		state = &joysticks.back();

		state->joystick = SDL_JoystickOpen(n);
		if (state->joystick == NULL) {
			fprintf(stderr, "SDL_JoystickOpen(%i): %s\n", n, SDL_GetError());
			continue;
		}

		state->axes.resize(SDL_JoystickNumAxes(state->joystick));
		state->buttons.resize(SDL_JoystickNumButtons(state->joystick));
		state->hats.resize(SDL_JoystickNumHats(state->joystick));
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
		SDL_ShowCursor(0);
		SDL_WM_GrabInput(SDL_GRAB_ON);
//		SDL_SetRelativeMouseMode(true);
		doingMouseGrab = true;
	}
	else if(doingMouseGrab && !on) {
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
//		SDL_SetRelativeMouseMode(false);
		doingMouseGrab = false;
	}
}

float Pi::GetMoveSpeedShiftModifier() {
	// Suggestion: make x1000 speed on pressing both keys?
	if (Pi::KeyState(SDLK_LSHIFT)) return 100.f;
	if (Pi::KeyState(SDLK_RSHIFT)) return 10.f;
	return 1;
}
