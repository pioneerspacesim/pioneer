// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "libs.h"
#include "Body.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"

struct ProjectileData {
	float lifespan;
	float damage;
	float length;
	float width;
	bool mining;
	float speed;
	Color color;
};


class Frame;
namespace Graphics {
	class Renderer;
	class VertexArray;
}

class Projectile: public Body {
public:
	OBJDEF(Projectile, Body, PROJECTILE);

	static void Add(Body *parent, float lifespan, float dam, float length, float width, bool mining, const Color& color, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);
	static void Add(Body *parent, const ProjectileData& prData, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel ) {
		Add( parent, prData.lifespan, prData.damage, prData.length, prData.width, prData.mining, prData.color, pos, baseVel, dirVel );
	}

	Projectile();
	virtual ~Projectile();
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	void TimeStepUpdate(const float timeStep) override;
	void StaticUpdate(const float timeStep) override;
	virtual void NotifyRemoved(const Body* const removedBody) override;
	virtual void UpdateInterpTransform(double alpha) override;
	virtual void PostLoadFixup(Space *space) override;

	static void FreeModel();

protected:
	virtual void SaveToJson(Json::Value &jsonObj, Space *space) override;
	virtual void LoadFromJson(const Json::Value &jsonObj, Space *space) override;

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

	static std::unique_ptr<Graphics::VertexArray> s_sideVerts;
	static std::unique_ptr<Graphics::VertexArray> s_glowVerts;
	static std::unique_ptr<Graphics::Material> s_sideMat;
	static std::unique_ptr<Graphics::Material> s_glowMat;
	static Graphics::RenderState *s_renderState;
};

#endif /* _PROJECTILE_H */
