#ifndef _LUA_MODEL_COMPILER_H
#define _LUA_MODEL_COMPILER_H

#include <map>
#include <vector>
#include <sigc++/sigc++.h>
#include "MyLuaMathTypes.h"

// LMR = Lua Model Renderer
class LmrGeomBuffer;
class LmrCollMesh;
class GeomTree;

namespace Graphics { class Renderer; }

class EquipSet;

#define LMR_MAX_LOD 4

struct LmrMaterial {
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;
	// make sure save and load routines in ShipFlavour are matching
};

struct LmrLight {
	float position[4];
	float color[4];
	float quadraticAttenuation;
};

struct LmrObjParams
{
	enum { LMR_ANIMATION_MAX = 10 };

	const char *animationNamespace; // the namespace to look up animation names in, from LuaConstants

	double time;
	int animStages[LMR_ANIMATION_MAX];
	double animValues[LMR_ANIMATION_MAX];
	const char *label;
	const EquipSet *equipment; // for ships
	int flightState;

	float linthrust[3];		// 1.0 to -1.0
	float angthrust[3];		// 1.0 to -1.0

	struct LmrMaterial pMat[3];
};

struct RenderState;
class LmrCollMesh;

class LmrModel {
public:
	LmrModel(const char *model_name);
	virtual ~LmrModel();
	void Render(const matrix4x4f &trans, const LmrObjParams *params);
	void Render(const RenderState *rstate, const vector3f &cameraPos, const matrix4x4f &trans, const LmrObjParams *params);
	void GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform, const LmrObjParams *params);
	float GetDrawClipRadius() const { return m_drawClipRadius; }
	float GetFloatAttribute(const char *attr_name) const;
	int GetIntAttribute(const char *attr_name) const;
	bool GetBoolAttribute(const char *attr_name) const;
	void PushAttributeToLuaStack(const char *attr_name) const;
	const char *GetName() const { return m_name.c_str(); }
	bool HasTag(const char *tag) const;
private:
	void Build(int lod, const LmrObjParams *params);

	// index into m_materials
	std::map<std::string, int> m_materialLookup;
	std::vector<LmrMaterial> m_materials;
	std::vector<LmrLight> m_lights;
	float m_lodPixelSize[LMR_MAX_LOD];
	int m_numLods;
	LmrGeomBuffer *m_staticGeometry[LMR_MAX_LOD];
	LmrGeomBuffer *m_dynamicGeometry[LMR_MAX_LOD];
	std::string m_name;
	bool m_hasDynamicFunc;
	// only used for lod pixel size at the moment
	float m_drawClipRadius;
	float m_scale;
	friend class LmrGeomBuffer;
};

class TextureCache;
void LmrModelCompilerInit(Graphics::Renderer *r, TextureCache *textureCache);
void LmrModelCompilerUninit();
struct LmrModelNotFoundException {};
LmrModel *LmrLookupModelByName(const char *name);
void LmrModelRender(LmrModel *m, const matrix4x4f &transform);
int LmrModelGetStatsTris();
void LmrModelClearStatsTris();
void LmrNotifyScreenWidth(float width);
void LmrGetModelsWithTag(const char *tag, std::vector<LmrModel*> &outModels);
lua_State *LmrGetLuaState();

class LmrCollMesh
{
public:
	LmrCollMesh(LmrModel *m, const LmrObjParams *params);
	~LmrCollMesh();

	const Aabb &GetAabb() const { return m_aabb; }
	float GetBoundingRadius() const { return m_radius; }
	int GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const;
	GeomTree *geomTree;
	// num vertices, num indices, num flags
	int nv, ni, nf;
	float *pVertex;
	int *pIndex;
	int m_numTris; // ni/3
	unsigned int *pFlag; // 1 per tri
	friend class LmrModel;
	friend class LmrGeomBuffer;
private:
	Aabb m_aabb;
	float m_radius;
};


#endif /* _LUA_MODEL_COMPILER_H */
