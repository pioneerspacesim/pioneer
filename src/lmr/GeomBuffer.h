#ifndef _LMRGEOMBUFFER_H
#define _LMRGEOMBUFFER_H

#include "libs.h"
#include "SmartPtr.h"

#include "BufferObject.h"
#include "ShipThruster.h"
#include "Utils.h"

class LmrModel;
class LmrObjParams;
class LmrCollMesh;
class ModelTexture;
class BillboardTexture;
namespace Graphics {
	class Renderer;
	class Texture;
}

namespace LMR {

class GeomBuffer {
public:
	GeomBuffer(LmrModel *model, bool isStatic);

	int GetIndicesPos() const { return m_indices.size(); }
	int GetVerticesPos() const { return m_vertices.size(); }

	void SetGeomFlag(Uint16 flag) { curTriFlag = flag; }
	Uint16 GetGeomFlag() const { return curTriFlag; }

	void PreBuild();
	void PostBuild();

	void FreeGeometry();

	void Render(Graphics::Renderer *r, const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params);
	void RenderThrusters(Graphics::Renderer *r, const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params);

	void PushThruster(const vector3f &pos, const vector3f &dir, const float power, bool linear_only);

	int PushVertex(const vector3f &pos, const vector3f &normal);
	void SetVertex(int idx, const vector3f &pos, const vector3f &normal);

	int PushVertex(const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v);
	void SetVertex(int idx, const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v);

	void SetTexture(const char *tex);
	void SetGlowMap(const char *tex);

	void SetTexMatrix(const matrix4x4f &texMatrix) { curTexMatrix = texMatrix; } 

	void PushTri(int i1, int i2, int i3);

	void SetInsideOut(bool a) { m_putGeomInsideout = a; }
	
	void PushZBias(float amount, const vector3f &pos, const vector3f &norm);

	void PushSetLocalLighting(bool enable);

	void SetLight(int num, float quadratic_attenuation, const vector3f &pos, const vector3f &col);

	void PushUseLight(int num);

	void PushCallModel(LmrModel *m, const matrix4x4f &transform, float scale);

	void PushInvisibleTri(int i1, int i2, int i3);
	
	void PushBillboards(const char *texname, const float size, const vector3f &color, const int numPoints, const vector3f *points);

	void SetMaterial(const char *mat_name, const float mat[11]);

	void PushUseMaterial(const char *mat_name);

	/* return start vertex index */
	int AllocVertices(int num);

	const vector3f &GetVertex(int num) const { return m_vertices[num].v; }

	void GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform, const LmrObjParams *params);

	void SaveToCache(FILE *f);
	void LoadFromCache(FILE *f);

	static int GetStatsTris() { return s_numTrisRendered; }
	static void ClearStatsTris() { s_numTrisRendered = 0; }

private:
	void BindBuffers();

	void OpDrawElements(int numIndices);

	void PushCurOp() { m_ops.push_back(curOp); }

	void PushIdx(Uint16 v);

	enum OpType { OP_NONE, OP_DRAW_ELEMENTS, OP_DRAW_BILLBOARDS, OP_SET_MATERIAL, OP_ZBIAS,
			OP_CALL_MODEL, OP_LIGHTING_TYPE, OP_USE_LIGHT };

	struct Op {
		enum OpType type;
		union {
			struct { std::string *textureFile; mutable Graphics::Texture *texture; std::string *glowmapFile; mutable Graphics::Texture *glowmap; int start, count, elemMin, elemMax; } elems;
			struct { int material_idx; } col;
			struct { float amount; float pos[3]; float norm[3]; } zbias;
			struct { LmrModel *model; float transform[16]; float scale; } callmodel;
			struct { std::string *textureFile; mutable Graphics::Texture *texture; int start, count; float size; float col[4]; } billboards;
			struct { bool local; } lighting_type;
			struct { int num; float quadratic_attenuation; float pos[4], col[4]; } light;
		};
	};

	SHADER_CLASS_BEGIN(LmrShader)
		SHADER_UNIFORM_INT(usetex)
		SHADER_UNIFORM_INT(useglow)
		SHADER_UNIFORM_SAMPLER(tex)
		SHADER_UNIFORM_SAMPLER(texGlow)
	SHADER_CLASS_END()

	void UseProgram(LmrShader *shader, bool Textured = false, bool Glowmap = false);

	static void StaticInit(Graphics::Renderer *renderer);

	static BufferObjectPool<sizeof(Vertex)> s_staticBufferPool;
	static int s_numTrisRendered;

	static bool s_initialized;
	static ScopedPtr<LmrShader> s_sunlightShader[4];
	static ScopedPtr<LmrShader> s_pointlightShader[4];

	/* this crap is only used at build time... could move this elsewhere */
	Op curOp;
	Uint16 curTriFlag;
	std::string *curTexture;
	std::string *curGlowmap;
	matrix4x4f curTexMatrix;

	std::vector<Vertex> m_vertices;
	std::vector<Uint16> m_indices;
	std::vector<Uint16> m_triflags;
	std::vector<Op> m_ops;
	std::vector<ShipThruster::Thruster> m_thrusters;
	LmrModel *m_model;
	int m_boIndexBase;
	BufferObject<sizeof(Vertex)> *m_bo;
	bool m_isStatic;
	bool m_putGeomInsideout;
};

}

#endif
