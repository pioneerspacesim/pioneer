// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAME_H
#define _GAME_H

#include "JsonFwd.h"
#include "galaxy/Galaxy.h"
#include "galaxy/SystemPath.h"
#include "gameconsts.h"
#include <string>

class GameLog;
class HyperspaceCloud;
class Player;
class Space;

namespace Graphics {
	class Renderer;
}

struct CannotSaveCurrentGameState {};
struct CannotSaveInHyperspace : public CannotSaveCurrentGameState {};
struct CannotSaveDeadPlayer : public CannotSaveCurrentGameState {};
struct InvalidGameStartLocation {
	std::string error;
	InvalidGameStartLocation(const std::string &error_) :
		error(error_) {}
};

class View;
class SectorView;
class SystemView;
class WorldView;
class DeathView;
class ObjectViewerView;

class Game {
public:
	static Json LoadGameToJson(const std::string &filename);
	// LoadGame and SaveGame throw exceptions on failure
	static Game *LoadGame(const std::string &filename);
	static bool CanLoadGame(const std::string &filename);
	// XXX game arg should be const, and this should probably be a member function
	// (or LoadGame/SaveGame should be somewhere else entirely)
	static void SaveGame(const std::string &filename, Game *game);
	static bool DeleteSave(const std::string &filename);
	static int CurrentSaveVersion();

	// start docked in station referenced by path or nearby to body if it is no station
	Game(const SystemPath &path, const double startDateTime, const char *shipType = "kanara");

	// load game
	Game(const Json &jsonObj);

	~Game();

	// save game
	void ToJson(Json &jsonObj);

	// various game states
	bool IsNormalSpace() const { return m_state == State::NORMAL; }
	bool IsHyperspace() const { return m_state == State::HYPERSPACE; }

	RefCountedPtr<Galaxy> GetGalaxy() const { return m_galaxy; }
	Space *GetSpace() const { return m_space.get(); }
	double GetTime() const { return m_time; }
	Player *GetPlayer() const { return m_player.get(); }

	// physics step
	void TimeStep(float step);

	// update time acceleration once per render frame
	// returns true if timeaccel was changed
	bool UpdateTimeAccel();

	// request switch to hyperspace
	void WantHyperspace();

	// hyperspace parameters. only meaningful when IsHyperspace() is true
	float GetHyperspaceProgress() const { return m_hyperspaceProgress; }
	double GetHyperspaceDuration() const { return m_hyperspaceDuration; }
	double GetHyperspaceEndTime() const { return m_hyperspaceEndTime; }
	double GetHyperspaceArrivalProbability() const;
	const SystemPath &GetHyperspaceDest() const { return m_hyperspaceDest; }
	const SystemPath &GetHyperspaceSource() const { return m_hyperspaceSource; }
	void RemoveHyperspaceCloud(HyperspaceCloud *);

	enum TimeAccel {
		TIMEACCEL_PAUSED,
		TIMEACCEL_1X,
		TIMEACCEL_10X,
		TIMEACCEL_100X,
		TIMEACCEL_1000X,
		TIMEACCEL_10000X,
		TIMEACCEL_HYPERSPACE
	};

	void SetTimeAccel(TimeAccel t);
	void RequestTimeAccel(TimeAccel t, bool force = false);

	/// Requests an increase in time acceleration
	/// @param force if set to false the system can reject the request under certain conditions
	void RequestTimeAccelInc(bool force = false);
	/// Requests a decrease in time acceleration
	/// @param force if set to false the system can reject the request under certain conditions
	void RequestTimeAccelDec(bool force = false);

	TimeAccel GetTimeAccel() const { return m_timeAccel; }
	TimeAccel GetRequestedTimeAccel() const { return m_requestedTimeAccel; }
	bool IsPaused() const { return m_timeAccel == TIMEACCEL_PAUSED; }

	float GetTimeAccelRate() const { return s_timeAccelRates[m_timeAccel]; }
	float GetInvTimeAccelRate() const { return s_timeInvAccelRates[m_timeAccel]; }

	float GetTimeStep() const { return s_timeAccelRates[m_timeAccel] * (1.0f / PHYSICS_HZ); }

	SectorView *GetSectorView() const { return m_gameViews->m_sectorView; }
	SystemView *GetSystemView() const { return m_gameViews->m_systemView; }
	WorldView *GetWorldView() const { return m_gameViews->m_worldView; }
	DeathView *GetDeathView() const { return m_gameViews->m_deathView; }
	View *GetSpaceStationView() const { return m_gameViews->m_spaceStationView; }
	View *GetInfoView() const { return m_gameViews->m_infoView; }

	/* Only use #if WITH_OBJECTVIEWER */
	ObjectViewerView *GetObjectViewerView() const;

	GameLog *log;

private:
	class Views {
	public:
		Views();
		void Init(Game *game);
		void LoadFromJson(const Json &jsonObj, Game *game);
		~Views();

		void SetRenderer(Graphics::Renderer *r);

		SectorView *m_sectorView = nullptr;
		SystemView *m_systemView = nullptr;
		WorldView *m_worldView = nullptr;
		DeathView *m_deathView = nullptr;
		View *m_spaceStationView = nullptr;
		View *m_infoView = nullptr;

		/* Only use #if WITH_OBJECTVIEWER */
		ObjectViewerView *m_objectViewerView = nullptr;
	};

	void CreateViews();
	void LoadViewsFromJson(const Json &jsonObj);
	void DestroyViews();

	static void EmitPauseState(bool paused);

	void SwitchToHyperspace();
	void SwitchToNormalSpace();

	std::unique_ptr<Player> m_player;

	RefCountedPtr<Galaxy> m_galaxy;
	std::unique_ptr<Views> m_gameViews;
	std::unique_ptr<Space> m_space;
	double m_time;

	enum class State {
		NORMAL,
		HYPERSPACE,
	};
	State m_state;

	bool m_wantHyperspace;

	std::list<HyperspaceCloud *> m_hyperspaceClouds;
	SystemPath m_hyperspaceSource;
	SystemPath m_hyperspaceDest;
	double m_hyperspaceProgress;
	double m_hyperspaceDuration;
	double m_hyperspaceEndTime;

	TimeAccel m_timeAccel;
	TimeAccel m_requestedTimeAccel;
	bool m_forceTimeAccel;
	static const float s_timeAccelRates[];
	static const float s_timeInvAccelRates[];
};

#endif
