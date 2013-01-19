// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include <list>
#include "HyperspaceCloud.h"
#include "MarketAgent.h"
#include "RefList.h"
#include "Ship.h"
#include "ShipController.h"
#include "galaxy/StarSystem.h"

namespace Graphics { class Renderer; }

class Player: public Ship, public MarketAgent {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Id shipId);
	Player() { }; //default constructor used before Load
	virtual void SetDockedWith(SpaceStation *, int port);
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	virtual bool FireMissile(int idx, Ship *target);
	virtual void SetAlertState(Ship::AlertState as);
	virtual void NotifyRemoved(const Body* const removedBody);

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

	virtual Ship::HyperjumpStatus StartHyperspaceCountdown(const SystemPath &dest);
	virtual void ResetHyperspaceCountdown();

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);

	virtual void OnEnterSystem();
	virtual void OnEnterHyperspace();

	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
};

#endif /* _PLAYER_H */
