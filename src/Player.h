// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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
	Player(const ShipType::Id &shipId);
	Player() {}; //default constructor used before Load
	virtual void SetDockedWith(SpaceStation *, int port) override;
	virtual bool DoCrushDamage(float kgDamage) override final; // overloaded to add "crush" audio
	virtual bool OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData) override;
	virtual bool SetWheelState(bool down) override; // returns success of state change, NOT state itself
	virtual Missile * SpawnMissile(ShipType::Id missile_type, int power=-1) override;
	virtual void SetAlertState(Ship::AlertState as) override;
	virtual void NotifyRemoved(const Body* const removedBody) override;

	virtual void SetShipType(const ShipType::Id &shipId) override;

	PlayerShipController *GetPlayerController() const;
	//XXX temporary things to avoid causing too many changes right now
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetSetSpeedTarget() const;
	void SetCombatTarget(Body* const target, bool setSpeedTo = false);
	void SetNavTarget(Body* const target, bool setSpeedTo = false);
	void SetSetSpeedTarget(Body* const target);

	virtual Ship::HyperjumpStatus InitiateHyperjumpTo(const SystemPath &dest, int warmup_time, double duration, LuaRef checks) override;
	virtual void AbortHyperjump() override;

	// XXX cockpit is here for now because it has a physics component
	void InitCockpit();
	ShipCockpit* GetCockpit() const {return m_cockpit.get();}
	void OnCockpitActivated();

	virtual void StaticUpdate(const float timeStep) override;
	sigc::signal<void> onChangeEquipment;
	virtual vector3d GetManeuverVelocity() const;
	virtual int GetManeuverTime() const;

protected:
	virtual void SaveToJson(Json::Value &jsonObj, Space *space) override;
	virtual void LoadFromJson(const Json::Value &jsonObj, Space *space) override;

	virtual void OnEnterSystem() override;
	virtual void OnEnterHyperspace() override;

private:
	std::unique_ptr<ShipCockpit> m_cockpit;
};

#endif /* _PLAYER_H */
