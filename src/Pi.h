#ifndef _PI_H
#define _PI_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "mtrand.h"
#include "gameconsts.h"
#include "GameConfig.h"
#include "LuaEventQueue.h"
#include "LuaSerializer.h"
#include "LuaTimer.h"
#include "CargoBody.h"
#include "Space.h"
#include <map>
#include <string>
#include <vector>

class Player;
class SectorView;
class SystemView;
class WorldView;
class SystemInfoView;
class ShipCpanel;
class StarSystem;
class SpaceStationView;
class InfoView;
class SpaceStation;
class GalacticView;
class Ship;
class GameMenuView;
class LuaConsole;
class LuaNameGen;
namespace Sound { class MusicPlayer; }
class TextureCache;

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
	static void RedirectStdio();
	static void InitGame();
	static void StarportStart(Uint32 starport);
	static void StartGame();
	static void EndGame();
	static void Start();
	static void MainLoop();
	static void TombStoneLoop();
	static void HandleMenuKey(int n);
	static void OnChangeDetailLevel();
	static void ToggleLuaConsole();
	static void Quit() __attribute((noreturn));
	static float GetFrameTime() { return frameTime; }
	static float GetGameTickAlpha() { return gameTickAlpha; }
	static int GetScrWidth() { return scrWidth; }
	static int GetScrHeight() { return scrHeight; }
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
	static int MouseButtonState(int button) { return mouseButton[button]; }
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	static void SetMouseGrab(bool on);
	static void BoinkNoise();
	static float CalcHyperspaceRange(int hyperclass, int total_mass_in_tonnes);
	static void Message(const std::string &message, const std::string &from = "", enum MsgLevel level = MSG_NORMAL);

	static sigc::signal<void, SDL_keysym*> onKeyPress;
	static sigc::signal<void, SDL_keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;
	static sigc::signal<void> onPlayerChangeTarget; // navigation or combat
	static sigc::signal<void> onPlayerChangeFlightControlState;
	static sigc::signal<void> onPlayerChangeEquipment;
	static sigc::signal<void, const SpaceStation*> onDockingClearanceExpired;

	static LuaManager *luaManager;

	static LuaSerializer *luaSerializer;
	static LuaTimer *luaTimer;

	static LuaEventQueue<> *luaOnGameStart;
	static LuaEventQueue<> *luaOnGameEnd;
	static LuaEventQueue<Ship> *luaOnEnterSystem;
	static LuaEventQueue<Ship> *luaOnLeaveSystem;
	static LuaEventQueue<Body> *luaOnFrameChanged;
	static LuaEventQueue<Ship,Body> *luaOnShipDestroyed;
	static LuaEventQueue<Ship,Body> *luaOnShipHit;
	static LuaEventQueue<Ship,Body> *luaOnShipCollided;
	static LuaEventQueue<Ship,SpaceStation> *luaOnShipDocked;
	static LuaEventQueue<Ship,SpaceStation> *luaOnShipUndocked;
	static LuaEventQueue<Ship,Body> *luaOnShipLanded;
	static LuaEventQueue<Ship,Body> *luaOnShipTakeOff;
	static LuaEventQueue<Ship,const char *> *luaOnShipAlertChanged;
	static LuaEventQueue<Ship,CargoBody> *luaOnJettison;
	static LuaEventQueue<Body,const char *> *luaOnCargoUnload;
	static LuaEventQueue<Ship,const char *> *luaOnAICompleted;
	static LuaEventQueue<SpaceStation> *luaOnCreateBB;
	static LuaEventQueue<SpaceStation> *luaOnUpdateBB;
	static LuaEventQueue<> *luaOnSongFinished;
	static LuaEventQueue<Ship> *luaOnShipFlavourChanged;
	static LuaEventQueue<Ship,const char *> *luaOnShipEquipmentChanged;

	static LuaNameGen *luaNameGen;

	static TextureCache *textureCache;

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
	static SpaceStationView *spaceStationView;
	static InfoView *infoView;
	static LuaConsole *luaConsole;
	static ShipCpanel *cpan;
	static GLUquadric *gluQuadric;
	static Sound::MusicPlayer &GetMusicPlayer() { return musicPlayer; }

#if WITH_OBJECTVIEWER
	static ObjectViewerView *objectViewerView;
#endif

	static Game *game;

	static int CombatRating(int kills);
	static const char * const combatRating[];

	static struct DetailLevel detail;
	static GameConfig config;
private:
	static void InitOpenGL();
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
	static int scrWidth, scrHeight;
	static float scrAspect;
	static SDL_Surface *scrSurface;
	static char keyState[SDLK_LAST];
	static int keyModState;
	static char mouseButton[6];
	static int mouseMotion[2];
	static bool doingMouseGrab;
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
};

#endif /* _PI_H */
