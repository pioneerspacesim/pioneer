#ifndef _PI_H
#define _PI_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "mtrand.h"
#include "StarSystem.h"
#include <map>
#include <string>

class Player;
class SectorView;
class SystemView;
class WorldView;
class SystemInfoView;
class ShipCpanel;
class StarSystem;
class SpaceStationView;
class InfoView;

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

/*
 * Implementation is in main.cpp, just to confuse you.
 */
class Pi {
public:
	static void Init(IniConfig &config);
	static void MainLoop();
	static void Quit();
	static float GetFrameTime() { return frameTime; }
	static double GetGameTime() { return gameTime; }
	static void SetTimeAccel(float s) { timeAccel = s; }
	static float GetTimeAccel() { return timeAccel; }
	static float GetTimeStep() { return timeAccel*frameTime; }
	static int GetScrWidth() { return scrWidth; }
	static int GetScrHeight() { return scrHeight; }
	static float GetScrAspect() { return scrAspect; }
	static int KeyState(SDLKey k) { return keyState[k]; }
	static int MouseButtonState(int button) { return mouseButton[button]; }
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	
	static sigc::signal<void, SDL_keysym*> onKeyPress;
	static sigc::signal<void, SDL_keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;

	static MTRand rng;

	static void HyperspaceTo(StarSystem *destination);
	enum CamType { CAM_FRONT, CAM_REAR, CAM_EXTERNAL };
	enum MapView { MAP_NOMAP, MAP_SECTOR, MAP_SYSTEM };
	static void SetCamType(enum CamType);
	static void SetMapView(enum MapView);
	static enum CamType GetCamType() { return cam_type; }
	static enum MapView GetMapView() { return map_view; }
	static void SetView(View *v);
	static View *GetView() { return current_view; }
	static StarSystem *GetSelectedSystem();

	static systemloc_t playerLoc;
	static Player *player;
	static SectorView *sector_view;
	static SystemInfoView *system_info_view;
	static WorldView *world_view;
	static SpaceStationView *spaceStationView;
	static InfoView *infoView;
	static ShipCpanel *cpan;
	static GLUquadric *gluQuadric;
private:
	static void InitOpenGL();
	static void HandleEvents();
	
	static View *current_view;
	static SystemView *system_view;
	
	static double gameTime;
	static StarSystem *selected_system;
	static enum CamType cam_type;
	static enum MapView map_view;
	static float timeAccel;
	static float frameTime;
	static int scrWidth, scrHeight;
	static float scrAspect;
	static SDL_Surface *scrSurface;
	static char keyState[SDLK_LAST];
	static char mouseButton[5];
	static int mouseMotion[2];
};

#endif /* _PI_H */
