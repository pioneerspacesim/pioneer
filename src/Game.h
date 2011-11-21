#ifndef _GAME_H
#define _GAME_H

// Game takes care of creating and destroying spaces as the player
// moves from system to hyperspace to system

#include "vector3.h"
#include "SystemPath.h"
#include "Serializer.h"

class Space;
class Player;
class HyperspaceCloud;

class Game {
public:
	// start docked in station referenced by path
	Game(const SystemPath &path);

	// start at position relative to body referenced by path
	Game(const SystemPath &path, const vector3d &pos);

	// load game
	Game(Serializer::Reader &rd);

	// save game
	void Serialize(Serializer::Writer &wr);

	// various game states
	bool IsNormalSpace() const { return m_state == STATE_NORMAL; }
	bool IsHyperspace() const { return m_state == STATE_HYPERSPACE; }

	Space *GetSpace() const { return m_space.Get(); }
	Player *GetPlayer() const { return m_player.Get(); }

	// request switch to hyperspace
	void WantHyperspace();

	// physics step
	void TimeStep(float step);

	// hyperspace parameters. only meaningful when IsHyperspace() is true
	float GetHyperspaceProgress() const { return m_hyperspaceProgress; }
	double GetHyperspaceDuration() const { return m_hyperspaceDuration; }
	double GetHyperspaceEndTime() const { return m_hyperspaceEndTime; }

private:
	void CreatePlayer();

	void SwitchToHyperspace();
	void SwitchToNormalSpace();

	ScopedPtr<Space> m_space;
	ScopedPtr<Player> m_player;

	enum State {
		STATE_NORMAL,
		STATE_HYPERSPACE,
	};
	State m_state;

	bool m_wantHyperspace;

	std::list<HyperspaceCloud*> m_hyperspaceClouds;
	SystemPath m_hyperspaceSource;
	double m_hyperspaceProgress;
	double m_hyperspaceDuration;
	double m_hyperspaceEndTime;
};

#endif
