#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include <list>
#include "HyperspaceCloud.h"
#include "MarketAgent.h"
#include "RefList.h"
#include "Ship.h"
#include "ShipController.h"
#include "StarSystem.h"

namespace Graphics { class Renderer; }

struct Mission : RefItem<Mission> {
	enum MissionState { // <enum scope='Mission' name=MissionStatus>
		ACTIVE,
		COMPLETED,
		FAILED,
	};

	std::string  type;
	std::string  client;
	SystemPath   location;
	double       due;
	Sint64       reward;
	MissionState status;
};

class Player: public Ship, public MarketAgent {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Type shipType);
	Player() { }; //default constructor used before Load
	virtual void SetDockedWith(SpaceStation *, int port);
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void OnHaveKilled(Body *guyWeKilled);
	int GetKillCount() const { return m_knownKillCount; }
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	virtual bool FireMissile(int idx, Ship *target);
	virtual void SetAlertState(Ship::AlertState as);
	virtual void NotifyRemoved(const Body* const removedBody);

	RefList<Mission> missions;

	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { assert(0); return 0; }
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const { return true; }
	Sint64 GetPrice(Equip::Type t) const;

	PlayerShipController *GetPlayerController() const;
	//XXX temporary things to avoid causing too many changes right now
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetSetSpeedTarget() const;
	void SetCombatTarget(Body* const target, bool setSpeedTo = false);
	void SetNavTarget(Body* const target, bool setSpeedTo = false);

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);

	virtual void OnEnterSystem();
	virtual void OnEnterHyperspace();

	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);

private:
	int m_killCount;
	int m_knownKillCount; // updated on docking
};

#endif /* _PLAYER_H */
