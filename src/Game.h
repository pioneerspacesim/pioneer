// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAME_H
#define _GAME_H

#include <string>
#include "libs.h"
#include "gameconsts.h"
#include "GameLog.h"
#include "Serializer.h"
#include "galaxy/Galaxy.h"
#include "galaxy/SystemPath.h"

class HyperspaceCloud;
class Player;
class ShipController;
class Space;

struct CannotSaveCurrentGameState {};
struct CannotSaveInHyperspace : public CannotSaveCurrentGameState {};
struct CannotSaveDeadPlayer : public CannotSaveCurrentGameState {};
struct InvalidGameStartLocation {
	std::string error;
	InvalidGameStartLocation(const std::string& error_) : error(error_) {}
};

class SectorView;
class UIView;
class SystemInfoView;
class SystemView;
class WorldView;
class DeathView;
class UIView;
class UIView;
class ShipCpanel;
#if WITH_OBJECTVIEWER
class ObjectViewerView;
#endif

class Game {
public:
	// LoadGame and SaveGame throw exceptions on failure
	static Game *LoadGame(const std::string &filename);
	static bool CanLoadGame(const std::string &filename);
	// XXX game arg should be const, and this should probably be a member function
	// (or LoadGame/SaveGame should be somewhere else entirely)
	static void SaveGame(const std::string &filename, Game *game);

	// start docked in station referenced by path or nearby to body if it is no station
	Game(const SystemPath &path, double time = 0.0);

	// load game
	Game(const Json::Value &jsonObj);

	~Game();

	// save game
	void ToJson(Json::Value &jsonObj);

	// various game states
	bool IsNormalSpace() const { return m_state == STATE_NORMAL; }
	bool IsHyperspace() const { return m_state == STATE_HYPERSPACE; }

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
	const SystemPath& GetHyperspaceDest() const { return m_hyperspaceDest; }
	const SystemPath& GetHyperspaceSource() const { return m_hyperspaceSource; }
	void RemoveHyperspaceCloud(HyperspaceCloud*);

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

	float GetTimeStep() const { return s_timeAccelRates[m_timeAccel]*(1.0f/PHYSICS_HZ); }

	SectorView* GetSectorView() const { return m_gameViews->m_sectorView; }
	UIView* GetGalacticView() const { return m_gameViews->m_galacticView; }
	UIView* GetSettingsView() const { return m_gameViews->m_settingsView; }
	SystemInfoView* GetSystemInfoView() const { return m_gameViews->m_systemInfoView; }
	SystemView* GetSystemView() const { return m_gameViews->m_systemView; }
	WorldView* GetWorldView() const { return m_gameViews->m_worldView; }
	DeathView* GetDeathView() const { return m_gameViews->m_deathView; }
	UIView* GetSpaceStationView() const { return m_gameViews->m_spaceStationView; }
	UIView* GetInfoView() const { return m_gameViews->m_infoView; }
	ShipCpanel* GetCpan() const { return m_gameViews->m_cpan; }
#if WITH_OBJECTVIEWER
	ObjectViewerView* GetObjectViewerView() const { return m_gameViews->m_objectViewerView; }
#endif

	GameLog *log;

private:
	class Views {
	public:
		Views();
		void Init(Game* game);
		void LoadFromJson(const Json::Value &jsonObj, Game* game);
		~Views();

		void SetRenderer(Graphics::Renderer *r);

		SectorView* m_sectorView;
		UIView* m_galacticView;
		UIView* m_settingsView;
		SystemInfoView* m_systemInfoView;
		SystemView* m_systemView;
		WorldView* m_worldView;
		DeathView* m_deathView;
		UIView* m_spaceStationView;
		UIView* m_infoView;
		ShipCpanel* m_cpan;
#if WITH_OBJECTVIEWER
		ObjectViewerView* m_objectViewerView;
#endif
	};

	void CreateViews();
	void LoadViewsFromJson(const Json::Value &jsonObj);
	void DestroyViews();

	static void EmitPauseState(bool paused);

	void SwitchToHyperspace();
	void SwitchToNormalSpace();

	RefCountedPtr<Galaxy> m_galaxy;
	std::unique_ptr<Views> m_gameViews;
	std::unique_ptr<Space> m_space;
	double m_time;

	std::unique_ptr<Player> m_player;

	enum State {
		STATE_NORMAL,
		STATE_HYPERSPACE,
	};
	State m_state;

	bool m_wantHyperspace;

	std::list<HyperspaceCloud*> m_hyperspaceClouds;
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
