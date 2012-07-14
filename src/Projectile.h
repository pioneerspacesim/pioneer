#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Body.h"
#include "EquipType.h"
#include "graphics/Material.h"
#include "SmartPtr.h"

class Frame;
namespace Graphics {
	class Renderer;
	class VertexArray;
}

class Projectile: public Body {
public:
	OBJDEF(Projectile, Body, PROJECTILE);

	static void Add(Body *parent, Equip::Type type, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);

	Projectile();
	virtual ~Projectile();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const { return vector3d(m_orient[12], m_orient[13], m_orient[14]); }
	virtual double GetBoundingRadius() const { return m_radius; }
	virtual void Render(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	void TimeStepUpdate(const float timeStep);
	void StaticUpdate(const float timeStep);
	virtual void NotifyRemoved(const Body* const removedBody);
	virtual void UpdateInterpolatedTransform(double alpha);
	virtual void PostLoadFixup(Space *space);
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	float GetDamage() const;
	double GetRadius() const;
	Body *m_parent;
	matrix4x4d m_orient;
	vector3d m_baseVel;
	vector3d m_dirVel;
	float m_age;
	int m_type;
	double m_radius;

	int m_parentIndex; // deserialisation

	ScopedPtr<Graphics::VertexArray> m_sideVerts;
	ScopedPtr<Graphics::VertexArray> m_glowVerts;
	Graphics::Material m_sideMat;
	Graphics::Material m_glowMat;
};

#endif /* _PROJECTILE_H */
