// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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
	virtual void SetLabel(const std::string &label) override;
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	virtual void TimeStepUpdate(const float timeStep) override;
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel) override;
	virtual bool OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData) override;
protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;
	virtual void LoadFromJson(const Json &jsonObj, Space *space) override;
private:
	void Init();
	LuaRef m_cargo;
	float m_hitpoints;
	float m_selfdestructTimer;
	bool m_hasSelfdestruct;
};

#endif /* _CARGOBODY_H */
