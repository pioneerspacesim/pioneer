// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PLAYER_H
#define _PLAYER_H

#include "Ship.h"
#include "ShipCockpit.h"

class PlayerShipController;

namespace Graphics {
	class Renderer;
}

class Player : public Ship {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player() = delete;
	Player(const Json &jsonObj, Space *space);
	Player(const ShipType::Id &shipId);

	void SetDockedWith(SpaceStation *, int port) override;
	bool DoDamage(float kgDamage) final; // overloaded to add "crush" audio
	bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) override;
	bool SetWheelState(bool down) override; // returns success of state change, NOT state itself
	Missile *SpawnMissile(ShipType::Id missile_type, int power = -1) override;
	void SetAlertState(Ship::AlertState as) override;
	void NotifyRemoved(const Body *const removedBody) override;
	bool ManualDocking() const override { return !AIIsActive(); }

	void DoFixspeedTakeoff(SpaceStation *from = nullptr);

	void SetShipType(const ShipType::Id &shipId) override;

	PlayerShipController *GetPlayerController() const;
	//XXX temporary things to avoid causing too many changes right now
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetFollowTarget() const;
	void SetCombatTarget(Body *const target, bool setFollowTo = false);
	void SetNavTarget(Body *const target);
	void SetFollowTarget(Body *const target);
	void ChangeCruiseSpeed(double delta);

	Ship::HyperjumpStatus InitiateHyperjumpTo(const SystemPath &dest, int warmup_time, double duration, const HyperdriveSoundsTable &sounds, LuaRef checks) override;
	void AbortHyperjump() override;

	// XXX cockpit is here for now because it has a physics component
	void InitCockpit();
	ShipCockpit *GetCockpit() const { return m_cockpit.get(); }
	void OnCockpitActivated();

	void StaticUpdate(const float timeStep) override;
	virtual vector3d GetManeuverVelocity() const;
	virtual int GetManeuverTime() const;

protected:
	void SaveToJson(Json &jsonObj, Space *space) override;

	void OnEnterSystem() override;
	void OnEnterHyperspace() override;

private:
	std::unique_ptr<ShipCockpit> m_cockpit;
	Sound::Event m_creakSound;
	vector3d m_atmosAccel;
	vector3d m_atmosJerk;
};

#endif /* _PLAYER_H */
