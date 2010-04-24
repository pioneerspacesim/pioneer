#ifndef _PI_H
#define _PI_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "mtrand.h"
#include "gameconsts.h"
#include <map>
#include <string>

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
struct SBodyPath;
class GameMenuView;
struct lua_State;

class IniConfig: private std::map<std::string, std::string> {
	public:
	IniConfig(const char *filename);
	int Int(const char *key) {
		return atoi((*this)[key].c_str());
	}
	float Float(const char *key) {
		float val;
		if (sscanf((*this)[key].c_str(), "%f", &val)==1) return val;
		else return 0;
	}
	std::string String(const char *key) {
		return (*this)[key];
	}
};

struct DetailLevel {
	int planets;
	int cities;
};

class Frame;
/*
 * Implementation is in main.cpp, just to confuse you.
 */
class Pi {
public:
	static void Init(IniConfig &config);
	static void InitGame();
	static void StartGame();
	static void UninitGame();
	static void EndGame();
	static void Start();
	static void MainLoop();
	static void TombStoneLoop();
	static void OnChangeDetailLevel();
	static void Quit();
	static void Serialize();
	static void Unserialize();
	static float GetFrameTime() { return frameTime; }
	static double GetGameTime() { return gameTime; }
	static void SetTimeAccel(int v);
	static void RequestTimeAccel(int v);
	static float GetRequestedTimeAccelIdx() { return requestedTimeAccelIdx; }
	static float GetTimeAccelIdx() { return timeAccelIdx; }
	static float GetTimeAccel() { return timeAccelRates[timeAccelIdx]; }
	static float GetTimeStep() { return timeAccelRates[timeAccelIdx]*(1.0f/62.5f); }
	static int GetScrWidth() { return scrWidth; }
	static int GetScrHeight() { return scrHeight; }
	static float GetScrAspect() { return scrAspect; }
	static int KeyState(SDLKey k) { return keyState[k]; }
	static int MouseButtonState(int button) { return mouseButton[button]; }
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	static void BoinkNoise();
	static bool IsGameStarted() { return isGameStarted; }
	static float CalcHyperspaceRange(int hyperclass, int total_mass_in_tonnes);
	
	static sigc::signal<void, SDL_keysym*> onKeyPress;
	static sigc::signal<void, SDL_keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;
	static sigc::signal<void> onPlayerChangeTarget; // navigation or combat
	static sigc::signal<void> onPlayerChangeHyperspaceTarget;
	static sigc::signal<void> onPlayerHyperspaceToNewSystem;
	static sigc::signal<void> onPlayerMissionListChanged;
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
private:
	static void InitOpenGL();
	static void HandleEvents();
	
	static View *currentView;
	
	static double gameTime;
	static StarSystem *selectedSystem;
	static int timeAccelIdx;
	static int requestedTimeAccelIdx;
	static float frameTime;
	static int scrWidth, scrHeight;
	static float scrAspect;
	static SDL_Surface *scrSurface;
	static char keyState[SDLK_LAST];
	static char mouseButton[6];
	static int mouseMotion[2];
	static const float timeAccelRates[];
	static bool isGameStarted;
};

#endif /* _PI_H */
