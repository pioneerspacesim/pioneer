#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "sbre/sbre.h"
#include <vector>
class ObjMesh;

class ModelBody: public Body {
public:
	ModelBody();
	virtual ~ModelBody();
	void SetPosition(vector3d p);
	virtual void SetRotation(const matrix4x4d &r);
	// not valid to do SetVelocity on these. if you want them to move then use a DynamicBody
	vector3d GetPosition();
	void TransformToModelCoords(const Frame *camFrame);
	void ViewingRotation();
	void GetRotMatrix(matrix4x4d &m);
	virtual void SetFrame(Frame *f);
	void GeomsSetBody(dBodyID body);
	// to remove from simulation for a period
	virtual void Disable();
	virtual void Enable();
	
	void TriMeshUpdateLastPos();
	void SetModel(int sbreModel);

	void RenderSbreModel(const Frame *camFrame, int model, ObjParams *params);
	class Geom: public Object {
	public:
		virtual Type GetType() { return Object::GEOM; }
		Body *parent;
		int flags;
	};
protected:
	std::vector<Geom> geomColl;
private:
	std::vector<dGeomID> geoms;
	dReal triMeshTrans[32];
	int triMeshLastMatrixIndex;
};

#endif /* _MODELBODY_H */
