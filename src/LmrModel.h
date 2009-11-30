#ifndef _LUA_MODEL_COMPILER_H
#define _LUA_MODEL_COMPILER_H

#include <map>

// LMR = Lua Model Renderer
class LmrGeomBuffer;
class LmrCollMesh;
class GeomTree;

#define LMR_MAX_LOD 4

#define LMR_ARG_MAX 40

struct LmrObjParams
{
	float argFloats[LMR_ARG_MAX];

	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0

//	struct Material pColor[3];

	char argStrings[3][256];
};

struct RenderState;

class LmrModel {
public:
	LmrModel(const char *model_name);
	virtual ~LmrModel();
	void Render(const matrix4x4f &trans, const LmrObjParams *params);
	void Render(const RenderState *rstate, const vector3f &cameraPos, const matrix4x4f &trans, const LmrObjParams *params);
	void GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform, const LmrObjParams *params);
private:
	void Build(int lod, const LmrObjParams *params);
	struct Material {
		Material() {}
		float diffuse[4];
		float specular[4];
		float shininess;
		float emissive[4];
	};

	// index into m_materials
	std::map<std::string, int> m_materialLookup;
	std::vector<Material> m_materials;
	float m_lodPixelSize[LMR_MAX_LOD];
	int m_numLods;
	LmrGeomBuffer *m_staticGeometry[LMR_MAX_LOD];
	LmrGeomBuffer *m_dynamicGeometry[LMR_MAX_LOD];
	std::string m_name;
	bool m_hasDynamicFunc;
	float m_boundingRadius;
	friend class LmrGeomBuffer;
};

void LmrModelCompilerInit();
struct LmrModelNotFoundException {};
LmrModel *LmrLookupModelByName(const char *name) throw (LmrModelNotFoundException);
void LmrModelRender(LmrModel *m, const matrix4x4f &transform);
int LmrModelGetStatsTris();
void LmrModelClearStatsTris();
void LmrNotifyScreenWidth(float width);

class LmrCollMesh
{
public:
	LmrCollMesh(LmrModel *m, const LmrObjParams *params);
	~LmrCollMesh();

	const Aabb &GetAabb() const { return m_aabb; }
	float GetBoundingRadius() const { return m_radius; }
	GeomTree *geomTree;
	// num vertices, num indices, num flags
	int nv, ni, nf;
	float *pVertex;
	int *pIndex;
	int m_numTris; // ni/3
	int *pFlag; // 1 per tri
	friend class LmrModel;
	friend class LmrGeomBuffer;
private:
	Aabb m_aabb;
	float m_radius;
};


#endif /* _LUA_MODEL_COMPILER_H */
