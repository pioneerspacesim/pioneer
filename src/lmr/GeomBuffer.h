#ifndef _LMRGEOMBUFFER_H
#define _LMRGEOMBUFFER_H

#include "libs.h"
#include "SmartPtr.h"

#include "graphics/StaticMesh.h"
#include "graphics/Surface.h"

#include "BufferObject.h"
#include "ShipThruster.h"
#include "Utils.h"

class LmrModel;
class LmrObjParams;
class LmrCollMesh;
namespace Graphics {
	class Renderer;
	class Texture;
	class VertexArray;
}

namespace LMR {

class GeomBuffer {
public:
	GeomBuffer(LmrModel *model, bool isStatic, Graphics::Renderer *renderer);

	//int GetIndicesPos() const { return m_indices.size(); }
	int GetVerticesPos() const { return m_curSurface->GetVertices()->position.size(); } // XXX direct access again

	void SetGeomFlag(Uint16 flag) { curTriFlag = flag; }
	Uint16 GetGeomFlag() const { return curTriFlag; }

	void PreBuild();
	void PostBuild();

	void FreeGeometry();

	void Render(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params);
	void RenderThrusters(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params);

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
	
	void PushZBias(float amount);

	void PushSetLocalLighting(bool enable);

	void SetLight(int num, float quadratic_attenuation, const vector3f &pos, const vector3f &col);

	void PushUseLight(int num);

	void PushCallModel(LmrModel *m, const matrix4x4f &transform, float scale);

	void PushInvisibleTri(int i1, int i2, int i3);
	
	void PushBillboards(const char *texname, const float size, const Color &color, const int numPoints, const vector3f *points);

	void SetMaterial(const char *mat_name, const float mat[11]);

	void PushUseMaterial(const char *mat_name);

	/* return start vertex index */
	int AllocVertices(int num);

	//const vector3f &GetVertex(int num) const { return m_vertices[num].v; }
	const vector3f &GetVertex(int num) const { return m_curSurface->GetVertices()->position[num]; } // XXX direct access again

	void GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform, const LmrObjParams *params);

	void SaveToCache(FILE *f);
	void LoadFromCache(FILE *f);

	static int GetStatsTris() { return s_numTrisRendered; }
	static void ClearStatsTris() { s_numTrisRendered = 0; }

private:
	void BindBuffers();

	enum OpType { OP_DRAW_BILLBOARDS, OP_ZBIAS, OP_CALL_MODEL, OP_LIGHTING_TYPE, OP_USE_LIGHT };

	struct Op {
		Op(OpType _type) : type(_type) {}
		const OpType type;
	};

	struct OpDrawBillboards : public Op {
		OpDrawBillboards() : Op(OP_DRAW_BILLBOARDS), textureFile(0), texture(0), size(0), col(0.0f) {}
		std::string *textureFile;
		mutable Graphics::Texture *texture;
		std::vector<vector3f> positions;
		float size;
		Color col;
	};

	struct OpZBias : public Op {
		OpZBias(float _amount) : Op(OP_ZBIAS), amount(_amount) {}
		const float amount;
	};

	struct OpCallModel : public Op {
		OpCallModel(LmrModel *_model, const matrix4x4f &_transform, float _scale) : Op(OP_CALL_MODEL), model(_model), transform(_transform), scale(_scale) {}
		LmrModel *model;
		const matrix4x4f transform;
		const float scale;
	};

	struct OpLightingType : public Op {
		OpLightingType(bool _local) : Op(OP_LIGHTING_TYPE), local(_local) {}
		const bool local;
	};

	struct OpUseLight : public Op {
		OpUseLight(int _num) : Op(OP_USE_LIGHT), num(_num) {}
		const int num;
	};

	SHADER_CLASS_BEGIN(LmrShader)
		SHADER_UNIFORM_INT(usetex)
		SHADER_UNIFORM_INT(useglow)
		SHADER_UNIFORM_SAMPLER(tex)
		SHADER_UNIFORM_SAMPLER(texGlow)
	SHADER_CLASS_END()

	void UseProgram(LmrShader *shader, bool Textured = false, bool Glowmap = false);

	void CompleteSurface();
	void PushOp(Op *op);

	static void StaticInit(Graphics::Renderer *renderer);

	static BufferObjectPool<sizeof(Vertex)> s_staticBufferPool;
	static int s_numTrisRendered;

	static bool s_initialized;
	static ScopedPtr<LmrShader> s_sunlightShader[4];
	static ScopedPtr<LmrShader> s_pointlightShader[4];

	Graphics::Renderer *m_renderer;

	ScopedPtr<Graphics::StaticMesh> m_mesh;

	ScopedPtr<Graphics::Surface> m_curSurface;

	int m_curMaterialIdx;

	/* this crap is only used at build time... could move this elsewhere */
	Uint16 curTriFlag;
	std::string *curTexture;
	std::string *curGlowmap;
	matrix4x4f curTexMatrix;

	std::vector<Uint16> m_triflags;
	std::vector<Op*> m_ops;
	std::vector<ShipThruster::Thruster> m_thrusters;
	LmrModel *m_model;
	int m_boIndexBase;
	BufferObject<sizeof(Vertex)> *m_bo;
	bool m_isStatic;
	bool m_putGeomInsideout;
};

}

#endif
