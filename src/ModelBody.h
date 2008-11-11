#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "sbre/sbre.h"
#include <vector>
class ObjMesh;
class CollMeshSet;
class Geom;

class ModelBody: public Body {
public:
	OBJDEF(ModelBody, Body, MODELBODY);
	ModelBody();
	virtual ~ModelBody();
	void SetPosition(vector3d p);
	virtual void SetRotMatrix(const matrix4x4d &r);
	vector3d GetPosition() const;
	virtual double GetRadius() const;
	void TransformToModelCoords(const Frame *camFrame);
	void GetRotMatrix(matrix4x4d &m) const;
	virtual void SetFrame(Frame *f);
	// to remove from simulation for a period
	virtual void Disable();
	virtual void Enable();
	void GetAabb(Aabb &aabb);
	
	void TriMeshUpdateLastPos(const matrix4x4d &currentTransform);
	void SetModel(int sbreModel);

	void RenderSbreModel(const Frame *camFrame, int model, ObjParams *params);
	class GeomBit: public Object {
	public:
		OBJDEF(GeomBit, Object, GEOM);
		Body *parent;
		int flags;
	};
protected:
	virtual void Save();
	virtual void Load();
private:
	CollMeshSet *m_collMeshSet;
	Geom *m_geom;
};

#endif /* _MODELBODY_H */
