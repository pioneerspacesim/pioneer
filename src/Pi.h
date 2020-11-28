// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PI_H
#define _PI_H

#include "JobQueue.h"
#include "Random.h"
#include "core/GuiApplication.h"
#include "gameconsts.h"
#include "libs.h"

#include "SDL_keyboard.h"
#include <sigc++/sigc++.h>
#include <map>
#include <string>
#include <vector>

namespace Input {
	class Manager;
} //namespace Input

namespace PiGui {
	class Instance;
} //namespace PiGui

class Game;

class GameConfig;
class Intro;
class LuaConsole;
class LuaNameGen;
class LuaTimer;
class ModelCache;
class ObjectViewerView;
class Player;
class SystemPath;
class TransferPlanner;
class View;
class SDLGraphics;
class LuaSerializer;

#if ENABLE_SERVER_AGENT
class ServerAgent;
#endif

namespace Graphics {
	class Renderer;
	class Texture;
	class RenderState;
	class RenderTarget;
	namespace Drawables {
		class TexturedQuad;
	}
} // namespace Graphics

namespace SceneGraph {
	class Model;
}

namespace Sound {
	class MusicPlayer;
}

class DetailLevel {
public:
	DetailLevel() :
		planets(0),
		cities(0) {}
	int planets;
	int cities;
};

class Pi {
public:
	Pi() = delete;

	class App final : public GuiApplication {
	public:
		// TODO: headless mode should be part of a different, process-wide inter
		bool HeadlessMode() { return m_noGui; }

		void SetStartPath(const SystemPath &startPath);

	protected:
		// for compatibility, while we're moving Pi's internals into App
		friend class Pi;

		// Pi-internal lifecycle classes
		friend class MainMenu;
		friend class GameLoop;
		friend class TombstoneLoop;

		App() :
			GuiApplication("Pioneer") {}

		void Startup() override;
		void Shutdown() override;

		void PreUpdate() override;
		void PostUpdate() override;

		void RunJobs();

		void HandleRequests();
		bool HandleEvent(SDL_Event &ev) override;

	private:
		// msgs/requests that can be posted which the game processes at the end of a game loop in HandleRequests
		enum class InternalRequests {
			END_GAME = 0,
			QUIT_GAME,
		};

		std::vector<InternalRequests> internalRequests;

		bool m_noGui;

		std::shared_ptr<Lifecycle> m_loader;
		std::shared_ptr<Lifecycle> m_mainMenu;
		std::shared_ptr<Lifecycle> m_gameLoop;
	};

public:
	static void Init(const std::map<std::string, std::string> &options, bool no_gui = false);

	static void StartGame(Game *game);

	static void RequestEndGame(); // request that the game is ended as soon as safely possible
	static void RequestQuit();

	static void OnChangeDetailLevel();
	static float GetFrameTime() { return frameTime; }
	static float GetGameTickAlpha() { return gameTickAlpha; }
	// for internal use, don't modify unless you know what you're doing
	static void SetGameTickAlpha(float alpha) { gameTickAlpha = alpha; }

	static void SetShowDebugInfo(bool enabled) { showDebugInfo = enabled; };
	static void ToggleShowDebugInfo() { showDebugInfo = !showDebugInfo; };

	// FIXME: hacked-in singleton pattern, find a better way to locate the application
	static App *GetApp() { return m_instance; }

	static bool IsNavTunnelDisplayed() { return navTunnelDisplayed; }
	static void SetNavTunnelDisplayed(bool state) { navTunnelDisplayed = state; }
	static bool AreSpeedLinesDisplayed() { return speedLinesDisplayed; }
	static void SetSpeedLinesDisplayed(bool state) { speedLinesDisplayed = state; }
	static bool AreHudTrailsDisplayed() { return hudTrailsDisplayed; }
	static void SetHudTrailsDisplayed(bool state) { hudTrailsDisplayed = state; }

	static void SetMouseGrab(bool on);
	static bool DoingMouseGrab() { return doingMouseGrab; }

	// Get the default speed modifier to apply to movement (scrolling, zooming...), depending on the "shift" keys.
	// This is a default value only, centralized here to promote uniform user expericience.
	static float GetMoveSpeedShiftModifier();

	static std::string GetSaveDir();
	static SceneGraph::Model *FindModel(const std::string &, bool allowPlaceholder = true);

	static const char SAVE_DIR_NAME[];

	static LuaSerializer *luaSerializer;
	static LuaTimer *luaTimer;

	static LuaNameGen *luaNameGen;

#if ENABLE_SERVER_AGENT
	static ServerAgent *serverAgent;
#endif

	static PiGui::Instance *pigui;

	static Random rng;
	static int statSceneTris;
	static int statNumPatches;

	static void SetView(View *v);
	static View *GetView() { return currentView; }

	static void SetAmountBackgroundStars(const float pc)
	{
		amountOfBackgroundStarsDisplayed = Clamp(pc, 0.01f, 1.0f);
		bRefreshBackgroundStars = true;
	}
	static float GetAmountBackgroundStars() { return amountOfBackgroundStarsDisplayed; }
	static bool MustRefreshBackgroundClearFlag()
	{
		const bool bRet = bRefreshBackgroundStars;
		bRefreshBackgroundStars = false;
		return bRet;
	}

	/* Only use #if WITH_DEVKEYS */
	static bool showDebugInfo;

#if PIONEER_PROFILER
	static std::string profilerPath;
	static std::string profileOnePath;
	static bool doProfileSlow;
	static bool doProfileOne;
#endif

	static void RequestProfileFrame(const std::string &profilePath = "");

	static Input::Manager *input;
	static Player *player;
	static TransferPlanner *planner;
	static std::unique_ptr<LuaConsole> luaConsole;
	static Sound::MusicPlayer &GetMusicPlayer() { return musicPlayer; }
	static Graphics::Renderer *renderer;
	static ModelCache *modelCache;
	static Intro *intro;
	static SDLGraphics *sdl;

	static Game *game;

	static DetailLevel detail;
	static GameConfig *config;

	static JobQueue *GetAsyncJobQueue() { return asyncJobQueue.get(); }
	static JobQueue *GetSyncJobQueue() { return syncJobQueue.get(); }

	static bool DrawGUI;

private:
	static void HandleKeyDown(SDL_Keysym *key);

	// private members
	static const Uint32 SYNC_JOBS_PER_LOOP = 1;
	static std::unique_ptr<AsyncJobQueue> asyncJobQueue;
	static std::unique_ptr<SyncJobQueue> syncJobQueue;

	static bool menuDone;

	static View *currentView;

	/** So, the game physics rate (50Hz) can run slower
	  * than the frame rate. gameTickAlpha is the interpolation
	  * factor between one physics tick and another [0.0-1.0]
	  */
	static float gameTickAlpha;
	static float frameTime;

	static Sound::MusicPlayer musicPlayer;

	static bool navTunnelDisplayed;
	static bool speedLinesDisplayed;
	static bool hudTrailsDisplayed;
	static bool bRefreshBackgroundStars;
	static float amountOfBackgroundStarsDisplayed;

	static Graphics::RenderTarget *renderTarget;
	static RefCountedPtr<Graphics::Texture> renderTexture;
	static std::unique_ptr<Graphics::Drawables::TexturedQuad> renderQuad;
	static Graphics::RenderState *quadRenderState;

	static bool doingMouseGrab;

	static bool isRecordingVideo;
	static FILE *ffmpegFile;

private:
	// for compatibility, while we're moving Pi's internals into App
	friend class App;

	static App *m_instance;
};

#endif /* _PI_H */
