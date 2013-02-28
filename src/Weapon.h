// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WEAPON_H
#define _WEAPON_H
/*
 * Gun used by ships (or why not other entities too)
 */
#include "libs.h"
#include "EquipType.h"
#include "Serializer.h"
#include "ShipType.h"

class Ship;
namespace Graphics { class Renderer; }
namespace SceneGraph { class Model; }

class Weapon {
public:
	Weapon(Equip::Type, Ship *s, const ShipType::GunMount&);
	virtual ~Weapon();

	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);

	void Render(Graphics::Renderer*, const matrix4x4f&);

	bool CanFire() const;
	bool Fire();
	void Update(float timeStep);

	const vector3d &GetPosition() const  { return m_position; }
	void SetPosition(const vector3d &v)  { m_position = v; }
	const vector3d &GetDirection() const { return m_direction; }
	void SetDirection(const vector3d &v) { m_direction = v; }

	float GetTemperature() const { return m_temperature; }

	void SetCoolingMultiplier(float mult) { m_coolingMultiplier = mult; }

private:
	friend class Ship;

	float m_recharge;
	float m_temperature;

	float m_coolingRate;
	float m_coolingMultiplier;

	Ship *m_ship;
	Equip::Type m_equipType;
	const LaserType *m_laserType;

	vector3d m_position;
	vector3d m_direction;
	std::vector<vector3d> m_muzzles;

	SceneGraph::Model *m_model; //not an instance
};

typedef std::vector<Weapon*>::iterator WeaponIterator;

#endif
