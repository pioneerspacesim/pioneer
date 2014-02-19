// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include <list>
#include "HyperspaceCloud.h"
#include "Ship.h"
#include "ShipController.h"
#include "ShipCockpit.h"
#include "galaxy/StarSystem.h"

namespace Graphics { class Renderer; }

class Player: public Ship {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Id shipId);
	Player() {}; //default constructor used before Load
	virtual void SetDockedWith(SpaceStation *, int port);
	virtual bool OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData);
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	virtual Missile * SpawnMissile(ShipType::Id missile_type, int power=-1);
	virtual void SetAlertState(Ship::AlertState as);
	virtual void NotifyRemoved(const Body* const removedBody);

	PlayerShipController *GetPlayerController() const;
	//XXX temporary things to avoid causing too many changes right now
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetSetSpeedTarget() const;
	void SetCombatTarget(Body* const target, bool setSpeedTo = false);
	void SetNavTarget(Body* const target, bool setSpeedTo = false);

	virtual Ship::HyperjumpStatus InitiateHyperjumpTo(const SystemPath &dest, int warmup_time, double duration, LuaRef checks);
	virtual void AbortHyperjump();
	virtual Ship::HyperjumpStatus StartHyperspaceCountdown(const SystemPath &dest);
	virtual void ResetHyperspaceCountdown();

	// XXX cockpit is here for now because it has a physics component
	void InitCockpit();
	ShipCockpit* GetCockpit() const {return m_cockpit.get();}
	void OnCockpitActivated();

	virtual void StaticUpdate(const float timeStep);

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);

	virtual void OnEnterSystem();
	virtual void OnEnterHyperspace();

private:
	std::unique_ptr<ShipCockpit> m_cockpit;
};

#endif /* _PLAYER_H */
