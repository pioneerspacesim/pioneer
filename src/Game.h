// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAME_H
#define _GAME_H

#include "libs.h"
#include "gameconsts.h"
#include "GameLog.h"
#include "Serializer.h"
#include "galaxy/SystemPath.h"

class HyperspaceCloud;
class Player;
class ShipController;
class Space;

struct CannotSaveCurrentGameState {};
struct CannotSaveInHyperspace : public CannotSaveCurrentGameState {};
struct CannotSaveDeadPlayer : public CannotSaveCurrentGameState {};

class Game {
public:
	// LoadGame and SaveGame throw exceptions on failure
	static Game *LoadGame(const std::string &filename);
	// XXX game arg should be const, and this should probably be a member function
	// (or LoadGame/SaveGame should be somewhere else entirely)
	static void SaveGame(const std::string &filename, Game *game);

	// start docked in station referenced by path
	Game(const SystemPath &path, double time = 0.0);

	// start at position relative to body referenced by path
	Game(const SystemPath &path, const vector3d &pos, double time = 0.0);

	// load game
	Game(Serializer::Reader &rd);

	~Game();

	// save game
	void Serialize(Serializer::Writer &wr);

	// various game states
	bool IsNormalSpace() const { return m_state == STATE_NORMAL; }
	bool IsHyperspace() const { return m_state == STATE_HYPERSPACE; }

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

	TimeAccel GetTimeAccel() const { return m_timeAccel; }
	TimeAccel GetRequestedTimeAccel() const { return m_requestedTimeAccel; }
	bool IsPaused() const { return m_timeAccel == TIMEACCEL_PAUSED; }

	float GetTimeAccelRate() const { return s_timeAccelRates[m_timeAccel]; }
	float GetInvTimeAccelRate() const { return s_timeInvAccelRates[m_timeAccel]; }

	float GetTimeStep() const { return s_timeAccelRates[m_timeAccel]*(1.0f/PHYSICS_HZ); }

	GameLog *log;

private:
	void CreateViews();
	void LoadViews(Serializer::Reader &rd);
	void DestroyViews();

	void SwitchToHyperspace();
	void SwitchToNormalSpace();

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
