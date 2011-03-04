#ifndef _PI_H
#define _PI_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "mtrand.h"
#include "gameconsts.h"
#include "Serializer.h"
#include "IniConfig.h"
#include <map>
#include <string>
#include <vector>

class Player;
class SectorView;
class SystemView;
class WorldView;
class ObjectViewerView;
class SystemInfoView;
class ShipCpanel;
class StarSystem;
class SpaceStationView;
class InfoView;
class SpaceStation;
class GalacticView;
class Ship;
class SBodyPath;
class GameMenuView;
struct lua_State;

struct DetailLevel {
	int planets;
	int cities;
};

enum MsgLevel {
	MSG_NORMAL,
	MSG_IMPORTANT
};

class Frame;

#define PHYSICS_HZ (60.0f)

class Pi {
public:
	static void Init();
	static void InitGame();
	static void StartGame();
	static void UninitGame();
	static void EndGame();
	static void Start();
	static void MainLoop();
	static void TombStoneLoop();
	static void OnChangeDetailLevel();
	static void Quit();
	static void Serialize(Serializer::Writer &wr);
	static void Unserialize(Serializer::Reader &rd);
	static float GetFrameTime() { return frameTime; }
	static double GetGameTime() { return gameTime; }
	static void SetTimeAccel(int v);
	static void RequestTimeAccel(int v);
	static int GetRequestedTimeAccelIdx() { return requestedTimeAccelIdx; }
	static int GetTimeAccelIdx() { return timeAccelIdx; }
	static float GetTimeAccel() { return timeAccelRates[timeAccelIdx]; }
	static float GetTimeStep() { return timeAccelRates[timeAccelIdx]*(1.0f/PHYSICS_HZ); }
	static float GetGameTickAlpha() { return gameTickAlpha; }
	static int GetScrWidth() { return scrWidth; }
	static int GetScrHeight() { return scrHeight; }
	static float GetScrAspect() { return scrAspect; }
	static int KeyState(SDLKey k) { return keyState[k]; }
	static int KeyModState() { return keyModState; }
	static int JoystickButtonState(int joystick, int button);
	static int JoystickHatState(int joystick, int hat);
	static float JoystickAxisState(int joystick, int axis);
	static int MouseButtonState(int button) { return mouseButton[button]; }
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	static void BoinkNoise();
	static bool IsGameStarted() { return isGameStarted; }
	static float CalcHyperspaceRange(int hyperclass, int total_mass_in_tonnes);
	static void Message(const std::string &message, const std::string &from = "", enum MsgLevel level = MSG_NORMAL);

	static sigc::signal<void, SDL_keysym*> onKeyPress;
	static sigc::signal<void, SDL_keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;
	static sigc::signal<void> onPlayerChangeTarget; // navigation or combat
	static sigc::signal<void> onPlayerChangeHyperspaceTarget;
	static sigc::signal<void> onPlayerHyperspaceToNewSystem;
	static sigc::signal<void> onPlayerChangeFlightControlState;
	static sigc::signal<void> onPlayerChangeEquipment;
	static sigc::signal<void, const SpaceStation*> onDockingClearanceExpired;

	static MTRand rng;
	static int statSceneTris;

	static void SetView(View *v);
	static View *GetView() { return currentView; }
	static StarSystem *GetSelectedSystem();

	static bool showDebugInfo;
	static Player *player;
	static SectorView *sectorView;
	static GalacticView *galacticView;
	static GameMenuView *gameMenuView;
	static SystemInfoView *systemInfoView;
	static SystemView *systemView;
	static WorldView *worldView;
	static ObjectViewerView *objectViewerView;
	static SpaceStationView *spaceStationView;
	static InfoView *infoView;
	static ShipCpanel *cpan;
	static GLUquadric *gluQuadric;
	static StarSystem *currentSystem;
	static lua_State *luaPersistent;

	static int CombatRating(int kills);
	static const char * const combatRating[];

	static struct DetailLevel detail;
	static IniConfig config;
private:
	static void InitOpenGL();
	static void HandleEvents();
	static void InitJoysticks();

	static View *currentView;

	static double gameTime;
	/** So, the game physics rate (50Hz) can run slower
	  * than the frame rate. gameTickAlpha is the interpolation
	  * factor between one physics tick and another [0.0-1.0]
	  */
	static float gameTickAlpha;
	static StarSystem *selectedSystem;
	static int timeAccelIdx;
	static int requestedTimeAccelIdx;
	static float frameTime;
	static int scrWidth, scrHeight;
	static float scrAspect;
	static SDL_Surface *scrSurface;
	static char keyState[SDLK_LAST];
	static int keyModState;
	static char mouseButton[6];
	static int mouseMotion[2];
	static const float timeAccelRates[];
	static bool isGameStarted;

	struct JoystickState {
		SDL_Joystick *joystick;
		std::vector<bool> buttons;
		std::vector<int> hats;
		std::vector<float> axes;
	};
	static std::vector<JoystickState> joysticks;
};

#endif /* _PI_H */
