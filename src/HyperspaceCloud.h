// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _HYPERSPACECLOUD_H
#define _HYPERSPACECLOUD_H

#include "Body.h"

class Frame;
class Ship;
namespace Graphics {
	class Material;
	class Renderer;
	class VertexArray;
	class RenderState;
}

/** XXX TODO XXX Not applied to yet... */
#define HYPERCLOUD_DURATION (60.0*60.0*24.0*2.0)

class HyperspaceCloud: public Body {
public:
	OBJDEF(HyperspaceCloud, Body, HYPERSPACECLOUD);
	HyperspaceCloud(Ship *, double dateDue, bool isArrival);
	HyperspaceCloud();
	virtual ~HyperspaceCloud();
	virtual void SetVelocity(const vector3d &v) { m_vel = v; }
	virtual vector3d GetVelocity() const { return m_vel; }
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void PostLoadFixup(Space *space);
	virtual void TimeStepUpdate(const float timeStep);
	Ship *GetShip() { return m_ship; }
	Ship *EvictShip();
	double GetDueDate() const { return m_due; }
	void SetIsArrival(bool isArrival);
	bool IsArrival() const { return m_isArrival; }
	virtual void UpdateInterpTransform(double alpha);
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);

private:
	void InitGraphics();

	Ship *m_ship;
	vector3d m_vel;
	double m_birthdate;
	double m_due;
	bool m_isArrival;

	struct Graphic {
		std::unique_ptr<Graphics::VertexArray> vertices;
		std::unique_ptr<Graphics::Material> material;
		Graphics::RenderState *renderState;
	};
	Graphic m_graphic;
};

#endif /* _HYPERSPACECLOUD_H */
