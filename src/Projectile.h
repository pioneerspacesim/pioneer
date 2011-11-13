#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "Body.h"
#include "EquipType.h"

class Frame;

class Projectile: public Body {
public:
	OBJDEF(Projectile, Body, PROJECTILE);

	static void Add(Body *parent, Equip::Type type, const vector3d &pos, const vector3d &baseVel, const vector3d &dirVel);

	Projectile();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const { return vector3d(m_orient[12], m_orient[13], m_orient[14]); }
	virtual double GetBoundingRadius() const { return 10; }
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	void TimeStepUpdate(const float timeStep);
	void StaticUpdate(const float timeStep);
	virtual void NotifyDeleted(const Body* const deletedBody);
	virtual void UpdateInterpolatedTransform(double alpha);
	virtual void PostLoadFixup();
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
private:
	float GetDamage() const;
	Body *m_parent;
	matrix4x4d m_orient;
	vector3d m_baseVel;
	vector3d m_dirVel;
	float m_age;
	int m_type;

	int m_parentIndex; // deserialisation
};

#endif /* _PROJECTILE_H */
