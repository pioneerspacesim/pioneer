// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Body.h"
#include "Color.h"

struct ProjectileData {
	ProjectileData() :
		lifespan(0.0f),
		damage(0.0f),
		length(0.0f),
		width(0.0f),
		speed(0.0f),
		color(Color::WHITE),
		mining(false),
		beam(false) {}
	float lifespan;
	float damage;
	float length;
	float width;
	float speed;
	Color color;
	bool mining;
	bool beam;
};

class Frame;

namespace Graphics {
	class Material;
	class Renderer;
	class MeshObject;
} // namespace Graphics

class Projectile : public Body {
public:
	OBJDEF(Projectile, Body, PROJECTILE);

	static void Add(Body *parent, float lifespan, float dam, float length, float width, bool mining, const Color &color, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);
	static void Add(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);

	Projectile() = delete;
	Projectile(Body *parent, const ProjectileData &prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);
	Projectile(const Json &jsonObj, Space *space);
	virtual ~Projectile();
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override final;
	void TimeStepUpdate(const float timeStep) override final;
	void StaticUpdate(const float timeStep) override final;
	virtual void NotifyRemoved(const Body *const removedBody) override final;
	virtual void UpdateInterpTransform(double alpha) override final;
	virtual void PostLoadFixup(Space *space) override final;

	static void FreeModel();

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override final;

private:
	float GetDamage() const;
	double GetRadius() const;
	Body *m_parent;
	vector3d m_baseVel;
	vector3d m_dirVel;
	float m_age;
	float m_lifespan;
	float m_baseDam;
	float m_length;
	float m_width;
	bool m_mining;
	Color m_color;

	int m_parentIndex; // deserialisation

	static void BuildModel();

	static std::unique_ptr<Graphics::MeshObject> s_sideMesh;
	static std::unique_ptr<Graphics::MeshObject> s_glowMesh;
	static std::unique_ptr<Graphics::Material> s_sideMat;
	static std::unique_ptr<Graphics::Material> s_glowMat;
};

#endif /* _PROJECTILE_H */
