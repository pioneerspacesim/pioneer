// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUA_MODEL_COMPILER_H
#define _LUA_MODEL_COMPILER_H

#include <map>
#include <vector>
#include <sigc++/sigc++.h>
#include "CollMesh.h"
#include "ModelBase.h"
#include "LmrTypes.h"

// LMR = Lua Model Renderer
namespace Graphics { class Renderer; }
class LmrGeomBuffer;
class LmrCollMesh;
struct RenderState;
struct lua_State;

#define LMR_MAX_LOD 4

struct LmrLight {
	float position[4];
	float color[4];
	float quadraticAttenuation;
};

class LmrModel : public ModelBase {
public:
	LmrModel(const char *model_name);
	virtual ~LmrModel();
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, LmrObjParams *params);
	void Render(const RenderState *rstate, const vector3f &cameraPos, const matrix4x4f &trans, LmrObjParams *params);
	virtual RefCountedPtr<CollMesh> CreateCollisionMesh(const LmrObjParams *p);
	void GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform, const LmrObjParams *params);
	virtual float GetDrawClipRadius() const { return m_drawClipRadius; }
	float GetFloatAttribute(const char *attr_name) const;
	int GetIntAttribute(const char *attr_name) const;
	bool GetBoolAttribute(const char *attr_name) const;
	void PushAttributeToLuaStack(const char *attr_name) const;
	const char *GetName() const { return m_name.c_str(); }
	bool HasTag(const char *tag) const;
	std::string GetDumpPath(const char *pMainFolderName=0);
	void Dump(const LmrObjParams *params, const char* pMainFolderName=0);
	void SetLabel(const std::string &label) { m_label = label; }
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
	std::string m_label;
	friend class LmrGeomBuffer;

	bool m_dumped;
};

void LmrModelCompilerInit(Graphics::Renderer *r);
void LmrModelCompilerUninit();
struct LmrModelNotFoundException {};
LmrModel *LmrLookupModelByName(const char *name);
void LmrModelRender(LmrModel *m, const matrix4x4f &transform);
int LmrModelGetStatsTris();
void LmrModelClearStatsTris();
void LmrGetModelsWithTag(const char *tag, std::vector<LmrModel*> &outModels);
void LmrGetAllModelNames(std::vector<std::string> &modelNames);
lua_State *LmrGetLuaState();

class LmrCollMesh : public CollMesh
{
public:
	LmrCollMesh(LmrModel *m, const LmrObjParams *params);
	~LmrCollMesh();

	int GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const;

	// num vertices, num indices, num flags
	int nv, ni, nf;
	float *pVertex;
	int *pIndex;
	int m_numTris; // ni/3
	unsigned int *pFlag; // 1 per tri
	friend class LmrModel;
	friend class LmrGeomBuffer;
};


#endif /* _LUA_MODEL_COMPILER_H */
