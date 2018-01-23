// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _BEAM_H
#define _BEAM_H

#include "libs.h"
#include "Body.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"

class Frame;
namespace Graphics {
	class Renderer;
	class VertexArray;
}
struct ProjectileData;

class Beam: public Body {
public:
	OBJDEF(Beam, Body, PROJECTILE);

	static void Add(Body *parent, const ProjectileData& prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dir);

	Beam();
	virtual ~Beam();
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override final;
	void TimeStepUpdate(const float timeStep) override final;
	void StaticUpdate(const float timeStep) override final;
	virtual void NotifyRemoved(const Body* const removedBody) override final;
	virtual void PostLoadFixup(Space *space) override final;
	virtual void UpdateInterpTransform(double alpha) override final;

	static void FreeModel();

protected:
	virtual void SaveToJson(Json::Value &jsonObj, Space *space) override final;
	virtual void LoadFromJson(const Json::Value &jsonObj, Space *space) override final;

private:
	float GetDamage() const;
	double GetRadius() const;
	Body *m_parent;
	vector3d m_baseVel;
	vector3d m_dir;
	float m_baseDam;
	float m_length;
	bool m_mining;
	Color m_color;
	bool m_canKill;

	int m_parentIndex; // deserialisation

	static void BuildModel();

	static std::unique_ptr<Graphics::VertexArray> s_sideVerts;
	static std::unique_ptr<Graphics::VertexArray> s_glowVerts;
	static std::unique_ptr<Graphics::Material> s_sideMat;
	static std::unique_ptr<Graphics::Material> s_glowMat;
	static Graphics::RenderState *s_renderState;
	Graphics::Drawables::Line3D m_line;

/*

Space::AddLaserBeam(GetFrame(), pos, dir, 10000.0, this, damage);


// Assumed to be at model coords
void Ship::RenderLaserfire()
{
	static const GLfloat fogDensity = 0.001;
	static const GLfloat fogColor[4] = { 0,0,0,1.0 };
	const ShipType &stype = GetShipType();
	glDisable(GL_LIGHTING);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glHint(GL_FOG_HINT, GL_NICEST);
	
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		if (!m_gunState[i]) continue;
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
		switch (m_equipment.Get(Equip::SLOT_LASER, i)) {
			case Equip::LASER_2MW_BEAM:
				glColor3f(1,.5,0); break;
			case Equip::LASER_4MW_BEAM:
				glColor3f(1,1,0); break;
			default:
			case Equip::LASER_1MW_BEAM:
				glColor3f(1,0,0); break;
		}
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		vector3f pos = stype.gunMount[i].pos;
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3fv(&((10000)*stype.gunMount[i].dir)[0]);
		glEnd();
		glPopAttrib();
	}
	glDisable(GL_FOG);
	glEnable(GL_LIGHTING);
}

	
*/
};

#endif /* _BEAM_H */
