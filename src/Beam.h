// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _BEAM_H
#define _BEAM_H

#include "Body.h"
#include "Color.h"
#include "matrix4x4.h"
#include "vector3.h"

class Camera;
class Space;

namespace Graphics {
	class Material;
	class Renderer;
	class MeshObject;
} // namespace Graphics

struct ProjectileData;

class Beam : public Body {
public:
	OBJDEF(Beam, Body, PROJECTILE);

	static void Add(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dir);

	Beam() = delete;
	Beam(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dir);
	Beam(const Json &jsonObj, Space *space);
	virtual ~Beam();
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override final;
	void TimeStepUpdate(const float timeStep) override final;
	void StaticUpdate(const float timeStep) override final;
	virtual void NotifyRemoved(const Body *const removedBody) override final;
	virtual void PostLoadFixup(Space *space) override final;
	virtual void UpdateInterpTransform(double alpha) override final;

	static void FreeModel();

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override final;

private:
	float GetDamage() const;
	double GetRadius() const;
	Body *m_parent;
	vector3d m_baseVel;
	vector3d m_dir;
	Color m_color;
	float m_baseDam;
	float m_length;
	float m_age;
	bool m_mining;
	bool m_active;

	int m_parentIndex; // deserialisation

	static void BuildModel();

	static std::unique_ptr<Graphics::MeshObject> s_sideMesh;
	static std::unique_ptr<Graphics::MeshObject> s_glowMesh;
	static std::unique_ptr<Graphics::Material> s_sideMat;
	static std::unique_ptr<Graphics::Material> s_glowMat;
};

#endif /* _BEAM_H */
