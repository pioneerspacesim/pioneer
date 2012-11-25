// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "libs.h"
#include "AmbientSounds.h"
#include "CargoBody.h"
#include "CityOnPlanet.h"
#include "Factions.h"
#include "FileSystem.h"
#include "Frame.h"
#include "GalacticView.h"
#include "Game.h"
#include "GameLoaderSaver.h"
#include "GameMenuView.h"
#include "GeoSphere.h"
#include "Intro.h"
#include "Lang.h"
#include "LmrModel.h"
#include "LuaManager.h"
#include "LuaDev.h"
#include "LuaRef.h"
#include "LuaBody.h"
#include "LuaCargoBody.h"
#include "LuaChatForm.h"
#include "LuaComms.h"
#include "LuaConsole.h"
#include "LuaConstants.h"
#include "LuaEngine.h"
#include "LuaFaction.h"
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
#include "Tombstone.h"
#include "WorldView.h"
#include "DeathView.h"
#include "UIView.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "ui/Context.h"
#include "ui/Lua.h"
#include "gameui/Lua.h"
#include "SDLWrappers.h"
#include "ModManager.h"
#include "graphics/Light.h"
#include "gui/Gui.h"
#include <algorithm>
#include <sstream>

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
DeathView *Pi::deathView;
SpaceStationView *Pi::spaceStationView;
UIView *Pi::infoView;
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
RefCountedPtr<UI::Context> Pi::ui;

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
	LuaFaction::RegisterClass();

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
	LuaDev::Register();
	LuaConsole::Register();

	// XXX sigh
	UI::Lua::Init();
	GameUI::Lua::Init();

	// XXX load everything. for now, just modules
	lua_State *l = Lua::manager->GetLuaState();
	pi_lua_dofile_recursive(l, "libs");
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

const char Pi::SAVE_DIR_NAME[] = "savefiles";

std::string Pi::GetSaveDir()
{
	return FileSystem::JoinPath(FileSystem::GetUserDir(), Pi::SAVE_DIR_NAME);
}

void Pi::Init()
{

	OS::NotifyLoadBegin();

	FileSystem::Init();
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists

	Pi::config = new GameConfig();
	KeyBindings::InitBindings();

	if (config->Int("RedirectStdio"))
		OS::RedirectStdio();

	ModManager::Init();

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
	videoSettings.useTextureCompression = (config->Int("UseTextureCompression") != 0);

	Pi::renderer = Graphics::Init(videoSettings);
	{
		std::ostringstream buf;
		renderer->PrintDebugInfo(buf);

		FILE *f = FileSystem::userFiles.OpenWriteStream("opengl.txt", FileSystem::FileSourceFS::WRITE_TEXT);
		if (!f)
			fprintf(stderr, "Could not open 'opengl.txt'\n");
		const std::string &s = buf.str();
		fwrite(s.c_str(), 1, s.size(), f);
		fclose(f);
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

	// XXX UI requires Lua  but Pi::ui must exist before we start loading
	// templates. so now we have crap everywhere :/
	Lua::Init();

	Pi::ui.Reset(new UI::Context(Lua::manager, Pi::renderer, scrWidth, scrHeight));

	LuaInit();

	// Gui::Init shouldn't initialise any VBOs, since we haven't tested
	// that the capability exists. (Gui does not use VBOs so far)
	Gui::Init(renderer, scrWidth, scrHeight, 800, 600);

	draw_progress(0.1f);

	Galaxy::Init();
	draw_progress(0.2f);

	Faction::Init();
	draw_progress(0.3f);

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

	OS::NotifyLoadEnd();

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

	KeyBindings::toggleManualRotation.onPress.connect(sigc::ptr_fun(&Pi::ToggleManualRotation));

	gameMenuView = new GameMenuView();
	config->Save();
}

void Pi::ToggleManualRotation() {
	Pi::player->SetManualRotationState(!Pi::player->GetManualRotationState());
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
	Pi::ui.Reset(0);
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
	if (currentView) currentView->Detach();
	currentView = v;
	if (currentView) currentView->Attach();
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
		if (event.type == SDL_QUIT) {
			if (Pi::game)
				Pi::EndGame();
			Pi::Quit();
		}
		else if (ui->DispatchSDLEvent(event))
			continue;

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
									const std::string name = "_quicksave";
									GameSaver saver(Pi::game);
									if (saver.SaveToFile(name))
										Pi::cpan->MsgLog()->Message("", Lang::GAME_SAVED_TO + FileSystem::JoinPath(GetSaveDir(), name));
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
		}
	}
}

void Pi::TombStoneLoop()
{
	ScopedPtr<Tombstone> tombstone(new Tombstone(Pi::renderer, GetScrWidth(), GetScrHeight()));
	Uint32 last_time = SDL_GetTicks();
	float _time = 0;
	do {
		Pi::HandleEvents();
		Pi::SetMouseGrab(false);
		Pi::renderer->BeginFrame();
		tombstone->Draw(_time);
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

	//reset input states
	keyModState = 0;
	std::fill(keyState, keyState + COUNTOF(keyState), 0);
	std::fill(mouseButton, mouseButton + COUNTOF(mouseButton), 0);
	std::fill(mouseMotion, mouseMotion + COUNTOF(mouseMotion), 0);
	for (std::vector<JoystickState>::iterator stick = joysticks.begin(); stick != joysticks.end(); ++stick) {
		std::fill(stick->buttons.begin(), stick->buttons.end(), false);
		std::fill(stick->hats.begin(), stick->hats.end(), 0);
		std::fill(stick->axes.begin(), stick->axes.end(), 0.f);
	}

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

void Pi::Start()
{
	Intro *intro = new Intro(Pi::renderer, GetScrWidth(), GetScrHeight());

	ui->SetInnerWidget(ui->CallTemplate("MainMenu"));

	//XXX global ambient colour hack to make explicit the old default ambient colour dependency
	// for some models
	Pi::renderer->SetAmbientColor(Color(0.2f, 0.2f, 0.2f, 1.f));

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

		Pi::renderer->BeginFrame();
		Pi::renderer->SetPerspectiveProjection(75, Pi::GetScrAspect(), 1.f, 10000.f);
		Pi::renderer->SetTransform(matrix4x4f::Identity());
		intro->Draw(_time);
		Pi::renderer->EndFrame();

		ui->Update();
		ui->Draw();

		Pi::renderer->SwapBuffers();

		Pi::frameTime = 0.001f*(SDL_GetTicks() - last_time);
		_time += Pi::frameTime;
		last_time = SDL_GetTicks();
	}

	ui->RemoveInnerWidget();

	InitGame();
	StartGame();
	MainLoop();
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
				Pi::SetView(Pi::deathView);
				Pi::player->Disable();
				time_player_died = Pi::game->GetTime();
			}
		}

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
			accumulator = 0; // fix for huge pauses 10000x -> 1x

		if (!Pi::player->IsDead()) {
			// XXX should this really be limited to while the player is alive?
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

float Pi::CalcHyperspaceRangeMax(int hyperclass, int total_mass_in_tonnes)
{
	// 400.0f is balancing parameter
	return 400.0f * hyperclass * hyperclass / (total_mass_in_tonnes);
}

float Pi::CalcHyperspaceRange(int hyperclass, float total_mass_in_tonnes, int fuel)
{
	const float range_max = CalcHyperspaceRangeMax(hyperclass, total_mass_in_tonnes);
	int fuel_required_max = CalcHyperspaceFuelOut(hyperclass, range_max, range_max);

	if(fuel_required_max <= fuel)
		return range_max;
	else {
		// range is proportional to fuel - use this as first guess
		float range = range_max*fuel/fuel_required_max;

		// if the range is too big due to rounding error, lower it until is is OK.
		while(range > 0 && CalcHyperspaceFuelOut(hyperclass, range, range_max) > fuel)
			range -= 0.05;

		// range is never negative
		range = std::max(range, 0.0f);
		return range;
	}
}

float Pi::CalcHyperspaceDuration(int hyperclass, int total_mass_in_tonnes, float dist)
{
	float hyperspace_range_max = CalcHyperspaceRangeMax(hyperclass, total_mass_in_tonnes);

	// 0.45 is balancing parameter
	return ((dist * dist * 0.45) / (hyperspace_range_max * hyperclass)) *
			(60.0 * 60.0 * 24.0 * sqrtf(total_mass_in_tonnes));
}

float Pi::CalcHyperspaceFuelOut(int hyperclass, float dist, float hyperspace_range_max)
{
	int outFuelRequired = int(ceil(hyperclass*hyperclass*dist / hyperspace_range_max));
	if (outFuelRequired > hyperclass*hyperclass) outFuelRequired = hyperclass*hyperclass;
	if (outFuelRequired < 1) outFuelRequired = 1;

	return outFuelRequired;
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
