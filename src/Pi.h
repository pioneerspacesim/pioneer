// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PI_H
#define _PI_H

#include "utils.h"
#include "gui/Gui.h"
#include "mtrand.h"
#include "gameconsts.h"
#include "GameConfig.h"
#include "LuaSerializer.h"
#include "LuaTimer.h"
#include "CargoBody.h"
#include "Space.h"
#include <map>
#include <string>
#include <vector>

class Player;
class View;
class SectorView;
class SystemView;
class WorldView;
class DeathView;
class SystemInfoView;
class ShipCpanel;
class StarSystem;
class SpaceStationView;
class SpaceStation;
class GalacticView;
class UIView;
class Ship;
class GameMenuView;
class LuaConsole;
class LuaNameGen;
class ModelCache;
class ModelBase;
namespace Graphics { class Renderer; }
namespace Sound { class MusicPlayer; }
namespace UI { class Context; }

#if WITH_OBJECTVIEWER
class ObjectViewerView;
#endif

struct DetailLevel {
	int planets;
	int textures;
	int fracmult;
	int cities;
};

enum MsgLevel {
	MSG_NORMAL,
	MSG_IMPORTANT
};

class Frame;
class Game;

class Pi {
public:
	static void Init();
	static void InitGame();
	static void StarportStart(Uint32 starport);
	static void StartGame();
	static void EndGame();
	static void Start();
	static void MainLoop();
	static void TombStoneLoop();
	static void OnChangeDetailLevel();
	static void ToggleLuaConsole();
	static void Quit() __attribute((noreturn));
	static float GetFrameTime() { return frameTime; }
	static float GetGameTickAlpha() { return gameTickAlpha; }
	static float GetScrAspect() { return scrAspect; }
	static int KeyState(SDLKey k) { return keyState[k]; }
	static int KeyModState() { return keyModState; }
	static bool IsConsoleActive();
	static int JoystickButtonState(int joystick, int button);
	static int JoystickHatState(int joystick, int hat);
	static float JoystickAxisState(int joystick, int axis);
	static bool IsJoystickEnabled() { return joystickEnabled; }
	static void SetJoystickEnabled(bool state) { joystickEnabled = state; }
    static void SetMouseYInvert(bool state) { mouseYInvert = state; }
    static bool IsMouseYInvert() { return mouseYInvert; }
	static bool IsNavTunnelDisplayed() { return navTunnelDisplayed; }
	static void SetNavTunnelDisplayed(bool state) { navTunnelDisplayed = state; }
	static int MouseButtonState(int button) { return mouseButton[button]; }
	/// Get the default speed modifier to apply to movement (scrolling, zooming...), depending on the "shift" keys.
	/// This is a default value only, centralized here to promote uniform user expericience.
	static float GetMoveSpeedShiftModifier();
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	static void SetMouseGrab(bool on);
	static void BoinkNoise();
	static float CalcHyperspaceRangeMax(int hyperclass, int total_mass_in_tonnes);
	static float CalcHyperspaceRange(int hyperclass, float total_mass_in_tonnes, int fuel);
	static float CalcHyperspaceDuration(int hyperclass, int total_mass_in_tonnes, float dist);
	static float CalcHyperspaceFuelOut(int hyperclass, float dist, float hyperspace_range_max);
	static void Message(const std::string &message, const std::string &from = "", enum MsgLevel level = MSG_NORMAL);
	static std::string GetSaveDir();
	static ModelBase *FindModel(const std::string&);

	static const char SAVE_DIR_NAME[];

	static sigc::signal<void, SDL_keysym*> onKeyPress;
	static sigc::signal<void, SDL_keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;
	static sigc::signal<void> onPlayerChangeTarget; // navigation or combat
	static sigc::signal<void> onPlayerChangeFlightControlState;
	static sigc::signal<void> onPlayerChangeEquipment;
	static sigc::signal<void, const SpaceStation*> onDockingClearanceExpired;

	static LuaSerializer *luaSerializer;
	static LuaTimer *luaTimer;

	static LuaNameGen *luaNameGen;

	static RefCountedPtr<UI::Context> ui;

	static MTRand rng;
	static int statSceneTris;

	static void SetView(View *v);
	static View *GetView() { return currentView; }

#if WITH_DEVKEYS
	static bool showDebugInfo;
#endif
	static Player *player;
	static SectorView *sectorView;
	static GalacticView *galacticView;
	static GameMenuView *gameMenuView;
	static SystemInfoView *systemInfoView;
	static SystemView *systemView;
	static WorldView *worldView;
	static DeathView *deathView;
	static SpaceStationView *spaceStationView;
	static UIView *infoView;
	static LuaConsole *luaConsole;
	static ShipCpanel *cpan;
	static Sound::MusicPlayer &GetMusicPlayer() { return musicPlayer; }
	static Graphics::Renderer *renderer; // blargh
	static ModelCache *modelCache;

#if WITH_OBJECTVIEWER
	static ObjectViewerView *objectViewerView;
#endif

	static Game *game;

	static struct DetailLevel detail;
	static GameConfig *config;
private:
	static void HandleEvents();
	static void InitJoysticks();

	static bool menuDone;

	static View *currentView;

	/** So, the game physics rate (50Hz) can run slower
	  * than the frame rate. gameTickAlpha is the interpolation
	  * factor between one physics tick and another [0.0-1.0]
	  */
	static float gameTickAlpha;
	static int timeAccelIdx;
	static int requestedTimeAccelIdx;
	static bool forceTimeAccel;
	static float frameTime;
	static float scrAspect;
	static char keyState[SDLK_LAST];
	static int keyModState;
	static char mouseButton[6];
	static int mouseMotion[2];
	static bool doingMouseGrab;
	static bool warpAfterMouseGrab;
	static int mouseGrabWarpPos[2];
	static const float timeAccelRates[];

	static bool joystickEnabled;
	static bool mouseYInvert;
	struct JoystickState {
		SDL_Joystick *joystick;
		std::vector<bool> buttons;
		std::vector<int> hats;
		std::vector<float> axes;
	};
	static std::vector<JoystickState> joysticks;
	static Sound::MusicPlayer musicPlayer;

	static bool navTunnelDisplayed;

	static Gui::Fixed *menu;
};

#endif /* _PI_H */
