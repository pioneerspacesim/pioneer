// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PI_H
#define _PI_H

#include "utils.h"
#include "gui/Gui.h"
#include "Random.h"
#include "gameconsts.h"
#include "GameConfig.h"
#include "LuaSerializer.h"
#include "LuaTimer.h"
#include "CargoBody.h"
#include "Space.h"
#include "JobQueue.h"
#include <map>
#include <string>
#include <vector>

class DeathView;
class GalacticView;
class Galaxy;
class Intro;
class LuaConsole;
class LuaNameGen;
class ModelCache;
class Player;
class SectorView;
class Ship;
class ShipCpanel;
class SpaceStation;
class StarSystem;
class SystemInfoView;
class SystemView;
class UIView;
class View;
class WorldView;
class SDLGraphics;
namespace Graphics { class Renderer; }
namespace SceneGraph { class Model; }
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
	static void Init(const std::map<std::string,std::string> &options, bool no_gui = false);
	static void InitGame();
	static void StarportStart(Uint32 starport);
	static void StartGame();
	static void EndGame();
	static void Start();
	static void MainLoop();
	static void TombStoneLoop();
	static void OnChangeDetailLevel();
	static void Quit() __attribute((noreturn));
	static float GetFrameTime() { return frameTime; }
	static float GetGameTickAlpha() { return gameTickAlpha; }
	static bool KeyState(SDL_Keycode k) { return keyState[k]; }
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
	static bool AreSpeedLinesDisplayed() { return speedLinesDisplayed; }
	static void SetSpeedLinesDisplayed(bool state) { speedLinesDisplayed = state; }
	static bool AreHudTrailsDisplayed() { return hudTrailsDisplayed; }
	static void SetHudTrailsDisplayed(bool state) { hudTrailsDisplayed = state; }
	static int MouseButtonState(int button) { return mouseButton[button]; }
	/// Get the default speed modifier to apply to movement (scrolling, zooming...), depending on the "shift" keys.
	/// This is a default value only, centralized here to promote uniform user expericience.
	static float GetMoveSpeedShiftModifier();
	static void GetMouseMotion(int motion[2]) {
		memcpy(motion, mouseMotion, sizeof(int)*2);
	}
	static void SetMouseGrab(bool on);
	static void FlushCaches();
	static void BoinkNoise();
	static void Message(const std::string &message, const std::string &from = "", enum MsgLevel level = MSG_NORMAL);
	static std::string GetSaveDir();
	static SceneGraph::Model *FindModel(const std::string&, bool allowPlaceholder = true);

	static void CreateRenderTarget(const Uint16 width, const Uint16 height);
	static void DrawRenderTarget();
	static void BeginRenderTarget();
	static void EndRenderTarget();

	static const char SAVE_DIR_NAME[];

	static sigc::signal<void, SDL_Keysym*> onKeyPress;
	static sigc::signal<void, SDL_Keysym*> onKeyRelease;
	static sigc::signal<void, int, int, int> onMouseButtonUp;
	static sigc::signal<void, int, int, int> onMouseButtonDown;
	static sigc::signal<void, bool> onMouseWheel;
	static sigc::signal<void> onPlayerChangeTarget; // navigation or combat
	static sigc::signal<void> onPlayerChangeFlightControlState;

	static LuaSerializer *luaSerializer;
	static LuaTimer *luaTimer;

	static LuaNameGen *luaNameGen;

	static RefCountedPtr<UI::Context> ui;

	static Random rng;
	static int statSceneTris;

	static void SetView(View *v);
	static View *GetView() { return currentView; }

#if WITH_DEVKEYS
	static bool showDebugInfo;
#endif
#if PIONEER_PROFILER
	static std::string profilerPath;
	static bool doProfileSlow;
	static bool doProfileOne;
#endif

	static Player *player;
	static SectorView *sectorView;
	static GalacticView *galacticView;
	static UIView *settingsView;
	static SystemInfoView *systemInfoView;
	static SystemView *systemView;
	static WorldView *worldView;
	static DeathView *deathView;
	static UIView *spaceStationView;
	static UIView *infoView;
	static LuaConsole *luaConsole;
	static ShipCpanel *cpan;
	static Sound::MusicPlayer &GetMusicPlayer() { return musicPlayer; }
	static Graphics::Renderer *renderer; // blargh
	static ModelCache *modelCache;
	static Intro *intro;
	static SDLGraphics *sdl;

#if WITH_OBJECTVIEWER
	static ObjectViewerView *objectViewerView;
#endif

	static Game *game;

	static struct DetailLevel detail;
	static GameConfig *config;

	static JobQueue *GetAsyncJobQueue() { return asyncJobQueue.get();}
	static JobQueue *GetSyncJobQueue() { return syncJobQueue.get();}

	static Galaxy* GetGalaxy() { return s_galaxy; }

	static bool DrawGUI;

private:
	static void HandleEvents();
	static void InitJoysticks();

	static const Uint32 SYNC_JOBS_PER_LOOP = 1;
	static std::unique_ptr<AsyncJobQueue> asyncJobQueue;
	static std::unique_ptr<SyncJobQueue> syncJobQueue;

	static Galaxy* s_galaxy;
	static bool menuDone;

	static View *currentView;

	/** So, the game physics rate (50Hz) can run slower
	  * than the frame rate. gameTickAlpha is the interpolation
	  * factor between one physics tick and another [0.0-1.0]
	  */
	static float gameTickAlpha;
	static float frameTime;
	static std::map<SDL_Keycode,bool> keyState;
	static int keyModState;
	static char mouseButton[6];
	static int mouseMotion[2];
	static bool doingMouseGrab;
	static bool warpAfterMouseGrab;
	static int mouseGrabWarpPos[2];

	static bool joystickEnabled;
	static bool mouseYInvert;
	struct JoystickState {
		SDL_Joystick *joystick;
		std::vector<bool> buttons;
		std::vector<int> hats;
		std::vector<float> axes;
	};
	static std::map<SDL_JoystickID,JoystickState> joysticks;
	static Sound::MusicPlayer musicPlayer;

	static bool navTunnelDisplayed;
	static bool speedLinesDisplayed;
	static bool hudTrailsDisplayed;

	static Gui::Fixed *menu;

	static Graphics::RenderTarget *renderTarget;
	static RefCountedPtr<Graphics::Texture> renderTexture;
	static std::unique_ptr<Graphics::Drawables::TexturedQuad> renderQuad;
	static Graphics::RenderState *quadRenderState;
};

#endif /* _PI_H */
