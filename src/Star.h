#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"
#include "StarSystem.h"

class Frame;

class Star: public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star(SBody *sbody);
	Star();
	virtual ~Star() {};
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	virtual double GetBoundingRadius() const { return radius*2.0f; }
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual double GetMass() const { return mass; }
	virtual const SBody *GetSBody() const { return m_sbody; }
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
private:
	SBody *m_sbody;
	vector3d pos;
	double radius;
	double mass;
};

#endif /* _STAR_H */
