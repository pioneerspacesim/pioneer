// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CARGOBODY_H
#define _CARGOBODY_H

#include "libs.h"
#include "DynamicBody.h"
#include "LuaRef.h"

namespace Graphics { class Renderer; }

class CargoBody: public DynamicBody {
public:
	OBJDEF(CargoBody, DynamicBody, CARGOBODY);
	CargoBody(const LuaRef& cargo, float selfdestructTimer=86400.0f); // default to 24 h lifetime
	CargoBody() {}
	LuaRef GetCargoType() const { return m_cargo; }
	virtual void SetLabel(const std::string &label);
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void TimeStepUpdate(const float timeStep);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData);
protected:
	virtual void SaveToJson(Json::Value &jsonObj, Space *space);
	virtual void LoadFromJson(const Json::Value &jsonObj, Space *space);
private:
	void Init();
	LuaRef m_cargo;
	float m_hitpoints;
	float m_selfdestructTimer;
	bool m_hasSelfdestruct;
};

#endif /* _CARGOBODY_H */
