// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUNMOUNT_H
#define _GUNMOUNT_H
#include "vector3.h"
#include "EquipType.h"
#include "Serializer.h"

class Body;
class Ship;

struct TurretData {
	vector3d pos;
	vector3d dir;
	
	double extent;		// maximum angle from dir in radians
	double accel;
	double maxspeed;	// max training speed & accel in radians

	std::string name;
};

class Turret
{
  public:
	Turret() {}
	Turret(Ship *parent, const TurretData &turret);

	bool IsFiring() const { return m_firing; }
	void SetFiring(bool firing) { m_firing = firing; }
	void Update(float timeStep);			// timestep process

	void SetWeapon(Equip::Type weapontype, float coolfactor);
	Equip::Type GetWeapon() const { return m_weapontype; }

	float GetTemperature() const { return m_temperature; }
	const vector3d& GetPos() const { return m_turret.pos; }
	const vector3d& GetDir() const { return m_curdir; }
	const std::string& GetName() const { return m_turret.name; }

	void SetSkill(float skill) { m_skill = skill; }
	void SetTarget(Body *target) { m_target = target; m_leadTime = 0.0; }
	void OnDeleted(const Body *body) { if (body == m_target) m_target = 0; }

	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

  private:
	void FaceDirectionInternal(const vector3d &dir, double av);
	void MatchAngVelInternal(const vector3d &av);
	void AutoTarget(float timeStep);

	TurretData m_turret;

	Equip::Type m_weapontype;
	float m_coolrate;

	bool m_firing;			// needs saving
	float m_temperature;	// needs saving
	float m_recharge;		// needs saving
	
	Ship *m_parent;			// passed in constructor, used when creating 	
							// can be used to get pos/dir/size

	double m_dotextent;
	vector3d m_curdir;
	vector3d m_curvel;
	Body *m_target;

	float m_skill;				// skill of turret controller
	double m_leadTime;			// time to next update target heading
	vector3d m_leadOffset;
	vector3d m_leadDrift;
};

#endif // _GUNMOUNT_H


