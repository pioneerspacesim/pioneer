// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _CARGOBODY_H
#define _CARGOBODY_H

#include "DynamicBody.h"
#include "lua/LuaRef.h"

namespace Graphics {
	class Renderer;
}

class CargoBody : public DynamicBody {
public:
	OBJDEF(CargoBody, DynamicBody, CARGOBODY);
	CargoBody() = delete;
	CargoBody(const LuaRef &cargo, float selfdestructTimer = 86400.0f, uint8_t quantity = 1);			   // default to 24 h lifetime, and single quantity
	CargoBody(const char *modelName, const LuaRef &cargo, float selfdestructTimer = 86400.0f, uint8_t quantity = 1); // default to 24 h lifetime, and single quantity
	CargoBody(const Json &jsonObj, Space *space);
	LuaRef GetCargoType() const { return m_cargo; }
	uint8_t GetCargoQuantity() const { return m_quantity; }
	void SetLabel(const std::string &label) override;
	void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	void TimeStepUpdate(const float timeStep) override;
	bool OnCollision(Body *o, Uint32 flags, double relVel) override;
	bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) override;

	~CargoBody(){};

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;

private:
	void Init();
	LuaRef m_cargo;
	float m_hitpoints;
	float m_selfdestructTimer;
	uint8_t m_quantity; // how many instaces of the cargo does this contain
};

#endif /* _CARGOBODY_H */
