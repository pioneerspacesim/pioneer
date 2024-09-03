// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _HYPERSPACECLOUD_H
#define _HYPERSPACECLOUD_H

#include "Body.h"

class Frame;
class Ship;

namespace Graphics {
	class Material;
	class MeshObject;
	class Renderer;
} // namespace Graphics

class HyperspaceCloud : public Body {
public:
	OBJDEF(HyperspaceCloud, Body, HYPERSPACECLOUD);
	HyperspaceCloud() = delete;
	HyperspaceCloud(Ship *, double dateDue, bool isArrival);
	HyperspaceCloud(const Json &jsonObj, Space *space);
	virtual ~HyperspaceCloud();
	virtual void SetVelocity(const vector3d &v) override { m_vel = v; }
	virtual vector3d GetVelocity() const override { return m_vel; }
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	virtual void PostLoadFixup(Space *space) override;
	virtual void TimeStepUpdate(const float timeStep) override;
	Ship *GetShip() { return m_ship; }
	Ship *EvictShip();
	double GetDueDate() const { return m_due; }
	void SetIsArrival(bool isArrival);
	bool IsArrival() const { return m_isArrival; }
	virtual void UpdateInterpTransform(double alpha) override;

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;

private:
	static void InitGraphics(Graphics::Renderer *renderer);

	Ship *m_ship;
	vector3d m_vel;
	double m_birthdate;
	double m_due;
	bool m_isArrival;
	bool m_isBeingKilled;

	static std::unique_ptr<Graphics::Material> s_cloudMat;
	static std::unique_ptr<Graphics::MeshObject> s_cloudMeshArriving;
	static std::unique_ptr<Graphics::MeshObject> s_cloudMeshLeaving;
};

#endif /* _HYPERSPACECLOUD_H */
