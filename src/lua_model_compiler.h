#ifndef _LUA_MODEL_COMPILER_H
#define _LUA_MODEL_COMPILER_H

// why? for struct CollMesh
#include "sbre/sbre.h"

// LMR = Lua Model Renderer
class LmrModel;
class GeomTree;

void LmrModelCompilerInit();
struct LmrModelNotFoundException {};
LmrModel *LmrLookupModelByName(const char *name) throw (LmrModelNotFoundException);
void LmrModelRender(LmrModel *m, const matrix4x4f &transform);
int LmrModelGetStatsTris();
void LmrModelClearStatsTris();

class LmrCollMesh
{
public:
	LmrCollMesh(LmrModel *m);
	~LmrCollMesh();

	const Aabb &GetAabb() const { return m_aabb; }
	float GetBoundingRadius() const { return m_radius; }
	GeomTree *geomTree;
	int nv, ni;
	float *pVertex;
	int *pIndex;
	int m_numTris; // ni/3
	int *pFlag; // 1 per tri
	friend class LmrModel;
private:
	Aabb m_aabb;
	float m_radius;
};


#endif /* _LUA_MODEL_COMPILER_H */
