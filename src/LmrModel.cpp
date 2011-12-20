#include "libs.h"
#include <map>
#include "FontManager.h"
#include "VectorFont.h"
#include "LmrModel.h"
#include "collider/collider.h"
#include "perlin.h"
#include "render/Render.h"
#include "BufferObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EquipType.h"
#include "ShipType.h"
#include "TextureCache.h"
#include <set>
#include <algorithm>

/*
 * Interface: LMR
 *
 * Script interface to the model system.
 *
 * This documentation is incomplete!
 */

struct RenderState {
	/* For the root model this will be identity matrix.
	 * For sub-models called with call_model() then this will be the
	 * transform from sub-model coords to root-model coords.
	 * It is needed by the RenderThruster stuff so we know the centre of
	 * the root model and orientation when rendering thrusters on
	 * sub-models */
	matrix4x4f subTransform;
	// combination of model scale, call_model scale, and all parent scalings
	float combinedScale;
};
struct LmrUnknownMaterial {};

namespace ShipThruster {
	struct Thruster
	{
		Thruster() : pos(0.0), dir(0.0), power(0) {}	// zero this shit to stop denormal-copying on resize
		// cannot be used as an angular thruster
		bool linear_only;
		vector3f pos;
		vector3f dir;
		float power;
	};

	
	static vector3f ResolveHermiteSpline (const vector3f &p0, const vector3f &p1, const vector3f &n0, const vector3f &n1, float t)
	{
		float t2 = t*t, t3 = t*t*t;
		vector3f tv1, tv2, tv;
		tv1 = p0 * (2*t3-3*t2+1);
		tv = n0 * (t3-2*t2+t);
		tv1 = tv+tv1;
		tv2 = p1 * (-2*t3+3*t2);
		tv = n1 *  (t3-t2);
		tv2 = tv + tv2;
		return tv1 + tv2;
	}

	static const int pNumIndex[3] =
		{ (2*4+1*8)*3, (2*8+5*16)*3, (2*16+13*32)*3 };

	static vector3f pTVertex4pt[2*4+2];
	static vector3f pTVertex8pt[6*8+2];
	static vector3f pTVertex16pt[14*16+2];

	static Uint16 pTIndex4pt[(2*4+1*8)*3];
	static Uint16 pTIndex8pt[(2*8+5*16)*3];
	static Uint16 pTIndex16pt[(2*16+13*32)*3];

	static vector3f *ppTVertex[3] =
		{ pTVertex4pt, pTVertex8pt, pTVertex16pt };

	static Uint16 *ppTIndex[3] =
		{ pTIndex4pt, pTIndex8pt, pTIndex16pt };


	static void GenerateThrusters ()
	{
		vector3f pos0 = vector3f(0.0f, 0.0f, 0.0f);
		vector3f pos1 = vector3f(0.0f, 0.0f, 1.0f);
		vector3f tan0 = vector3f(0.0f, 1.0f, 0.2f);
		vector3f tan1 = vector3f(0.0f, -0.2f, 1.0f);

		int j, n;
		for (j=0, n=4; j<3; j++, n<<=1)
		{
			vector3f *pCur = ppTVertex[j];
			float t, incstep = 1.0f / (n-1);
			int i; for (i=0, t=incstep; i<n-2; i++, t+=incstep)
			{
				vector3f *pos = pCur;
				*pos = ResolveHermiteSpline (pos0, pos1, tan0, tan1, t);
				pCur++;

				float angstep = 2.0f * 3.141592f / n, ang = angstep;
				for (int k=1; k<n; k++, ang+=angstep, pCur++)
				{
					pCur->x = sin(ang) * pos->y;
					pCur->y = cos(ang) * pos->y;
					pCur->z = pos->z;
				}
			}
			*pCur = pos0; pCur++;
			*pCur = pos1; pCur++;

			int ni=0, k;
			Uint16 *pIndex = ppTIndex[j];

			for (k=0; k<n; k++) {
				int k1 = k+1==n ? 0 : k+1;
				pIndex[ni++] = (n-2)*n; pIndex[ni++] = k; pIndex[ni++] = k1;
				pIndex[ni++] = (n-2)*n+1; pIndex[ni++] = k1+(n-3)*n; pIndex[ni++] = k+(n-3)*n;
			}
			for (i=0; i<n-3; i++)
			{
				for (k=0; k<n; k++) {
					int k1 = k+1==n ? 0 : k+1;
					pIndex[ni++] = k+i*n; pIndex[ni++] = k+i*n+n; pIndex[ni++] = k1+i*n;
					pIndex[ni++] = k1+i*n; pIndex[ni++] = k+i*n+n;	pIndex[ni++] = k1+i*n+n;
				}
			}
		}
	}

	static const float s_black[4] = { 0, 0, 0, 0 };
	static const float s_alpha[4] = { 0, 0, 0, 0.6 };
	static bool inittted = false;

	static void RenderThruster(const RenderState *rstate, const LmrObjParams *params, Thruster *pThruster)
	{
		if (!inittted) GenerateThrusters();
		inittted = true;
		const float scale = 1.0;
		// to find v(0,0,0) position of root model (when putting thrusters on sub-models)
		vector3f compos = vector3f(rstate->subTransform[12], rstate->subTransform[13], rstate->subTransform[14]);
		matrix4x4f invSubModelMat = matrix4x4f::MakeRotMatrix(
					vector3f(rstate->subTransform[0], rstate->subTransform[1], rstate->subTransform[2]),
					vector3f(rstate->subTransform[4], rstate->subTransform[5], rstate->subTransform[6]),
					vector3f(rstate->subTransform[8], rstate->subTransform[9], rstate->subTransform[10]));

		vector3f start, end, dir = pThruster->dir;
		start = pThruster->pos * scale;
		float power = -dir.Dot(invSubModelMat * vector3f(params->linthrust));

		if (!pThruster->linear_only) {
			vector3f angdir, cpos;
			const vector3f at = invSubModelMat * vector3f(params->angthrust);
			cpos = compos + start;
			angdir = cpos.Cross(dir);
			float xp = angdir.x * at.x;
			float yp = angdir.y * at.y;
			float zp = angdir.z * at.z;
			if (xp+yp+zp > 0) {
				if (xp > yp && xp > zp && fabs(at.x) > power) power = fabs(at.x);
				else if (yp > xp && yp > zp && fabs(at.y) > power) power = fabs(at.y);
				else if (zp > xp && zp > yp && fabs(at.z) > power) power = fabs(at.z);
			}
		}

		if (power <= 0.001f) return;
		power *= scale;
		float width = sqrt(power)*pThruster->power*0.6f;
		float len = power*pThruster->power;
		end = dir * len;
		end += start;

		vector3f v1, v2, pos;
		matrix4x4f m2;
		matrix4x4f m = matrix4x4f::Identity();
		v1.x = dir.y; v1.y = dir.z; v1.z = dir.x;
		v2 = v1.Cross(dir).Normalized();
		v1 = v2.Cross(dir);
		m[0] = v1.x; m[4] = v2.x; m[8] = dir.x;
		m[1] = v1.y; m[5] = v2.y; m[9] = dir.y;
		m[2] = v1.z; m[6] = v2.z; m[10] = dir.z;
		m2 = /*objorient **/ m;

		pos = /*objorient **/ start;

		m2[12] = pos.x;
		m2[13] = pos.y;
		m2[14] = pos.z;
		
		glPushMatrix ();
		glMultMatrixf (&m2[0]);

		glScalef (width*0.5f, width*0.5f, len*0.666f);
		
		if (Render::IsHDREnabled())
			glColor4f(0.0f, 40.0f, 100.0f, 0.9f);
		else
			glColor4f(0.0f, 0.4f, 1.0f, 0.9f);

		glVertexPointer (3, GL_FLOAT, sizeof(vector3f), pTVertex8pt);
		glDrawElements (GL_TRIANGLES, pNumIndex[1], GL_UNSIGNED_SHORT, pTIndex8pt);

		glScalef (2.0f, 2.0f, 1.5f);

		if (Render::IsHDREnabled())
			glColor4f(100.0f, 100.0f, 100.0f, 0.9f);
		else
			glColor4f(0.4f, 0.0f, 1.0f, 0.9f);

		glVertexPointer (3, GL_FLOAT, sizeof(vector3f), pTVertex8pt);
		glDrawElements (GL_TRIANGLES, pNumIndex[1], GL_UNSIGNED_SHORT, pTIndex8pt);

		glPopMatrix ();
	}

	struct CameraDistance {
		Thruster *thruster;
		float dist;
	};
	struct CameraDistanceCompare : public std::binary_function<CameraDistance, CameraDistance, bool> {
		bool operator()(CameraDistance a, CameraDistance b)
		{
			return a.dist > b.dist;
		}
	};
}

class LmrGeomBuffer;

SHADER_CLASS_BEGIN(LmrShader)
	SHADER_UNIFORM_INT(usetex)
	SHADER_UNIFORM_INT(useglow)
	SHADER_UNIFORM_SAMPLER(tex)
	SHADER_UNIFORM_SAMPLER(texGlow)
SHADER_CLASS_END()

static LmrShader *s_sunlightShader[4];
static LmrShader *s_pointlightShader[4];
static float s_scrWidth = 800.0f;
static bool s_buildDynamic;
static FontManager s_fontManager;
static VectorFont *s_font;
static float NEWMODEL_ZBIAS = 0.0002f;
static LmrGeomBuffer *s_curBuf;
static const LmrObjParams *s_curParams;
static std::map<std::string, LmrModel*> s_models;
static lua_State *sLua;
static int s_numTrisRendered;
static std::string s_cacheDir;
static bool s_recompileAllModels = true;
static TextureCache *s_textureCache;

struct Vertex {
	Vertex() : v(0.0), n(0.0), tex_u(0.0), tex_v(0.0) {}		// zero this shit to stop denormal-copying on resize
	Vertex(const vector3f &v_, const vector3f &n_, const GLfloat tex_u_, const GLfloat tex_v_): v(v_), n(n_), tex_u(tex_u_), tex_v(tex_v_) {}
	vector3f v, n;
	GLfloat tex_u, tex_v;
};

static BufferObjectPool<sizeof(Vertex)> *s_staticBufferPool;

lua_State *LmrGetLuaState() { return sLua; }

void LmrNotifyScreenWidth(float width)
{
	s_scrWidth = width;
}

int LmrModelGetStatsTris() { return s_numTrisRendered; }
void LmrModelClearStatsTris() { s_numTrisRendered = 0; }
	
void UseProgram(LmrShader *shader, bool Textured = false, bool Glowmap = false) {
	if (Render::AreShadersEnabled()) {
		Render::State::UseProgram(shader);
		if (Textured) shader->set_tex(0);
		shader->set_usetex(Textured ? 1 : 0);
		if (Glowmap) shader->set_texGlow(1);
		shader->set_useglow(Glowmap ? 1 : 0);
	}
}

#define BUFFER_OFFSET(i) (reinterpret_cast<const GLvoid *>(i))

static void _fwrite_string(const std::string &str, FILE *f)
{
	int len = str.size()+1;
	fwrite(&len, sizeof(len), 1, f);
	fwrite(str.c_str(), sizeof(char), len, f);
}

static std::string _fread_string(FILE *f)
{
	int len = 0;
	fread_or_die(&len, sizeof(len), 1, f);
	char *buf = new char[len];
	fread_or_die(buf, sizeof(char), len, f);
	std::string str = std::string(buf);
	delete[] buf;
	return str;
}
	
class LmrGeomBuffer {
public:
	LmrGeomBuffer(LmrModel *model, bool isStatic) {
		curOp.type = OP_NONE;
		curTriFlag = 0;
		curTexture = 0;
		curGlowmap = 0;
		curTexMatrix = matrix4x4f::Identity();
		m_model = model;
		m_isStatic = isStatic;
		m_bo = 0;
		m_putGeomInsideout = false;
	}
	int GetIndicesPos() const {
		return m_indices.size();
	}
	int GetVerticesPos() const {
		return m_vertices.size();
	}
	void SetGeomFlag(Uint16 flag) {
		curTriFlag = flag;
	}
	Uint16 GetGeomFlag() const {
		return curTriFlag;
	}
	void PreBuild() {
		FreeGeometry();
		curTriFlag = 0;
	}
	void PostBuild() {
		PushCurOp();
		//printf("%d vertices, %d indices, %s\n", m_vertices.size(), m_indices.size(), m_isStatic ? "static" : "dynamic");
		if (m_isStatic && m_indices.size()) {
			s_staticBufferPool->AddGeometry(m_vertices.size(), &m_vertices[0], m_indices.size(), &m_indices[0],
					&m_boIndexBase, &m_bo);
		}
		curOp.type = OP_NONE;
	}
	void FreeGeometry() {
		m_vertices.clear();
		m_indices.clear();
		m_triflags.clear();
		m_ops.clear();
		m_thrusters.clear();
		m_putGeomInsideout = false;
	}

	void Render(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
		int activeLights = 0;
		s_numTrisRendered += m_indices.size()/3;
		
		LmrShader *curShader = s_sunlightShader[Render::State::GetNumLights()-1];

		BindBuffers();

		glDepthRange(0.0, 1.0);

		const unsigned int opEndIdx = m_ops.size();
		for (unsigned int i=0; i<opEndIdx; i++) {
			const Op &op = m_ops[i];
			switch (op.type) {
			case OP_DRAW_ELEMENTS:
				if (op.elems.texture != 0 ) {
					UseProgram(curShader, true, op.elems.glowmap != 0);
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
					op.elems.texture->Bind();
					if (op.elems.glowmap != 0) {
						glActiveTexture(GL_TEXTURE1);
						op.elems.glowmap->Bind();
					}
				} else {
					UseProgram(curShader, false);
				}
				if (m_isStatic) {
					// from static VBO
					glDrawElements(GL_TRIANGLES, 
							op.elems.count, GL_UNSIGNED_SHORT,
							BUFFER_OFFSET((op.elems.start+m_boIndexBase)*sizeof(Uint16)));
					//glDrawRangeElements(GL_TRIANGLES, m_boIndexBase + op.elems.elemMin,
					//		m_boIndexBase + op.elems.elemMax, op.elems.count, GL_UNSIGNED_SHORT,
					//		BUFFER_OFFSET((op.elems.start+m_boIndexBase)*sizeof(Uint16)));
				//	glDrawRangeElements(GL_TRIANGLES, op.elems.elemMin, op.elems.elemMax, 
				//		op.elems.count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(op.elems.start*sizeof(Uint16)));
				} else {
					// otherwise regular index vertex array
					glDrawElements(GL_TRIANGLES, op.elems.count, GL_UNSIGNED_SHORT, &m_indices[op.elems.start]);
				}
				if ( op.elems.texture != 0 ) {
					glActiveTexture(GL_TEXTURE0);
					glDisable(GL_TEXTURE_2D);
				}
				break;
			case OP_DRAW_BILLBOARDS:
				// XXX not using vbo yet
				Render::UnbindAllBuffers();
				op.billboards.texture->Bind();
				Render::PutPointSprites(op.billboards.count, &m_vertices[op.billboards.start].v, op.billboards.size,
						op.billboards.col, sizeof(Vertex));
				BindBuffers();
				break;
			case OP_SET_MATERIAL:
				{
					const LmrMaterial &m = m_model->m_materials[op.col.material_idx];
					glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, m.diffuse);
					glMaterialfv (GL_FRONT, GL_SPECULAR, m.specular);
					glMaterialfv (GL_FRONT, GL_EMISSION, m.emissive);
					glMaterialf (GL_FRONT, GL_SHININESS, m.shininess);
					if (m.diffuse[3] >= 1.0) {
						glDisable(GL_BLEND);
					} else {
						glEnable(GL_BLEND);
					}
				}
				break;
			case OP_ZBIAS:
				if (float_is_zero_general(op.zbias.amount)) {
					glDepthRange(0.0, 1.0);
				} else {
				//	vector3f tv = cameraPos - vector3f(op.zbias.pos);
				//	if (vector3f::Dot(tv, vector3f(op.zbias.norm)) < 0.0f) {
						glDepthRange(0.0, 1.0 - op.zbias.amount*NEWMODEL_ZBIAS);
				//	} else {
				//		glDepthRange(0.0, 1.0);
				//	}
				}
				break;
			case OP_CALL_MODEL:
				{
				// XXX materials fucked up after this
				const matrix4x4f trans = matrix4x4f(op.callmodel.transform);
				vector3f cam_pos = trans.InverseOf() * cameraPos;
				RenderState rstate2;
				rstate2.subTransform = rstate->subTransform * trans;
				rstate2.combinedScale = rstate->combinedScale * op.callmodel.scale * op.callmodel.model->m_scale;
				op.callmodel.model->Render(&rstate2, cam_pos, trans, params);
				// XXX re-binding buffer may not be necessary
				BindBuffers();
				}
				break;
			case OP_LIGHTING_TYPE:
				if (op.lighting_type.local) {
					glDisable(GL_LIGHT0);
					glDisable(GL_LIGHT1);
					glDisable(GL_LIGHT2);
					glDisable(GL_LIGHT3);
					float zilch[4] = { 0.0f,0.0f,0.0f,0.0f };
					for (int j=4; j<8; j++) {
						// so why are these set each
						// time? because the shader
						// path does not know if
						// lightsources are active and
						// uses them all (4-8)
						glLightfv(GL_LIGHT0+j, GL_DIFFUSE, zilch);
						glLightfv(GL_LIGHT0+j, GL_SPECULAR, zilch);
					}
					activeLights = 0;
				} else {
					int numLights = Render::State::GetNumLights();
					for (int j=0; j<numLights; j++) glEnable(GL_LIGHT0 + j);
					for (int j=4; j<8; j++) glDisable(GL_LIGHT0 + j);
					curShader = s_sunlightShader[Render::State::GetNumLights()-1];
				}
				break;
			case OP_USE_LIGHT:
				{
					if (m_model->m_lights.size() <= unsigned(op.light.num)) {
						m_model->m_lights.resize(op.light.num+1);
					}
					LmrLight &l = m_model->m_lights[op.light.num];
					glEnable(GL_LIGHT0 + 4 + activeLights);
					glLightf(GL_LIGHT0 + 4 + activeLights, GL_QUADRATIC_ATTENUATION, l.quadraticAttenuation);
					glLightfv(GL_LIGHT0 + 4 + activeLights, GL_POSITION, l.position);
					glLightfv(GL_LIGHT0 + 4 + activeLights, GL_DIFFUSE, l.color);
					glLightfv(GL_LIGHT0 + 4 + activeLights, GL_SPECULAR, l.color);
					curShader = s_pointlightShader[activeLights++];
					if (activeLights > 4) {
						Error("Too many active lights in model '%s' (maximum 4)", m_model->GetName());
					}
				}
				break;
			case OP_NONE:
				break;
			}
		}
		
		glDisableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_NORMAL_ARRAY);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);

		Render::UnbindAllBuffers();

		if (m_thrusters.size()) {
			glDisable(GL_LIGHTING);
			Render::State::UseProgram(Render::simpleShader);
			RenderThrusters(rstate, cameraPos, params);
		}
	}

	void RenderThrusters(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
		// depth sort thrusters so alpha doesn't look fucked up!!!
		ShipThruster::CameraDistance *dists = static_cast<ShipThruster::CameraDistance*>(alloca (m_thrusters.size() * sizeof(ShipThruster::CameraDistance)));

		for (unsigned int i=0; i<m_thrusters.size(); i++) {
			dists[i].thruster = &m_thrusters[i];
			dists[i].dist = (m_thrusters[i].pos - cameraPos).Length();
		}
		sort(dists, dists + m_thrusters.size(), ShipThruster::CameraDistanceCompare());

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
		glEnableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_NORMAL_ARRAY);
		glDisableClientState (GL_TEXTURE_COORD_ARRAY);
		glEnable (GL_BLEND);
		for (unsigned int i=0; i<m_thrusters.size(); i++) {
			ShipThruster::RenderThruster (rstate, params, dists[i].thruster);
		}
		glDisable (GL_BLEND);
		glDisableClientState (GL_VERTEX_ARRAY);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	}
	void PushThruster(const vector3f &pos, const vector3f &dir, const float power, bool linear_only) {
		unsigned int i = m_thrusters.size();
		m_thrusters.resize(i+1);
		m_thrusters[i].pos = pos;
		m_thrusters[i].dir = dir;
		m_thrusters[i].power = power;
		m_thrusters[i].linear_only = linear_only;
	}
	int PushVertex(const vector3f &pos, const vector3f &normal) {
		vector3f tex = curTexMatrix * pos;
		return PushVertex(pos, normal, tex.x, tex.y);
	}
	void SetVertex(int idx, const vector3f &pos, const vector3f &normal) {
		vector3f tex = curTexMatrix * pos;
		SetVertex(idx, pos, normal, tex.x, tex.y);
	}
	int PushVertex(const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v) {
		if (m_putGeomInsideout) {
			m_vertices.push_back(Vertex(pos, -normal, tex_u, tex_v));
		} else {
			m_vertices.push_back(Vertex(pos, normal, tex_u, tex_v));
		}
		return m_vertices.size() - 1;
	}
	void SetVertex(int idx, const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v) {
		if (m_putGeomInsideout) {
			m_vertices[idx] = Vertex(pos, -normal, tex_u, tex_v);
		} else {
			m_vertices[idx] = Vertex(pos, normal, tex_u, tex_v);
		}
	}
	void SetTexture(const char *tex) {
		if (tex) {
			curTexture = s_textureCache->GetModelTexture(tex);
		} else {
			curTexture = 0;
			curGlowmap = 0; //won't have these without textures
		}
	}
	void SetGlowMap(const char *tex) {
		if (tex) {
			curGlowmap = s_textureCache->GetModelTexture(tex);
		} else {
			curGlowmap = 0;
		}
	}
	void SetTexMatrix(const matrix4x4f &texMatrix) { curTexMatrix = texMatrix; } 
	void PushTri(int i1, int i2, int i3) {
		OpDrawElements(3);
		if (m_putGeomInsideout) {
			PushIdx(i1);
			PushIdx(i3);
			PushIdx(i2);
		} else {
			PushIdx(i1);
			PushIdx(i2);
			PushIdx(i3);
		}
		m_triflags.push_back(curTriFlag);
	}
	void SetInsideOut(bool a) {
		m_putGeomInsideout = a;
	}
	
	void PushZBias(float amount, const vector3f &pos, const vector3f &norm) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_ZBIAS;
		curOp.zbias.amount = amount;
		memcpy(curOp.zbias.pos, &pos.x, 3*sizeof(float));
		memcpy(curOp.zbias.norm, &norm.x, 3*sizeof(float));
	}

	void PushSetLocalLighting(bool enable) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_LIGHTING_TYPE;
		curOp.lighting_type.local = enable;
	}

	void SetLight(int num, float quadratic_attenuation, const vector3f &pos, const vector3f &col) {
		if (m_model->m_lights.size() <= unsigned(num)) {
			m_model->m_lights.resize(num+1);
		}
		LmrLight &l = m_model->m_lights[num];
		memcpy(l.position, &pos, sizeof(vector3f));
		memcpy(l.color, &col, sizeof(vector3f));
		l.position[3] = l.color[3] = 1.0;
		l.quadraticAttenuation = quadratic_attenuation;
	}

	void PushUseLight(int num) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_USE_LIGHT;
		curOp.light.num = num;
	}

	void PushCallModel(LmrModel *m, const matrix4x4f &transform, float scale) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_CALL_MODEL;
		memcpy(curOp.callmodel.transform, &transform[0], 16*sizeof(float));
		curOp.callmodel.model = m;
		curOp.callmodel.scale = scale;
	}

	void PushInvisibleTri(int i1, int i2, int i3) {
		if (curOp.type != OP_NONE) m_ops.push_back(curOp);
		curOp.type = OP_NONE;
		PushIdx(i1);
		PushIdx(i2);
		PushIdx(i3);
		m_triflags.push_back(curTriFlag);
	}
	
	void PushBillboards(const char *texname, const float size, const vector3f &color, const int numPoints, const vector3f *points)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), PIONEER_DATA_DIR"/textures/%s", texname);

		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_DRAW_BILLBOARDS;
		curOp.billboards.start = m_vertices.size();
		curOp.billboards.count = numPoints;
		curOp.billboards.texture = s_textureCache->GetModelTexture(buf, true);
		curOp.billboards.size = size;
		curOp.billboards.col[0] = color.x;
		curOp.billboards.col[1] = color.y;
		curOp.billboards.col[2] = color.z;
		curOp.billboards.col[3] = 1.0f;

		for (int i=0; i<numPoints; i++)
			PushVertex(points[i], vector3f());
	}

	void SetMaterial(const char *mat_name, const float mat[11]) {
		std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
		if (i != m_model->m_materialLookup.end()) {
			LmrMaterial &m = m_model->m_materials[(*i).second];
			m.diffuse[0] = mat[0];
			m.diffuse[1] = mat[1];
			m.diffuse[2] = mat[2];
			m.diffuse[3] = mat[3];
			m.specular[0] = mat[4];
			m.specular[1] = mat[5];
			m.specular[2] = mat[6];
			m.specular[3] = 1.0f;
			m.shininess = Clamp(mat[7], 1.0f, 100.0f);
			m.emissive[0] = mat[8];
			m.emissive[1] = mat[9];
			m.emissive[2] = mat[10];
			m.emissive[3] = 1.0f;
		} else {
			luaL_error(sLua, "Unknown material name '%s'.", mat_name);
			exit(0);
		}
	}

	void PushUseMaterial(const char *mat_name) {
		std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
		if (i != m_model->m_materialLookup.end()) {
			if (curOp.type) m_ops.push_back(curOp);
			curOp.type = OP_SET_MATERIAL;
			curOp.col.material_idx = (*i).second;
		} else {
			throw LmrUnknownMaterial();
		}
	}

	/* return start vertex index */
	int AllocVertices(int num) {
		int start = m_vertices.size();
		m_vertices.resize(start + num);
		return start;
	}

	const vector3f &GetVertex(int num) const {
		return m_vertices[num].v;
	}

	void GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform, const LmrObjParams *params) {
		const int vtxBase = c->nv;
		const int idxBase = c->ni;
		const int flagBase = c->nf;
		c->nv += m_vertices.size();
		c->ni += m_indices.size();
		c->nf += m_indices.size()/3;
		assert(m_triflags.size() == m_indices.size()/3);
		c->m_numTris += m_triflags.size();

		if (m_vertices.size()) {
			c->pVertex = static_cast<float*>(realloc(c->pVertex, 3*sizeof(float)*c->nv));
		
			for (unsigned int i=0; i<m_vertices.size(); i++) {
				const vector3f v = transform * m_vertices[i].v;
				c->pVertex[3*vtxBase + 3*i] = v.x;
				c->pVertex[3*vtxBase + 3*i+1] = v.y;
				c->pVertex[3*vtxBase + 3*i+2] = v.z;
				c->m_aabb.Update(vector3d(v));
			}
		}
		if (m_indices.size()) {
			c->pIndex = static_cast<int*>(realloc(c->pIndex, sizeof(int)*c->ni));
			c->pFlag = static_cast<unsigned int*>(realloc(c->pFlag, sizeof(unsigned int)*c->nf));
			for (unsigned int i=0; i<m_indices.size(); i++) {
				c->pIndex[idxBase + i] = vtxBase + m_indices[i];
			}
			for (unsigned int i=0; i<m_triflags.size(); i++) {
				c->pFlag[flagBase + i] = m_triflags[i];
			}
		}
		
		// go through Ops to see if we call other models
		const unsigned int opEndIdx = m_ops.size();
		for (unsigned int i=0; i<opEndIdx; i++) {
			const Op &op = m_ops[i];
			if (op.type == OP_CALL_MODEL) {
				matrix4x4f _trans = transform * matrix4x4f(op.callmodel.transform);
				op.callmodel.model->GetCollMeshGeometry(c, _trans, params);
			}
		}
	}

private:
	void BindBuffers() {
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_NORMAL_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);

		if (m_isStatic) {
			if (m_bo) m_bo->BindBuffersForDraw();
		} else {
			Render::UnbindAllBuffers();
			if (m_vertices.size()) {
				glNormalPointer(GL_FLOAT, sizeof(Vertex), &m_vertices[0].n);
				glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_vertices[0].v);
				glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m_vertices[0].tex_u);
			}
		}
	}

	void OpDrawElements(int numIndices) {
		if ((curOp.type != OP_DRAW_ELEMENTS) || (curOp.elems.texture != curTexture)) {
			if (curOp.type) m_ops.push_back(curOp);
			curOp.type = OP_DRAW_ELEMENTS;
			curOp.elems.start = m_indices.size();
			curOp.elems.count = 0;
			curOp.elems.elemMin = 1<<30;
			curOp.elems.elemMax = 0;
			curOp.elems.texture = curTexture;
			curOp.elems.glowmap = curGlowmap;
		}
		curOp.elems.count += numIndices;
	}
	void PushCurOp() {
		m_ops.push_back(curOp);
	}
	void PushIdx(Uint16 v) {
		curOp.elems.elemMin = std::min<int>(v, curOp.elems.elemMin);
		curOp.elems.elemMax = std::max<int>(v, curOp.elems.elemMax);
		m_indices.push_back(v);
	}

	enum OpType { OP_NONE, OP_DRAW_ELEMENTS, OP_DRAW_BILLBOARDS, OP_SET_MATERIAL, OP_ZBIAS,
			OP_CALL_MODEL, OP_LIGHTING_TYPE, OP_USE_LIGHT };

	struct Op {
		enum OpType type;
		union {
			struct { ModelTexture *texture; ModelTexture *glowmap; int start, count, elemMin, elemMax; } elems;
			struct { int material_idx; } col;
			struct { float amount; float pos[3]; float norm[3]; } zbias;
			struct { LmrModel *model; float transform[16]; float scale; } callmodel;
			struct { ModelTexture *texture; int start, count; float size; float col[4]; } billboards;
			struct { bool local; } lighting_type;
			struct { int num; float quadratic_attenuation; float pos[4], col[4]; } light;
		};
	};
	/* this crap is only used at build time... could move this elsewhere */
	Op curOp;
	Uint16 curTriFlag;
	ModelTexture *curTexture;
	ModelTexture *curGlowmap;
	matrix4x4f curTexMatrix;
	// 
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

public:
	void SaveToCache(FILE *f) {
		int numVertices = m_vertices.size();
		int numIndices = m_indices.size();
		int numTriflags = m_triflags.size();
		int numThrusters = m_thrusters.size();
		int numOps = m_ops.size();
		fwrite(&numVertices, sizeof(numVertices), 1, f);
		fwrite(&numIndices, sizeof(numIndices), 1, f);
		fwrite(&numTriflags, sizeof(numTriflags), 1, f);
		fwrite(&numThrusters, sizeof(numThrusters), 1, f);
		fwrite(&numOps, sizeof(numOps), 1, f);
		if (numVertices) fwrite(&m_vertices[0], sizeof(Vertex), numVertices, f);
		if (numIndices) fwrite(&m_indices[0], sizeof(Uint16), numIndices, f);
		if (numTriflags) fwrite(&m_triflags[0], sizeof(Uint16), numTriflags, f);
		if (numThrusters) fwrite(&m_thrusters[0], sizeof(ShipThruster::Thruster), numThrusters, f);
		if (numOps) {
			for (int i=0; i<numOps; i++) {
				fwrite(&m_ops[i], sizeof(Op), 1, f);
				if (m_ops[i].type == OP_CALL_MODEL) {
					_fwrite_string(m_ops[i].callmodel.model->GetName(), f);
				}
				else if ((m_ops[i].type == OP_DRAW_ELEMENTS) && (m_ops[i].elems.texture)) {
					_fwrite_string(m_ops[i].elems.texture->GetFilename(), f);
					if (m_ops[i].elems.glowmap)
						_fwrite_string(m_ops[i].elems.glowmap->GetFilename(), f);
				}
				else if ((m_ops[i].type == OP_DRAW_BILLBOARDS) && (m_ops[i].billboards.texture)) {
					_fwrite_string(m_ops[i].billboards.texture->GetFilename(), f);
				}
			}
		}
	}
	void LoadFromCache(FILE *f) {
		int numVertices, numIndices, numTriflags, numThrusters, numOps;
		fread_or_die(&numVertices, sizeof(numVertices), 1, f);
		fread_or_die(&numIndices, sizeof(numIndices), 1, f);
		fread_or_die(&numTriflags, sizeof(numTriflags), 1, f);
		fread_or_die(&numThrusters, sizeof(numThrusters), 1, f);
		fread_or_die(&numOps, sizeof(numOps), 1, f);
		assert(numVertices <= 65536);
		assert(numIndices < 1000000);
		assert(numTriflags < 1000000);
		assert(numThrusters < 1000);
		assert(numOps < 1000);
		if (numVertices) {
			m_vertices.resize(numVertices);
			fread_or_die(&m_vertices[0], sizeof(Vertex), numVertices, f);
		}
		if (numIndices) {
			m_indices.resize(numIndices);
			fread_or_die(&m_indices[0], sizeof(Uint16), numIndices, f);
		}
		if (numTriflags) {
			m_triflags.resize(numTriflags);
			fread_or_die(&m_triflags[0], sizeof(Uint16), numTriflags, f);
		}
		if (numThrusters) {
			m_thrusters.resize(numThrusters);
			fread_or_die(&m_thrusters[0], sizeof(ShipThruster::Thruster), numThrusters, f);
		}
		m_ops.resize(numOps);
		for (int i=0; i<numOps; i++) {
			fread_or_die(&m_ops[i], sizeof(Op), 1, f);
			if (m_ops[i].type == OP_CALL_MODEL) {
				m_ops[i].callmodel.model = s_models[_fread_string(f)];
			}
			else if ((m_ops[i].type == OP_DRAW_ELEMENTS) && (m_ops[i].elems.texture)) {
				m_ops[i].elems.texture = s_textureCache->GetModelTexture(_fread_string(f));
				if (m_ops[i].elems.glowmap)
					m_ops[i].elems.glowmap = s_textureCache->GetModelTexture(_fread_string(f));
			}
			else if ((m_ops[i].type == OP_DRAW_BILLBOARDS) && (m_ops[i].billboards.texture)) {
				m_ops[i].billboards.texture = s_textureCache->GetModelTexture(_fread_string(f));
			}
		}
	}
};

LmrModel::LmrModel(const char *model_name)
{
	m_name = model_name;
	m_drawClipRadius = 1.0f;
	m_scale = 1.0f;

	{
	LUA_DEBUG_START(sLua);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", model_name);
	lua_getglobal(sLua, buf);
	if (lua_istable(sLua, -1)) {
		m_numLods = 0;

		lua_getfield(sLua, -1, "bounding_radius");
		if (lua_isnumber(sLua, -1)) m_drawClipRadius = luaL_checknumber(sLua, -1);
		else luaL_error(sLua, "model %s_info missing bounding_radius=", model_name);
		lua_pop(sLua, 1);

		lua_getfield(sLua, -1, "lod_pixels");
		if (lua_istable(sLua, -1)) {
			for(int i=1;; i++) {
				lua_pushinteger(sLua, i);
				lua_gettable(sLua, -2);
				bool is_num = lua_isnumber(sLua, -1) != 0;
				if (is_num) {
					m_lodPixelSize[i-1] = luaL_checknumber(sLua, -1);
					m_numLods++;
				}
				lua_pop(sLua, 1);
				if (!is_num) break;
				if (i > LMR_MAX_LOD) {
					luaL_error(sLua, "Too many LODs (maximum %d)", LMR_MAX_LOD);
				}
			}
		} else {
			m_numLods = 1;
			m_lodPixelSize[0] = 0;
		}
		lua_pop(sLua, 1);

		lua_getfield(sLua, -1, "materials");
		if (lua_istable(sLua, -1)) {
			for(int i=1;; i++) {
				lua_pushinteger(sLua, i);
				lua_gettable(sLua, -2);
				bool is_string = lua_isstring(sLua, -1) != 0;
				if (is_string) {
					const char *mat_name = luaL_checkstring(sLua, -1);
					m_materialLookup[mat_name] = m_materials.size();
					m_materials.push_back(LmrMaterial());
				}
				lua_pop(sLua, 1);
				if (!is_string) break;
			}
		}
		lua_pop(sLua, 1);

		lua_getfield(sLua, -1, "scale");
		if (lua_isnumber(sLua, -1)) {
			m_scale = lua_tonumber(sLua, -1);
		}
		lua_pop(sLua, 1);

		/* pop model_info table */
		lua_pop(sLua, 1);
	} else {
		luaL_error(sLua, "Could not find function %s_info()", model_name);
	}
	
	snprintf(buf, sizeof(buf), "%s_dynamic", model_name);
	lua_getglobal(sLua, buf);
	m_hasDynamicFunc = lua_isfunction(sLua, -1);
	lua_pop(sLua, 1);

	LUA_DEBUG_END(sLua, 0);
	}

	for (int i=0; i<m_numLods; i++) {
		m_staticGeometry[i] = new LmrGeomBuffer(this, true);
		m_dynamicGeometry[i] = new LmrGeomBuffer(this, false);
	}

	const std::string cache_file = join_path(s_cacheDir.c_str(), model_name, 0) + ".bin";

	if (!s_recompileAllModels) {
		// load cached model
		FILE *f = fopen(cache_file.c_str(), "rb");
		if (!f) goto rebuild_model;

		for (int i=0; i<m_numLods; i++) {
			m_staticGeometry[i]->PreBuild();
			m_staticGeometry[i]->LoadFromCache(f);
			m_staticGeometry[i]->PostBuild();
		}
		int numMaterials;
		fread_or_die(&numMaterials, sizeof(numMaterials), 1, f);
		if (size_t(numMaterials) != m_materials.size()) {
			fclose(f);
			goto rebuild_model;
		}
		if (numMaterials) fread_or_die(&m_materials[0], sizeof(LmrMaterial), numMaterials, f);

		int numLights;
		fread_or_die(&numLights, sizeof(numLights), 1, f);
		if (size_t(numLights) != m_lights.size()) {
			fclose(f);
			goto rebuild_model;
		}
		if (numLights) fread_or_die(&m_lights[0], sizeof(LmrLight), numLights, f);

		fclose(f);
	} else {
rebuild_model:
		// run static build for each LOD level
		FILE *f = fopen(cache_file.c_str(), "wb");
		
		for (int i=0; i<m_numLods; i++) {
			LUA_DEBUG_START(sLua);
			m_staticGeometry[i]->PreBuild();
			s_curBuf = m_staticGeometry[i];
			lua_pushcfunction(sLua, pi_lua_panic);
			// call model static building function
			lua_getfield(sLua, LUA_GLOBALSINDEX, (m_name+"_static").c_str());
			// lod as first argument
			lua_pushnumber(sLua, i+1);
			lua_pcall(sLua, 1, 0, -3);
			lua_pop(sLua, 1);  // remove panic func
			s_curBuf = 0;
			m_staticGeometry[i]->PostBuild();
			m_staticGeometry[i]->SaveToCache(f);
			LUA_DEBUG_END(sLua, 0);
		}
		
		const int numMaterials = m_materials.size();
		fwrite(&numMaterials, sizeof(numMaterials), 1, f);
		if (numMaterials) fwrite(&m_materials[0], sizeof(LmrMaterial), numMaterials, f);
		const int numLights = m_lights.size();
		fwrite(&numLights, sizeof(numLights), 1, f);
		if (numLights) fwrite(&m_lights[0], sizeof(LmrLight), numLights, f);
		
		fclose(f);
	}
}

LmrModel::~LmrModel()
{
	for (int i=0; i<m_numLods; i++) {
		delete m_staticGeometry[i];
		delete m_dynamicGeometry[i];
	}
}

//static std::map<std::string, LmrModel*> s_models;
void LmrGetModelsWithTag(const char *tag, std::vector<LmrModel*> &outModels)
{
	for (std::map<std::string, LmrModel*>::iterator i = s_models.begin();
			i != s_models.end(); ++i) {
		if (i->second->HasTag(tag))
			outModels.push_back(i->second);
	}
}

float LmrModel::GetFloatAttribute(const char *attr_name) const
{
	LUA_DEBUG_START(sLua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(sLua, buf);
	lua_getfield(sLua, -1, attr_name);
	float result = luaL_checknumber(sLua, -1);
	lua_pop(sLua, 2);
	LUA_DEBUG_END(sLua, 0);
	return result;
}

int LmrModel::GetIntAttribute(const char *attr_name) const
{
	LUA_DEBUG_START(sLua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(sLua, buf);
	lua_getfield(sLua, -1, attr_name);
	int result = luaL_checkinteger(sLua, -1);
	lua_pop(sLua, 2);
	LUA_DEBUG_END(sLua, 0);
	return result;
}

bool LmrModel::GetBoolAttribute(const char *attr_name) const
{
	char buf[256];
	LUA_DEBUG_START(sLua);
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(sLua, buf);
	lua_getfield(sLua, -1, attr_name);
	bool result;
	if (lua_isnil(sLua, -1)) {
		result = false;
	} else {
		result = lua_toboolean(sLua, -1) != 0;
	}	
	lua_pop(sLua, 2);
	LUA_DEBUG_END(sLua, 0);
	return result;
}

void LmrModel::PushAttributeToLuaStack(const char *attr_name) const
{
	LUA_DEBUG_START(sLua);
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());
	lua_getglobal(sLua, buf);
	lua_getfield(sLua, -1, attr_name);
	lua_remove(sLua, -2);
	LUA_DEBUG_END(sLua, 1);
}

bool LmrModel::HasTag(const char *tag) const
{
	bool has_tag = false;

	LUA_DEBUG_START(sLua);

	char buf[256];
	snprintf(buf, sizeof(buf), "%s_info", m_name.c_str());

	lua_getglobal(sLua, buf);
	lua_getfield(sLua, -1, "tags");
	if (lua_istable(sLua, -1)) {
		for(int j=1;; j++) {
			lua_pushinteger(sLua, j);
			lua_gettable(sLua, -2);
			if (lua_isstring(sLua, -1)) {
				const char *s = luaL_checkstring(sLua, -1);
				if (0 == strcmp(tag, s)) {
					has_tag = true;
					lua_pop(sLua, 1);
					break;
				}
			} else if (lua_isnil(sLua, -1)) {
				lua_pop(sLua, 1);
				break;
			}
			lua_pop(sLua, 1);
		}
	}
	lua_pop(sLua, 2);

	LUA_DEBUG_END(sLua, 0);

	return has_tag;
}

void LmrModel::Render(const matrix4x4f &trans, const LmrObjParams *params)
{
	RenderState rstate;
	rstate.subTransform = matrix4x4f::Identity();
	rstate.combinedScale = m_scale;
	Render(&rstate, vector3f(-trans[12], -trans[13], -trans[14]), trans, params);
}

void LmrModel::Render(const RenderState *rstate, const vector3f &cameraPos, const matrix4x4f &trans, const LmrObjParams *params)
{
	glPushMatrix();
	glMultMatrixf(&trans[0]);
	glScalef(m_scale, m_scale, m_scale);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	float pixrad = 0.5f * s_scrWidth * rstate->combinedScale * m_drawClipRadius / cameraPos.Length();
	//printf("%s: %fpx\n", m_name.c_str(), pixrad);

	int lod = m_numLods-1;
	for (int i=lod-1; i>=0; i--) {
		if (pixrad < m_lodPixelSize[i]) lod = i;
	}
	//printf("%s: lod %d\n", m_name.c_str(), lod);

	Build(lod, params);

	const vector3f modelRelativeCamPos = trans.InverseOf() * cameraPos;

	m_staticGeometry[lod]->Render(rstate, modelRelativeCamPos, params);
	if (m_hasDynamicFunc) {
		m_dynamicGeometry[lod]->Render(rstate, modelRelativeCamPos, params);
	}
	s_curBuf = 0;

	glDisable(GL_NORMALIZE);
	glDisable(GL_BLEND);
	glPopMatrix();
}

void LmrModel::Build(int lod, const LmrObjParams *params)
{
	if (m_hasDynamicFunc) {
		LUA_DEBUG_START(sLua);
		m_dynamicGeometry[lod]->PreBuild();
		s_curBuf = m_dynamicGeometry[lod];
		s_curParams = params;
		lua_pushcfunction(sLua, pi_lua_panic);
		// call model dynamic bits
		lua_getfield(sLua, LUA_GLOBALSINDEX, (m_name+"_dynamic").c_str());
		// lod as first argument
		lua_pushnumber(sLua, lod+1);
		lua_pcall(sLua, 1, 0, -3);
		lua_pop(sLua, 1);  // remove panic func
		s_curBuf = 0;
		s_curParams = 0;
		m_dynamicGeometry[lod]->PostBuild();
		LUA_DEBUG_END(sLua, 0);
	}
}

void LmrModel::GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform, const LmrObjParams *params)
{
	// use lowest LOD
	Build(0, params);
	matrix4x4f m = transform * matrix4x4f::ScaleMatrix(m_scale);
	m_staticGeometry[0]->GetCollMeshGeometry(mesh, m, params);
	if (m_hasDynamicFunc) m_dynamicGeometry[0]->GetCollMeshGeometry(mesh, m, params);
}

LmrCollMesh::LmrCollMesh(LmrModel *m, const LmrObjParams *params)
{
	memset(this, 0, sizeof(LmrCollMesh));
	m_aabb.min = vector3d(DBL_MAX, DBL_MAX, DBL_MAX);
	m_aabb.max = vector3d(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	m->GetCollMeshGeometry(this, matrix4x4f::Identity(), params);
	m_radius = m_aabb.GetBoundingRadius();
	geomTree = new GeomTree(nv, m_numTris, pVertex, pIndex, pFlag);
}

/** returns number of tris found (up to 'num') */
int LmrCollMesh::GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const
{
	int found = 0;
	for (int i=0; (i<m_numTris) && (found<num); i++) {
		if (pFlag[i] == flags) {
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i]]);
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i+1]]);
			*(outVtx++) = vector3d(&pVertex[3*pIndex[3*i+2]]);
			found++;
		}
	}
	return found;
}

LmrCollMesh::~LmrCollMesh()
{
	// nice. mixed allocation. for the love of realloc...
	delete geomTree;
	free(pVertex);
	free(pIndex);
	free(pFlag);
}

LmrModel *LmrLookupModelByName(const char *name)
{
	std::map<std::string, LmrModel*>::iterator i = s_models.find(name);

	if (i == s_models.end()) {
		throw LmrModelNotFoundException();
	}
	return (*i).second;
}	

namespace ModelFuncs {
	/*
	 * Function: call_model
	 *
	 * Use another model as a submodel.
	 *
	 * > call_model(modelname, pos, xaxis, yaxis, scale)
	 *
	 * Parameters:
	 *
	 *   modelname - submodel to call, must be already loaded
	 *   pos - position to load the submodel at
	 *   xaxis - submodel orientation along x axis
	 *   yaxis - submodel orientation along y axis
	 *   scale - submodel scale
	 *
	 * Example:
	 *
	 * > call_model('front_wheel',v(0,0,-50),v(0,0,1),v(1,0,0),1.0)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int call_model(lua_State *L)
	{
		const char *obj_name = luaL_checkstring(L, 1);
//	subobject(object_name, vector pos, vector xaxis, vector yaxis [, scale=float, onflag=])
		if (!obj_name) return 0;
		if (!obj_name[0]) return 0;
		LmrModel *m = s_models[obj_name];
		if (!m) {
			luaL_error(L, "call_model() to undefined model '%s'. Referenced model must be registered before calling model", obj_name);
		} else {
			const vector3f *pos = MyLuaVec::checkVec(L, 2);
			const vector3f *_xaxis = MyLuaVec::checkVec(L, 3);
			const vector3f *_yaxis = MyLuaVec::checkVec(L, 4);
			float scale = luaL_checknumber(L, 5);

			vector3f zaxis = _xaxis->Cross(*_yaxis).Normalized();
			vector3f xaxis = _yaxis->Cross(zaxis).Normalized();
			vector3f yaxis = zaxis.Cross(xaxis);

			matrix4x4f trans = matrix4x4f::MakeInvRotMatrix(scale*xaxis, scale*yaxis, scale*zaxis);
			trans[12] = pos->x;
			trans[13] = pos->y;
			trans[14] = pos->z;

			s_curBuf->PushCallModel(m, trans, scale);
		}
		return 0;
	}

	/*
	 * Function: set_light
	 *
	 * Set parameters for a local light. Up to four lights are available.
	 * You can use it by calling use_light after set_local_lighting(true)
	 * has been called.
	 *
	 * > set_light(number, attenuation, position, color)
	 *
	 * Parameters:
	 *
	 *   number - number of the light to modify, 1 to 4
	 *   attenuation - quadratic attenuation
	 *   position - xyz position
	 *   color - rgb
	 *
	 * Example:
	 *
	 * > set_light(1, 0.00005, v(0,0,0), v(1,0.2,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int set_light(lua_State *L)
	{
		int num = luaL_checkinteger(L, 1)-1;
		if ((num < 0) || (num > 3)) {
			luaL_error(L, "set_light should have light number from 1 to 4.");
		}
		const float quadratic_attenuation = luaL_checknumber(L, 2);
		const vector3f *pos = MyLuaVec::checkVec(L, 3);
		const vector3f *col = MyLuaVec::checkVec(L, 4);
		s_curBuf->SetLight(num, quadratic_attenuation, *pos, *col);
		return 0;
	}

	/*
	 * Function: use_light
	 *
	 * Use one of the local lights.
	 *
	 * > use_light(number)
	 *
	 * Parameters:
	 *
	 *   number - local light number, 1 to 4
	 *
	 * Example:
	 *
	 * > use_light(1)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int use_light(lua_State *L)
	{
		int num = luaL_checkinteger(L, 1)-1;
		s_curBuf->PushUseLight(num);
		return 0;
	}

	/*
	 * Function: set_local_lighting
	 *
	 * Enable use of lights local to the model. They do not affect
	 * the surroundings, and are meant for lighting structure interiors.
	 *
	 * > set_local_lighting(state)
	 *
	 * Parameters:
	 *
	 *   state - true or false
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int set_local_lighting(lua_State *L)
	{
		const bool doIt = lua_toboolean(L, 1) != 0;
		s_curBuf->PushSetLocalLighting(doIt);
		return 0;
	}

	/*
	 * Function: set_insideout
	 *
	 * Flip faces. When enabled, subsequent drawing will be inside-out (reversed triangle
	 * winding and normals)
	 *
	 * >  set_insideout(state)
	 *
	 * Parameters:
	 *
	 *   state - true or false
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int insideout(lua_State *L)
	{
		const bool doIt = lua_toboolean(L, 1) != 0;
		s_curBuf->SetInsideOut(doIt);
		return 0;
	}

	/*
	 * Function: lathe
	 *
	 * Cylindrical shape that can be tapered at different lengths
	 *
	 * >  lathe(sides, start, end, up, steps)
	 *
	 * Parameters:
	 *
	 *   sides - number of sides, at least 3
	 *   start - position vector to start at
	 *   end - position vector to finish at
	 *   up - up direction vector, can be used to rotate shape
	 *   steps - table of position, radius pairs. Positions are from 0.0
	 *           (start of the cylinder) to 1.0 (end). If you want a closed
	 *           cylinder have a zero-radius positions at the start and the end.
	 *
	 * Example:
	 *
	 * > lathe(8, v(0,0,0), v(0,10,0), v(1,0,0), {0.0,0.0, 0.0,1.0, 0.4,1.2, 0.6,1.2, 1.0,1.0, 1.0,0.0})
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int lathe(lua_State *L)
	{
		const int steps = luaL_checkinteger(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);

		if (!lua_istable(L, 5)) {
			luaL_error(L, "lathe() takes a table of distance, radius numbers");
		}

		int num = lua_objlen(L, 5);
		if (num % 2) luaL_error(L, "lathe() passed list with unpaired distance, radius element");
		if (num < 4) luaL_error(L, "lathe() passed list with insufficient distance, radius pairs");

		// oh tom you fox
		float *jizz = static_cast<float*>(alloca(num*2*sizeof(float)));

		for (int i=1; i<=num; i++) {
			lua_pushinteger(L, i);
			lua_gettable(L, 5);
			jizz[i-1] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}

		const int vtxStart = s_curBuf->AllocVertices(steps*(num-2));

		const vector3f dir = (*end-*start).Normalized();
		const vector3f axis1 = updir->Normalized();
		const vector3f axis2 = updir->Cross(dir).Normalized();
		const float inc = 2.0f*M_PI / float(steps);

		for (int i=0; i<num-3; i+=2) {
			const float rad1 = jizz[i+1];
			const float rad2 = jizz[i+3];
			const vector3f _start = *start + (*end-*start)*jizz[i];
			const vector3f _end = *start + (*end-*start)*jizz[i+2];
			bool shitty_normal = float_equal_absolute(jizz[i], jizz[i+2], 1e-4f);

			const int basevtx = vtxStart + steps*i;
			float ang = 0;
			for (int j=0; j<steps; j++, ang += inc) {
				const vector3f p1 = rad1 * (sin(ang)*axis1 + cos(ang)*axis2);
				const vector3f p2 = rad2 * (sin(ang)*axis1 + cos(ang)*axis2);
				vector3f n;
				if (shitty_normal) {
					if (rad1 > rad2) n = dir;
					else n = -dir;
				} else {
					vector3f tmp = (_end+p2)-(_start+p1);
					n = tmp.Cross(p1).Cross(tmp).Normalized();
				}
				s_curBuf->SetVertex(basevtx + j, _start+p1, n);
				s_curBuf->SetVertex(basevtx + steps + j, _end+p2, n);
			}
			for (int j=0; j<steps-1; j++) {
				s_curBuf->PushTri(basevtx+j, basevtx+j+1, basevtx+j+steps);
				s_curBuf->PushTri(basevtx+j+1, basevtx+j+steps+1, basevtx+j+steps);
			}
			s_curBuf->PushTri(basevtx+steps-1, basevtx, basevtx+2*steps-1);
			s_curBuf->PushTri(basevtx, basevtx+steps, basevtx+2*steps-1);
		}
		return 0;
	}

	/*
	 * Function: extrusion
	 *
	 * Extrude an outline/cross-section. Ends will be closed.
	 *
	 * >  extrusion(start, end, up, radius, shape)
	 *
	 * Parameters:
	 *
	 *   start - position vector to start at
	 *   end - position vector to end at
	 *   up - up vector, can be used to rotate shape
	 *   radius - scale of the extrusion
	 *   shape - table of position vectors to define the outline, maximum 32
	 *
	 * Example:
	 *
	 * > extrusion(v(0,0,20), v(0,0,-20), v(0,1,0), 1.0, v(-20,0,0), v(20,0,0), v(20,200,0), v(-20,200,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int extrusion(lua_State *L)
	{
		const vector3f *start = MyLuaVec::checkVec(L, 1);
		const vector3f *end = MyLuaVec::checkVec(L, 2);
		const vector3f *updir = MyLuaVec::checkVec(L, 3);
		const float radius = luaL_checknumber(L, 4);

#define EXTRUSION_MAX_VTX 32
		int steps = lua_gettop(L)-4;
		if (steps > EXTRUSION_MAX_VTX) {
			luaL_error(L, "extrusion() takes at most %d points", EXTRUSION_MAX_VTX);
		}
		vector3f evtx[EXTRUSION_MAX_VTX];

		for (int i=0; i<steps; i++) {
			evtx[i] = *MyLuaVec::checkVec(L, i+5);
		}

		const int vtxStart = s_curBuf->AllocVertices(6*steps);

		vector3f yax = *updir;
		vector3f xax, zax;
		zax = ((*end) - (*start)).Normalized();
		xax = yax.Cross(zax);

		for (int i=0; i<steps; i++) {
			vector3f tv, norm;
			tv = xax * evtx[i].x;
			norm = yax * evtx[i].y;
			norm = norm + tv;

			vector3f p1 = norm * radius;
			s_curBuf->SetVertex(vtxStart + i, (*start) + p1, -zax);
			s_curBuf->SetVertex(vtxStart + i + steps, (*end) + p1, zax);
		}

		for (int i=0; i<steps-1; i++) {
			// top cap
			s_curBuf->PushTri(vtxStart, vtxStart+i+1, vtxStart+i);
			// bottom cap
			s_curBuf->PushTri(vtxStart+steps, vtxStart+steps+i, vtxStart+steps+i+1);
		}

		// sides
		for (int i=0; i<steps; i++) {
			const vector3f &v1 = s_curBuf->GetVertex(vtxStart + i);
			const vector3f &v2 = s_curBuf->GetVertex(vtxStart + (i + 1)%steps);
			const vector3f &v3 = s_curBuf->GetVertex(vtxStart + i + steps);
			const vector3f &v4 = s_curBuf->GetVertex(vtxStart + (i + 1)%steps + steps);
			const vector3f norm = (v2-v1).Cross(v3-v1).Normalized();

			const int idx = vtxStart + 2*steps + i*4;
			s_curBuf->SetVertex(idx, v1, norm);
			s_curBuf->SetVertex(idx+1, v2, norm);
			s_curBuf->SetVertex(idx+2, v3, norm);
			s_curBuf->SetVertex(idx+3, v4, norm);

			s_curBuf->PushTri(idx, idx+1, idx+3);
			s_curBuf->PushTri(idx, idx+3, idx+2);
		}

		return 0;
	}
	
	static vector3f eval_cubic_bezier_u(const vector3f p[4], float u)
	{
		vector3f out(0.0f);
		float Bu[4] = { (1.0f-u)*(1.0f-u)*(1.0f-u),
			3.0f*(1.0f-u)*(1.0f-u)*u,
			3.0f*(1.0f-u)*u*u,
			u*u*u };
		for (int i=0; i<4; i++) {
			out += p[i] * Bu[i];
		}
		return out;
	}

	static vector3f eval_quadric_bezier_u(const vector3f p[3], float u)
	{
		vector3f out(0.0f);
		float Bu[3] = { (1.0f-u)*(1.0f-u), 2.0f*u*(1.0f-u), u*u };
		for (int i=0; i<3; i++) {
			out += p[i] * Bu[i];
		}
		return out;
	}

	static int _flat(lua_State *L, bool xref)
	{
		const int divs = luaL_checkinteger(L, 1);
		const vector3f *normal = MyLuaVec::checkVec(L, 2);
		vector3f xrefnorm(0.0f);
		if (xref) xrefnorm = vector3f(-normal->x, normal->y, normal->z);
#define FLAT_MAX_SEG 32
		struct {
			const vector3f *v[3];
			int nv;
		} segvtx[FLAT_MAX_SEG];

		if (!lua_istable(L, 3)) {
			luaL_error(L, "argment 3 to flat() must be a table of line segments");
			return 0;
		}

		int argmax = lua_gettop(L);
		int seg = 0;
		int numPoints = 0;
		// iterate through table of line segments
		for (int n=3; n<=argmax; n++, seg++) {
			if (lua_istable(L, n)) {
				// this table is a line segment itself
				// 1 vtx = straight line
				// 2     = quadric bezier
				// 3     = cubic bezier
				int nv = 0;
				for (int i=1; i<4; i++) {
					lua_pushinteger(L, i);
					lua_gettable(L, n);
					if (lua_isnil(L, -1)) {
						lua_pop(L, 1);
						break;
					} else {
						segvtx[seg].v[nv++] = MyLuaVec::checkVec(L, -1);
						lua_pop(L, 1);
					}
				}
				segvtx[seg].nv = nv;

				if (!nv) {
					luaL_error(L, "number of points in a line segment must be 1-3 (straight, quadric, cubic)");
					return 0;
				} else if (nv == 1) {
					numPoints++;
				} else if (nv > 1) {
					numPoints += divs;
				}
			} else {
				luaL_error(L, "invalid crap in line segment list");
				return 0;
			}
		}

		const int vtxStart = s_curBuf->AllocVertices(xref ? 2*numPoints : numPoints);
		int vtxPos = vtxStart;

		const vector3f *prevSegEnd = segvtx[seg-1].v[ segvtx[seg-1].nv-1 ];
		// evaluate segments
		int maxSeg = seg;
		for (seg=0; seg<maxSeg; seg++) {
			if (segvtx[seg].nv == 1) {
				if (xref) {
					vector3f p = *segvtx[seg].v[0]; p.x = -p.x;
					s_curBuf->SetVertex(vtxPos + numPoints, p, xrefnorm);
				}
				s_curBuf->SetVertex(vtxPos++, *segvtx[seg].v[0], *normal);
				prevSegEnd = segvtx[seg].v[0];
			} else if (segvtx[seg].nv == 2) {
				vector3f _p[3];
				_p[0] = *prevSegEnd;
				_p[1] = *segvtx[seg].v[0];
				_p[2] = *segvtx[seg].v[1];
				float inc = 1.0f / float(divs);
				float u = inc;
				for (int i=1; i<=divs; i++, u+=inc) {
					vector3f p = eval_quadric_bezier_u(_p, u);
					s_curBuf->SetVertex(vtxPos, p, *normal);
					if (xref) {
						p.x = -p.x;
						s_curBuf->SetVertex(vtxPos+numPoints, p, xrefnorm);
					}
					vtxPos++;
				}
				prevSegEnd = segvtx[seg].v[1];
			} else if (segvtx[seg].nv == 3) {
				vector3f _p[4];
				_p[0] = *prevSegEnd;
				_p[1] = *segvtx[seg].v[0];
				_p[2] = *segvtx[seg].v[1];
				_p[3] = *segvtx[seg].v[2];
				float inc = 1.0f / float(divs);
				float u = inc;
				for (int i=1; i<=divs; i++, u+=inc) {
					vector3f p = eval_cubic_bezier_u(_p, u);
					s_curBuf->SetVertex(vtxPos, p, *normal);
					if (xref) {
						p.x = -p.x;
						s_curBuf->SetVertex(vtxPos+numPoints, p, xrefnorm);
					}
					vtxPos++;
				}
				prevSegEnd = segvtx[seg].v[2];
			}
		}

		for (int i=1; i<numPoints-1; i++) {
			s_curBuf->PushTri(vtxStart, vtxStart+i, vtxStart+i+1);
			if (xref) {
				s_curBuf->PushTri(vtxStart+numPoints, vtxStart+numPoints+1+i, vtxStart+numPoints+i);
			}
		}
		return 0;
	}
	
	/*
	 * Function: flat
	 *
	 * Multi-point patch shape.
	 *
	 * > flat(divs, normal, points)
	 *
	 * Parameters:
	 *
	 *   divs - number of subdivisions
	 *   normal - face direction vector
	 *   points - outline path segments as separate vector tables. Number of table elements
	 *            determines the segment type (linear, quadratic, cubic). Points can be mixed.
	 *            32 point maximum.
	 *
	 * Example:
	 *
	 * > --rectangle of four linear points
	 * > flat(6, v(0,1,0), {v(-2,0,0)}, {v(2,0,0)}, {v(2,2,0)}, {v(-2,2,0)})
	 * > --smoother, and top replaced with a curve
	 * > flat(16, v(0,1,0),{v(-2,0,0)}, {v(2,0,0)}, {v(2,2,0)}, {v(2,2,0), v(0,4,0), v(-2,2,0)})
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int flat(lua_State *L) { return _flat(L, false); }

	/*
	 * Function: xref_flat
	 *
	 * Symmetry version of <flat>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_flat(lua_State *L) { return _flat(L, true); }

	static vector3f eval_quadric_bezier_triangle(const vector3f p[6], float s, float t, float u)
	{
		vector3f out(0.0f);
		const float coef[6] = { s*s, 2.0f*s*t, t*t, 2.0f*s*u, 2.0f*t*u, u*u };
		for (int i=0; i<6; i++) {
			out += p[i] * coef[i];
		}
		return out;
	}

	static vector3f eval_cubic_bezier_triangle(const vector3f p[10], float s, float t, float u)
	{
		vector3f out(0.0f);
		const float coef[10] = { s*s*s, 3.0f*s*s*t, 3.0f*s*t*t, t*t*t, 3.0f*s*s*u, 6.0f*s*t*u, 3.0f*t*t*u, 3.0f*s*u*u, 3.0f*t*u*u, u*u*u };
		for (int i=0; i<10; i++) {
			out += p[i] * coef[i];
		}
		return out;
	}
	template <int BEZIER_ORDER>
	static void _bezier_triangle(lua_State *L, bool xref)
	{
		vector3f pts[10];
		const int divs = luaL_checkinteger(L, 1) + 1;
		assert(divs > 0);
		if (BEZIER_ORDER == 2) {
			for (int i=0; i<6; i++) {
				pts[i] = *MyLuaVec::checkVec(L, i+2);
			}
		} else if (BEZIER_ORDER == 3) {
			for (int i=0; i<10; i++) {
				pts[i] = *MyLuaVec::checkVec(L, i+2);
			}
		}

		const int numVertsInPatch = divs*(1+divs)/2;
		const int vtxStart = s_curBuf->AllocVertices(numVertsInPatch * (xref ? 2 : 1));
		int vtxPos = vtxStart;

		float inc = 1.0f / float(divs-1);
		float s,t,u;
		s = t = u = 0;
		for (int i=0; i<divs; i++, u += inc) {
			float pos = 0;
			float inc2 = 1.0f / float(divs-1-i);
			for (int j=i; j<divs; j++, pos += inc2) {
				s = (1.0f-u)*(1.0f-pos);
				t = (1.0f-u)*pos;
				vector3f p, pu, pv;
				if (BEZIER_ORDER == 2) {
					p = eval_quadric_bezier_triangle(pts, s, t, u);
					pu = eval_quadric_bezier_triangle(pts, s+0.1f*inc, t-0.1f*inc, u);
					pv = eval_quadric_bezier_triangle(pts, s-0.05f*inc, t-0.05f*inc, u+0.1f*inc);
				} else if (BEZIER_ORDER == 3) {
					p = eval_cubic_bezier_triangle(pts, s, t, u);
					pu = eval_cubic_bezier_triangle(pts, s+0.1f*inc, t-0.1f*inc, u);
					pv = eval_cubic_bezier_triangle(pts, s-0.05f*inc, t-0.05f*inc, u+0.1f*inc);
				}
				vector3f norm = (pu-p).Cross(pv-p).Normalized();
				s_curBuf->SetVertex(vtxPos, p, norm);

				if (xref) {
					norm.x = -norm.x;
					p.x = -p.x;
					s_curBuf->SetVertex(vtxPos + numVertsInPatch, p, norm);
				}
				vtxPos++;
			}
		}
		//assert((vtxPos - vtxStart) == numVertsInPatch);

		vtxPos = vtxStart;
		for (int y=0; y<divs-1; y++) {
			const int adv = divs-y;
			s_curBuf->PushTri(vtxPos, vtxPos+adv, vtxPos+1);
			for (int x=1; x<adv-1; x++) {
				s_curBuf->PushTri(vtxPos+x, vtxPos+x+adv-1, vtxPos+x+adv);
				s_curBuf->PushTri(vtxPos+x, vtxPos+x+adv, vtxPos+x+1);
			}
			if (xref) {
				const int refVtxPos = vtxPos + numVertsInPatch;
				s_curBuf->PushTri(refVtxPos, refVtxPos+1, refVtxPos+adv);
				for (int x=1; x<adv-1; x++) {
					s_curBuf->PushTri(refVtxPos+x, refVtxPos+x+adv, refVtxPos+x+adv-1);
					s_curBuf->PushTri(refVtxPos+x, refVtxPos+x+1, refVtxPos+x+adv);
				}
			}
			vtxPos += adv;
		}
	}

	/*
	 * Function: cubic_bezier_tri
	 *
	 * Bezier triangle, cubic interpolation
	 *
	 * > cubic_bezier_tri(divs, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)
	 *
	 * Parameters:
	 *
	 *   divs - number of subdivisions
	 *   v1-v10 - ten control points. v1, v4 and v10 are the triangle corners. v6 is the triangle center.
	 *
	 * Example:
	 *
	 * > --triangle with curved sides and depressed center
	 * > cubic_bezier_tri(16, v(-4,0,0), v(-1,0,0), v(1,0,0), v(4,0,0),
	 * >    v(-2,1,0), v(0,1,10), v(2,1,0),
	 * >    v(-1,2,0), v(1,2,0),
	 * >    v(0,5,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int cubic_bezier_triangle(lua_State *L) { _bezier_triangle<3>(L, false); return 0; }

	/*
	 * Function: xref_cubic_bezier_tri
	 *
	 * Symmetry version of <cubic_bezier_tri>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_cubic_bezier_triangle(lua_State *L) { _bezier_triangle<3>(L, true); return 0; }

	/*
	 * Function: quadric_bezier_tri
	 *
	 * Bezier triangle, quadratic interpolation
	 *
	 * > quadric_bezier_tri(divs, v1, v2, v3, v4, v5, v6)
	 *
	 * Parameters:
	 *
	 *   divs - number of subdivisions
	 *   v1-v6 - six control points, v1, v3 and v6 form the corners
	 *
	 * Example:
	 *
	 * > --triangle with concave sides
	 * > quadric_bezier_tri(16, v(-4,0,0), v(0,1,0), v(4,0,0), v(-1,2,0), v(1,2,0), v(0,4,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int quadric_bezier_triangle(lua_State *L) { _bezier_triangle<2>(L, false); return 0; }

	/*
	 * Function: xref_quadric_bezier_tri
	 *
	 * Symmetry version of <quadric_bezier_tri>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_quadric_bezier_triangle(lua_State *L) { _bezier_triangle<2>(L, true); return 0; }


	static vector3f eval_quadric_bezier_u_v(const vector3f p[9], float u, float v)
	{
		vector3f out(0.0f);
		float Bu[3] = { (1.0f-u)*(1.0f-u), 2.0f*u*(1.0f-u), u*u };
		float Bv[3] = { (1.0f-v)*(1.0f-v), 2.0f*v*(1.0f-v), v*v };
		for (int i=0; i<3; i++) {
			for (int j=0; j<3; j++) {
				out += p[i+3*j] * Bu[i] * Bv[j];
			}
		}
		return out;
	}

	static void _quadric_bezier_quad(lua_State *L, bool xref)
	{
		vector3f pts[9];
		const int divs_u = luaL_checkinteger(L, 1);
		const int divs_v = luaL_checkinteger(L, 2);
		for (int i=0; i<9; i++) {
			pts[i] = *MyLuaVec::checkVec(L, i+3);
		}

		const int numVertsInPatch = (divs_u+1)*(divs_v+1);
		const int vtxStart = s_curBuf->AllocVertices(numVertsInPatch * (xref ? 2 : 1));

		float inc_u = 1.0f / float(divs_u);
		float inc_v = 1.0f / float(divs_v);
		float u,v;
		u = v = 0;
		for (int i=0; i<=divs_u; i++, u += inc_u) {
			v = 0;
			for (int j=0; j<=divs_v; j++, v += inc_v) {
				vector3f p = eval_quadric_bezier_u_v(pts, u, v);
				// this is a very inefficient way of
				// calculating normals...
				vector3f pu = eval_quadric_bezier_u_v(pts, u+0.01f*inc_u, v);
				vector3f pv = eval_quadric_bezier_u_v(pts, u, v+0.01f*inc_v);
				vector3f norm = (pu-p).Cross(pv-p).Normalized();

				s_curBuf->SetVertex(vtxStart + i*(divs_v+1) + j, p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					s_curBuf->SetVertex(vtxStart + numVertsInPatch + i*(divs_v+1) + j, p, norm);
				}
			}
		}

		for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+1);
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+(divs_v+1), baseVtx+j+1+(divs_v+1));
			}
		}
		if (xref) for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + numVertsInPatch + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs_v+1));
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+(divs_v+1));
			}
		}
	}
	

	/*
	 * Function: quadric_bezier_quad
	 *
	 * Smoothly interpolated patch shape (quadratic interpolation)
	 *
	 * > quadric_bezier_quad(u, v, v1, v2, v3, v4, v5, v6, v7, v8, v9)
	 *
	 * Parameters:
	 *
	 *   u - 'horizontal' subdivisions
	 *   v - 'vertical' subdivisions
	 *   v1-v9 - nine control points. v1, v3, v7 and v9 form the corners.
	 *
	 * Example:
	 *
	 * > --patch with a sunken center
	 * > quadric_bezier_quad(8, 8,
	 *      v(0,0,0), v(1,0,0), v(2,0,0),
	 *      v(0,0,1), v(1,-3,1), v(2,0,1),
	 *      v(0,0,2), v(1,0,2), v(2,0,2))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int quadric_bezier_quad(lua_State *L) { _quadric_bezier_quad(L, false); return 0; }

	/*
	 * Function: xref_quadric_bezier_quad
	 *
	 * Symmetry version of <quadric_bezier_quad>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_quadric_bezier_quad(lua_State *L) { _quadric_bezier_quad(L, true); return 0; }

	static vector3f eval_cubic_bezier_u_v(const vector3f p[16], float u, float v)
	{
		vector3f out(0.0f);
		float Bu[4] = { (1.0f-u)*(1.0f-u)*(1.0f-u),
			3.0f*(1.0f-u)*(1.0f-u)*u,
			3.0f*(1.0f-u)*u*u,
			u*u*u };
		float Bv[4] = { (1.0f-v)*(1.0f-v)*(1.0f-v),
			3.0f*(1.0f-v)*(1.0f-v)*v,
			3.0f*(1.0f-v)*v*v,
			v*v*v };
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				out += p[i+4*j] * Bu[i] * Bv[j];
			}
		}
		return out;
	}

	static void _cubic_bezier_quad(lua_State *L, bool xref)
	{
		vector3f pts[16];
		const int divs_v = luaL_checkinteger(L, 1);
		const int divs_u = luaL_checkinteger(L, 2);
		if (lua_istable(L, 3)) {
			for (int i=0; i<16; i++) {
				lua_pushinteger(L, i+1);
				lua_gettable(L, 3);
				pts[i] = *MyLuaVec::checkVec(L, -1);
				lua_pop(L, 1);
			}
		} else {
			for (int i=0; i<16; i++) {
				pts[i] = *MyLuaVec::checkVec(L, i+3);
			}
		}

		const int numVertsInPatch = (divs_v+1)*(divs_u+1);
		const int vtxStart = s_curBuf->AllocVertices(numVertsInPatch * (xref ? 2 : 1));


		float inc_v = 1.0f / float(divs_v);
		float inc_u = 1.0f / float(divs_u);
		float u,v;
		u = v = 0;
		for (int i=0; i<=divs_u; i++, u += inc_u) {
			v = 0;
			for (int j=0; j<=divs_v; j++, v += inc_v) {
				vector3f p = eval_cubic_bezier_u_v(pts, u, v);
				// this is a very inefficient way of
				// calculating normals...
				vector3f pu = eval_cubic_bezier_u_v(pts, u+0.01f*inc_u, v);
				vector3f pv = eval_cubic_bezier_u_v(pts, u, v+0.01f*inc_v);
				vector3f norm = (pu-p).Cross(pv-p).Normalized();

				s_curBuf->SetVertex(vtxStart + i*(divs_v+1) + j, p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					s_curBuf->SetVertex(vtxStart + numVertsInPatch + i*(divs_v+1) + j, p, norm);
				}
			}
		}

		for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+1);
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+(divs_v+1), baseVtx+j+1+(divs_v+1));
			}
		}
		if (xref) for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + numVertsInPatch + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs_v+1));
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+(divs_v+1));
			}
		}
	}

	/*
	 * Function: cubic_bezier_quad
	 *
	 * Smoothly interpolated patch shape (cubic interpolation)
	 *
	 * > cubic_bezier_quad(u, v, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)
	 *
	 * Parameters:
	 *
	 *   u - 'horizontal' subdivisions
	 *   v - 'vertical' subdivisions
	 *   v1-v16 - sixteen control points. v1, v4, 13 and v16 form the corners.
	 *
	 * Example:
	 *
	 * > --patch with a raised center
	 * > cubic_bezier_quad(8, 8,
	 * >   v(0,0,0), v(1,0,0), v(2,0,0), v(3,0,0),
	 * >   v(0,1,0), v(1,1,3), v(2,1,3), v(3,1,0),
	 * >   v(0,2,0), v(1,2,3), v(2,2,3), v(3,2,0),
	 * >   v(0,3,0), v(1,3,0), v(2,3,0), v(3,3,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int cubic_bezier_quad(lua_State *L) { _cubic_bezier_quad(L, false); return 0; }

	/*
	 * Function: xref_cubic_bezier_quad
	 *
	 * Symmetry version of <cubic_bezier_quad>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_cubic_bezier_quad(lua_State *L) { _cubic_bezier_quad(L, true); return 0; }

	/*
	 * Function: set_material
	 *
	 * Set or update material properties. Materials are activated with <use_material>.
	 * Pioneer materials use the phong lighting model.
	 *
	 * >  set_material(name, red, green, blue, alpha, specular_red, specular_green, specular_blue, shininess, emissive_red, emissive_gree, emissive_blue)
	 *
	 * Parameters:
	 *
	 *   name - one of the names defined in model's materials table
	 *   red - diffuse color, red component
	 *   green - diffuse color, green component
	 *   blue - diffuse color, blue component
	 *   alpha - amount of material's translucency
	 *   specular_red - specular highlight color, red component
	 *   specular_green - specular highlight color, green component
	 *   specular_blue - specular highlight color, blue component
	 *   shininess - strength of specular highlights
	 *   emissive_red - self illumination, red component
	 *   emissive_green - self illumination, green component
	 *   emissive_blue - self illumination, blue component
	 *
	 * Example:
	 *
	 * > set_material('wall', 1.0,1.0,1.0,1.0, 0.3,0.3,0.3,5.0, 0.0,0.0,0.0)
	 * > set_material('windows', 0,0,0,1, 1,1,1,50, .5,.5,0)
	 * > set_material('blue', 0.0,0.0,0.8,1.0) --just rgba
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int set_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		float mat[11];
		if (lua_istable(L, 2)) {
			// material as table of 11 values
			for (int i=0; i<11; i++) {
				lua_pushinteger(L, i+1);
				lua_gettable(L, 2);
				mat[i] = luaL_checknumber(L, -1);
				lua_pop(L, 1);
			}
		} else {
			for (int i=0; i<11; i++) {
				mat[i] = lua_tonumber(L, i+2);
			}
		}
		s_curBuf->SetMaterial(mat_name, mat);
		return 0;
	}

	/*
	 * Function: use_material
	 *
	 * Activate a material to be used with subsequent drawing commands
	 *
	 * >  use_material(name)
	 *
	 * Parameters:
	 *
	 *   name - material defined in model's materials table
	 *
	 * Example:
	 *
	 * > use_material('wall')
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int use_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		try {
			s_curBuf->PushUseMaterial(mat_name);
		} catch (LmrUnknownMaterial) {
			printf("Unknown material name '%s'.\n", mat_name);
			exit(0);
		}
		return 0;
	}

	/*
	 * Function: texture
	 *
	 * Apply a texture map to subsequent geometry. Additionally define
	 * texture UV coordinates by projection.
	 *
	 * > texture(name, pos, uaxis, vaxis)
	 *
	 * Parameters:
	 *
	 *   name - texture file name. texture(nil) disables texture.
	 *   pos  - vector position
	 *   uaxis - U vector
	 *   vaxis - V vector
	 *
	 * Example:
	 *
	 * > texture("hull.png")
	 * > texture("wall.png", v(0,0,0), v(1,0,0), v(0,0,1))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int texture(lua_State *L)
	{
		const int nargs = lua_gettop(L);
		if (lua_isnil(L, 1)) {
			s_curBuf->SetTexture(0);
		} else {
			lua_getglobal(L, "CurrentDirectory");
			std::string dir = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			const char *texfile = luaL_checkstring(L, 1);
			std::string t = dir + std::string("/") + texfile;
			if (nargs == 4) {
				// texfile, pos, uaxis, vaxis
				vector3f pos = *MyLuaVec::checkVec(L, 2);
				vector3f uaxis = *MyLuaVec::checkVec(L, 3);
				vector3f vaxis = *MyLuaVec::checkVec(L, 4);
				vector3f waxis = uaxis.Cross(vaxis);

				matrix4x4f trans = matrix4x4f::MakeInvRotMatrix(uaxis, vaxis, waxis);
				trans[12] = -pos.x;
				trans[13] = -pos.y;
				s_curBuf->SetTexMatrix(trans);
			}

			s_curBuf->SetTexture(t.c_str());
		}
		return 0;
	}

/*
 * Function: texture_glow
 *
 * Set a glow map. Meant to be used alongside a texture(). The glow
 * map will override the material's emissive value. The glow texture will
 * be additively blended.
 *
 * > texture_glow('glowmap.png')
 *
 * Parameters:
 *
 *   name - RGB texture file name
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   experimental
 */
	static int texture_glow(lua_State *L)
	{
		if (lua_isnil(L, 1)) {
			s_curBuf->SetGlowMap(0);
		} else {
			lua_getglobal(L, "CurrentDirectory");
			std::string dir = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			const char *texfile = luaL_checkstring(L, 1);
			std::string t = dir + std::string("/") + texfile;
			s_curBuf->SetGlowMap(t.c_str());
		}
		return 0;
	}

		static matrix4x4f _textTrans;
		static vector3f _textNorm;
		static void _text_index_callback(int num, Uint16 *vals) {
			const int base = s_curBuf->GetVerticesPos();
			for (int i=0; i<num; i+=3) {
				s_curBuf->PushTri(vals[i]+base, vals[i+1]+base, vals[i+2]+base);
			}
		}
		static void _text_vertex_callback(int num, float offsetX, float offsetY, float *vals) {
			for (int i=0; i<num*3; i+=3) {
				vector3f p = vector3f(offsetX+vals[i], offsetY+vals[i+1], vals[i+2]);
				p = _textTrans * p;
				s_curBuf->PushVertex(p, _textNorm);
			}
		}

	/*
	 * Function: text
	 *
	 * Draw three-dimensional text. For ship registration ID, landing bay numbers...
	 * 
	 * Long strings can create a large number of triangles so try to be
	 * economical.
	 *
	 * > text(text, pos, normal, textdir, scale, centering)
	 *
	 * Parameters:
	 *
	 *   text - string of text
	 *   pos - vector position of lower left corner (if centering is off)
	 *   normal - face normal
	 *   textdir - text rotation
	 *   scale - text scale
	 *   centering - optional table with a named boolean, {center=true/false}, default off
	 *
	 * Example:
	 *
	 * > text("BLOB", v(0,0,0), v(0,0,1), v(1,0,0), 10.0, { center=true }) --horizontal text
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int text(lua_State *L)
	{
		const char *str = luaL_checkstring(L, 1);
		vector3f pos = *MyLuaVec::checkVec(L, 2);
		vector3f *norm = MyLuaVec::checkVec(L, 3);
		vector3f *textdir = MyLuaVec::checkVec(L, 4);
		float scale = luaL_checknumber(L, 5);
		vector3f yaxis = norm->Cross(*textdir).Normalized();
		vector3f zaxis = textdir->Cross(yaxis).Normalized();
		vector3f xaxis = yaxis.Cross(zaxis);
		_textTrans = matrix4x4f::MakeInvRotMatrix(scale*xaxis, scale*yaxis, scale*zaxis);
		
		bool do_center = false;
		if (lua_istable(L, 6)) {
			lua_pushstring(L, "center");
			lua_gettable(L, 6);
			do_center = lua_toboolean(L, -1) != 0;
			lua_pop(L, 1);

			lua_pushstring(L, "xoffset");
			lua_gettable(L, 6);
			float xoff = lua_tonumber(L, -1);
			lua_pop(L, 1);
			
			lua_pushstring(L, "yoffset");
			lua_gettable(L, 6);
			float yoff = lua_tonumber(L, -1);
			lua_pop(L, 1);
			pos += _textTrans * vector3f(xoff, yoff, 0);
		}
	
		if (do_center) {
			float xoff = 0, yoff = 0;
			s_font->MeasureString(str, xoff, yoff);
			pos -= 0.5f * (_textTrans * vector3f(xoff, yoff, 0));
		}
		_textTrans[12] = pos.x;
		_textTrans[13] = pos.y;
		_textTrans[14] = pos.z;
		_textNorm = *norm;
		s_font->GetStringGeometry(str, &_text_index_callback, &_text_vertex_callback);
//text("some literal string", vector pos, vector norm, vector textdir, [xoff=, yoff=, scale=, onflag=])
		return 0;
	}
	
	/*
	 * Function: geomflag
	 *
	 * Set flags for subsequent geometry. Used for collision detection special
	 * cases, such as space station docking bays.
	 *
	 * Model collision should not be disabled entirely or crashes can happen.
	 *
	 * > geomflag(flag)
	 *
	 * Parameters:
	 *
	 *   flag - 0x0:  remove special flag
	 *          0x10: first docking bay
	 *          0x11: second docking bay
	 *          0x12: third docking bay
	 *          0x14: fourth docking bay
	 *          0x8000:  disable collision detection
	 *
	 * Example:
	 *
	 * > geomflag(0x14)
	 * > extrusion(v(-100,0,0), v(-100,0,100), v(0,1,0), 1.0, v(-50,0,0), v(50,0,0), v(50,10,0), v(-50,10,0))
	 * > geomflag(0)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int geomflag(lua_State *L)
	{
		Uint16 flag = luaL_checkinteger(L, 1);
		s_curBuf->SetGeomFlag(flag);
		return 0;
	}

	/*
	 * Function: zbias
	 *
	 * Fine-tune depth range. Overlapping geometry can be rendered without 
	 * z-fighting using this parameter.
	 *
	 * > zbias(amount, position, normal)
	 *
	 * Parameters:
	 *
	 *   amount - adjustment value, use 0 to restore normal operation
	 *   position - unused
	 *   normal - unused
	 *
	 * Example:
	 *
	 * > quad(v(-1,-0.5,0),v(1,-0.5,0),v(1,0.5,0),v(-1,0.5,0))
	 * > zbias(1.0, v(0,0,0),v(0,0,1))
	 * > text("Some text", v(0,0,0), v(0,0,1), v(1,0,0), .2, {center=true})
   * > zbias(0)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int zbias(lua_State *L)
	{
		int amount = luaL_checkinteger(L, 1);
		if (! amount) {
			s_curBuf->PushZBias(0, vector3f(0.0), vector3f(0.0));
		} else {
			vector3f *pos = MyLuaVec::checkVec(L, 2);
			vector3f *norm = MyLuaVec::checkVec(L, 3);
			s_curBuf->PushZBias(float(amount), *pos, *norm);
		}
		return 0;
	}

	static void _circle(int steps, const vector3f &center, const vector3f &normal, const vector3f &updir, float radius) {
		const int vtxStart = s_curBuf->AllocVertices(steps);

		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(normal).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5f*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = center + radius * (sin(ang)*axis1 + cos(ang)*axis2);
			s_curBuf->SetVertex(vtxStart+i, p, normal);
		}

		for (int i=2; i<steps; i++) {
			// top cap
			s_curBuf->PushTri(vtxStart, vtxStart+i-1, vtxStart+i);
		}
	}

	/*
	 * Function: circle
	 *
	 * Circle (disc)
	 *
	 * > circle(steps, center, normal, up, radius)
	 *
	 * Parameters:
	 *
	 *   steps - number of vertices
	 *   center - vector position of the center
	 *   normal - face normal vector
	 *   up - up direction vector
	 *   radius - circle radius
	 *
	 * Example:
	 *
	 * > circle(8, v(0,0,0), v(0,1,0), v(0,0,1), .3)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int circle(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		const vector3f *center = MyLuaVec::checkVec(L, 2);
		const vector3f *normal = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_circle(steps, *center, *normal, *updir, radius);
		return 0;
	}

	/*
	 * Function: xref_circle
	 *
	 * Symmetry version of <circle>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * > xref_circle(steps, center, normal, up, radius)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_circle(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		vector3f center = *MyLuaVec::checkVec(L, 2);
		vector3f normal = *MyLuaVec::checkVec(L, 3);
		vector3f updir = *MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_circle(steps, center, normal, updir, radius);
		center.x = -center.x;
		normal.x = -normal.x;
		updir.x = -updir.x;
		_circle(steps, center, normal, updir, radius);
		return 0;
	}

	static void _tube(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float inner_radius, float outer_radius) {
		const int vtxStart = s_curBuf->AllocVertices(8*steps);

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		const float radmod = 1.0f/cosf(ang);
		inner_radius *= radmod;
		outer_radius *= radmod;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f p_inner = inner_radius * p;
			vector3f p_outer = outer_radius * p;

			s_curBuf->SetVertex(vtxStart+i, start+p_outer, p);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p_outer, p);
			s_curBuf->SetVertex(vtxStart+i+2*steps, start+p_inner, -p);
			s_curBuf->SetVertex(vtxStart+i+3*steps, end+p_inner, -p);

			s_curBuf->SetVertex(vtxStart+i+4*steps, start+p_outer, -dir);
			s_curBuf->SetVertex(vtxStart+i+5*steps, end+p_outer, dir);
			s_curBuf->SetVertex(vtxStart+i+6*steps, start+p_inner, -dir);
			s_curBuf->SetVertex(vtxStart+i+7*steps, end+p_inner, dir);
		}

		for (int i=0; i<steps-1; i++) {
			s_curBuf->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			s_curBuf->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
			s_curBuf->PushTri(vtxStart+i+2*steps, vtxStart+i+steps+2*steps, vtxStart+i+1+2*steps);
			s_curBuf->PushTri(vtxStart+i+1+2*steps, vtxStart+i+steps+2*steps, vtxStart+i+steps+1+2*steps);
		}
		s_curBuf->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		s_curBuf->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);
		
		s_curBuf->PushTri(vtxStart+3*steps-1, vtxStart+4*steps-1, vtxStart+2*steps);
		s_curBuf->PushTri(vtxStart+2*steps, vtxStart+4*steps-1, vtxStart+3*steps);

		for (int i=0; i<steps-1; i++) {
			// 'start' end
			s_curBuf->PushTri(vtxStart+4*steps+i, vtxStart+6*steps+i, vtxStart+4*steps+i+1);
			
			s_curBuf->PushTri(vtxStart+4*steps+i+1, vtxStart+6*steps+i, vtxStart+6*steps+i+1);
			// 'end' end *cough*
			s_curBuf->PushTri(vtxStart+5*steps+i, vtxStart+5*steps+i+1, vtxStart+7*steps+i);
			
			s_curBuf->PushTri(vtxStart+5*steps+i+1, vtxStart+7*steps+i+1, vtxStart+7*steps+i);
		}
		// 'start' end
		s_curBuf->PushTri(vtxStart+5*steps-1, vtxStart+7*steps-1, vtxStart+4*steps);
		s_curBuf->PushTri(vtxStart+4*steps, vtxStart+7*steps-1, vtxStart+6*steps);
		// 'end' end
		s_curBuf->PushTri(vtxStart+6*steps-1, vtxStart+5*steps, vtxStart+8*steps-1);
		s_curBuf->PushTri(vtxStart+5*steps, vtxStart+7*steps, vtxStart+8*steps-1);
	}
	
	/*
	 * Function: tube
	 *
	 * Hollow cylinder with definable wall thickness
	 *
	 * > tube(steps, start, end, up, innerradius, outerradius)
	 *
	 * Parameters:
	 *
	 *   steps - number of cross-section vertices
	 *   start - start position vector
	 *   end - end position vector
	 *   up - up vector to affect rotation
	 *   innerradius - inner radius
	 *   outerradius - outer radius, must be more than inner
	 *
	 * Example:
	 *
	 * > tube(5, vec(0,0,0), vec(0,20,0), vec(0,1,0), 5.0, 8.0)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int tube(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float inner_radius = lua_tonumber(L, 5);
		float outer_radius = lua_tonumber(L, 6);
		_tube(steps, *start, *end, *updir, inner_radius, outer_radius);
		return 0;
	}

	/*
	 * Function: xref_tuble
	 *
	 * Symmetry version of <tube>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * > xref_tube(steps, start, end, up, innerradius, outerradius)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_tube(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		vector3f start = *MyLuaVec::checkVec(L, 2);
		vector3f end = *MyLuaVec::checkVec(L, 3);
		vector3f updir = *MyLuaVec::checkVec(L, 4);
		float inner_radius = lua_tonumber(L, 5);
		float outer_radius = lua_tonumber(L, 6);
		_tube(steps, start, end, updir, inner_radius, outer_radius);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_tube(steps, start, end, updir, inner_radius, outer_radius);
		return 0;
	}

	static void _tapered_cylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius1, float radius2) {
		const int vtxStart = s_curBuf->AllocVertices(4*steps);

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		radius1 /= cosf(ang);
		radius2 /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p1 = radius1 * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f p2 = radius2 * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f tmp = (end+p2)-(start+p1);
			vector3f n = tmp.Cross(p1).Cross(tmp).Normalized();

			s_curBuf->SetVertex(vtxStart+i, start+p1, n);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p2, n);
			s_curBuf->SetVertex(vtxStart+i+2*steps, start+p1, -dir);
			s_curBuf->SetVertex(vtxStart+i+3*steps, end+p2, dir);
		}

		for (int i=0; i<steps-1; i++) {
			s_curBuf->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			s_curBuf->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		s_curBuf->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		s_curBuf->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);

		for (int i=2; i<steps; i++) {
			// bottom cap
			s_curBuf->PushTri(vtxStart+2*steps, vtxStart+2*steps+i, vtxStart+2*steps+i-1);
			// top cap
			s_curBuf->PushTri(vtxStart+3*steps, vtxStart+3*steps+i-1, vtxStart+3*steps+i);
		}
	}


	/*
	 * Function: tapered_cylinder
	 *
	 * A cylinder with one end wider than the other
	 *
	 * > tapered_cylinder(steps, start, end, up, radius, end_radius)
	 *
	 * Parameters:
	 *
	 *   steps - number of cross-section points
	 *   start - vector start position
	 *   end - vector end position
	 *   up - orientation of the ends (does not rotate the entire shape)
	 *   radius - start radius
	 *   end_radius - end radius
	 *
	 * Example:
	 *
	 * > tapered_cylinder(16*lod,v(0,-200,0),v(0,400,0),v(1,0,0),100,50)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int tapered_cylinder(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius1 = lua_tonumber(L, 5);
		float radius2 = lua_tonumber(L, 6);
		_tapered_cylinder(steps, *start, *end, *updir, radius1, radius2);
		return 0;
	}

	/*
	 * Function: xref_tapered_cylinder
	 *
	 * Symmetry version of <tapered_cylinder>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_tapered_cylinder(lua_State *L)
	{
		/* could optimise for x-reflection but fuck it */
		int steps = luaL_checkinteger(L, 1);
		vector3f start = *MyLuaVec::checkVec(L, 2);
		vector3f end = *MyLuaVec::checkVec(L, 3);
		vector3f updir = *MyLuaVec::checkVec(L, 4);
		float radius1 = lua_tonumber(L, 5);
		float radius2 = lua_tonumber(L, 6);
		_tapered_cylinder(steps, start, end, updir, radius1, radius2);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_tapered_cylinder(steps, start, end, updir, radius1, radius2);
		return 0;
	}

	static void _cylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius) {
		const int vtxStart = s_curBuf->AllocVertices(4*steps);

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			s_curBuf->SetVertex(vtxStart+i, start+p, n);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p, n);
			s_curBuf->SetVertex(vtxStart+i+2*steps, start+p, -dir);
			s_curBuf->SetVertex(vtxStart+i+3*steps, end+p, dir);
		}

		for (int i=0; i<steps-1; i++) {
			s_curBuf->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			s_curBuf->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		s_curBuf->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		s_curBuf->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);

		for (int i=2; i<steps; i++) {
			// bottom cap
			s_curBuf->PushTri(vtxStart+2*steps, vtxStart+2*steps+i, vtxStart+2*steps+i-1);
			// top cap
			s_curBuf->PushTri(vtxStart+3*steps, vtxStart+3*steps+i-1, vtxStart+3*steps+i);
		}
	}
	
	/*
	 * Function: cylinder
	 *
	 * A cylinder (ends will be closed)
	 *
	 * > cylinder(steps, start, end, up, radius)
	 *
	 * Parameters:
	 *
	 *   steps - number of cross-section vertices
	 *   start - vector starting position
	 *   end - vector ending position
	 *   up - orientation of the start and end caps, default (0,0,1). Does not
	 *        rotate the entire shape
	 *   radius - cylinder radius
	 *
	 * Example:
	 *
	 * > cylinder(8, v(-5,0,0), v(5,0,0), v(0,0,1), 3) --horizontal cylinder
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int cylinder(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_cylinder(steps, *start, *end, *updir, radius);
		return 0;
	}

	/*
	 * Function: xref_cylinder
	 *
	 * Symmetry version of <cylinder>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_cylinder(lua_State *L)
	{
		/* could optimise for x-reflection but fuck it */
		int steps = luaL_checkinteger(L, 1);
		vector3f start = *MyLuaVec::checkVec(L, 2);
		vector3f end = *MyLuaVec::checkVec(L, 3);
		vector3f updir = *MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_cylinder(steps, start, end, updir, radius);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_cylinder(steps, start, end, updir, radius);
		return 0;
	}

	static void _ring(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius) {

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const int vtxStart = s_curBuf->AllocVertices(2*steps);

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			s_curBuf->SetVertex(vtxStart+i, start+p, n);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p, n);
		}

		for (int i=0; i<steps-1; i++) {
			s_curBuf->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			s_curBuf->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		s_curBuf->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		s_curBuf->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);
	}

	/*
	 * Function: ring
	 *
	 * Uncapped cylinder.
	 *
	 * > ring(steps, start, end, up, radius)
	 *
	 * Parameters:
	 *
	 *   steps - number of cross-section vertices
	 *   start - vector starting position
	 *   end - vector ending position
	 *   up - orientation of the start and the end, default (0,0,1). Does not
	 *        rotate the entire shape
	 *   radius - cylinder radius
	 *
	 * Example:
	 *
	 * > ring(8, v(5,0,0), v(5,10,0), v(0,0,1), 3) --10m tall tube
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int ring(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_ring(steps, *start, *end, *updir, radius);
		return 0;
	}

	/*
	 * Function: xref_ring
	 *
	 * Symmetry version of <ring>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_ring(lua_State *L)
	{
		int steps = luaL_checkinteger(L, 1);
		vector3f start = *MyLuaVec::checkVec(L, 2);
		vector3f end = *MyLuaVec::checkVec(L, 3);
		vector3f updir = *MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_ring(steps, start, end, updir, radius);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_ring(steps, start, end, updir, radius);
		return 0;
	}

	/*
	 * Function: invisible_tri
	 *
	 * Invisible triangle useful for defining collision surfaces.
	 *
	 * > invisible_tri(v1, v2, v3)
	 *
	 * Parameters:
	 *
	 *   v1 - vector position of the first vertex
	 *   v2 - vector position of the second vertex
	 *   v3 - vector position of the third vertex
	 *
	 * Example:
	 *
	 * > invisible_tri(v(-100,600,-100),v(100,600,100),v(100,600,-100))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int invisible_tri(lua_State *L)
	{
		const vector3f *v1 = MyLuaVec::checkVec(L, 1);
		const vector3f *v2 = MyLuaVec::checkVec(L, 2);
		const vector3f *v3 = MyLuaVec::checkVec(L, 3);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = s_curBuf->PushVertex(*v1, n);
		int i2 = s_curBuf->PushVertex(*v2, n);
		int i3 = s_curBuf->PushVertex(*v3, n);
		s_curBuf->PushInvisibleTri(i1, i2, i3);
		return 0;
	}

	/*
	 * Function: tri
	 *
	 * Define one triangle.
	 *
	 * > tri(v1, v2, v3)
	 *
	 * Parameters:
	 *
	 *   v1 - vector position of the first vertex
	 *   v2 - vector position of the second vertex
	 *   v3 - vector position of the third vertex
	 *
	 * Example:
	 *
	 * > tri(v(-4,-4,0), v(4,-4,0), v(4,4,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int tri(lua_State *L)
	{
		const vector3f *v1 = MyLuaVec::checkVec(L, 1);
		const vector3f *v2 = MyLuaVec::checkVec(L, 2);
		const vector3f *v3 = MyLuaVec::checkVec(L, 3);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = s_curBuf->PushVertex(*v1, n);
		int i2 = s_curBuf->PushVertex(*v2, n);
		int i3 = s_curBuf->PushVertex(*v3, n);
		s_curBuf->PushTri(i1, i2, i3);
		return 0;
	}
	
	/*
	 * Function: xref_tri
	 *
	 * Symmetry version of <tri>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_tri(lua_State *L)
	{
		vector3f v1 = *MyLuaVec::checkVec(L, 1);
		vector3f v2 = *MyLuaVec::checkVec(L, 2);
		vector3f v3 = *MyLuaVec::checkVec(L, 3);
		
		vector3f n = (v1-v2).Cross(v1-v3).Normalized();
		int i1 = s_curBuf->PushVertex(v1, n);
		int i2 = s_curBuf->PushVertex(v2, n);
		int i3 = s_curBuf->PushVertex(v3, n);
		s_curBuf->PushTri(i1, i2, i3);
		v1.x = -v1.x; v2.x = -v2.x; v3.x = -v3.x; n.x = -n.x;
		i1 = s_curBuf->PushVertex(v1, n);
		i2 = s_curBuf->PushVertex(v2, n);
		i3 = s_curBuf->PushVertex(v3, n);
		s_curBuf->PushTri(i1, i3, i2);
		return 0;
	}
	
	/*
	 * Function: quad
	 *
	 * Define a quad (plane, one sided).
	 *
	 * > quad(v1, v2, v3, v4)
	 *
	 * Parameters:
	 *
	 *   v1 - vector location of first vertex
	 *   v2 - vector location of second vertex
	 *   v3 - vector location of third vertex
	 *   v4 - vector location of fourth vertex
	 *
	 * Example:
	 *
	 * > quad(v(-4,-4,0), v(4,-4,0), v(4,4,0), v(-4,4,0))
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int quad(lua_State *L)
	{
		const vector3f *v1 = MyLuaVec::checkVec(L, 1);
		const vector3f *v2 = MyLuaVec::checkVec(L, 2);
		const vector3f *v3 = MyLuaVec::checkVec(L, 3);
		const vector3f *v4 = MyLuaVec::checkVec(L, 4);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = s_curBuf->PushVertex(*v1, n);
		int i2 = s_curBuf->PushVertex(*v2, n);
		int i3 = s_curBuf->PushVertex(*v3, n);
		int i4 = s_curBuf->PushVertex(*v4, n);
		s_curBuf->PushTri(i1, i2, i3);
		s_curBuf->PushTri(i1, i3, i4);
		return 0;
	}
	
	/*
	 * Function: xref_quad
	 *
	 * Symmetry version of <quad>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_quad(lua_State *L)
	{
		vector3f v1 = *MyLuaVec::checkVec(L, 1);
		vector3f v2 = *MyLuaVec::checkVec(L, 2);
		vector3f v3 = *MyLuaVec::checkVec(L, 3);
		vector3f v4 = *MyLuaVec::checkVec(L, 4);
		
		vector3f n = (v1-v2).Cross(v1-v3).Normalized();
		int i1 = s_curBuf->PushVertex(v1, n);
		int i2 = s_curBuf->PushVertex(v2, n);
		int i3 = s_curBuf->PushVertex(v3, n);
		int i4 = s_curBuf->PushVertex(v4, n);
		s_curBuf->PushTri(i1, i2, i3);
		s_curBuf->PushTri(i1, i3, i4);
		v1.x = -v1.x; v2.x = -v2.x; v3.x = -v3.x; v4.x = -v4.x; n.x = -n.x;
		i1 = s_curBuf->PushVertex(v1, n);
		i2 = s_curBuf->PushVertex(v2, n);
		i3 = s_curBuf->PushVertex(v3, n);
		i4 = s_curBuf->PushVertex(v4, n);
		s_curBuf->PushTri(i1, i3, i2);
		s_curBuf->PushTri(i1, i4, i3);
		return 0;
	}

	/*
	 * Function: thruster
	 *
	 * Define a position for a ship thruster.
	 * 
	 * Thrusters are purely a visual effect and do not affect handling characteristics.
	 *
	 * > thruster(position, direction, size, linear_only)
	 *
	 * Parameters:
	 *
	 *   position - position vector
	 *   direction - direction vector, pointing "away" from the ship, 
	 *               determines also when the thruster is actually animated
	 *   size - scale of the thruster flame
	 *   linear_only - only appear for linear (back, forward) thrust
	 *
	 * Example:
	 *
	 * > thruster(v(0,5,-10), v(0,1,0), 10) --top thruster
	 * > thruster(v(0,0,5), v(0,0,1), 30, true) --back thruster
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int thruster(lua_State *L)
	{
		const vector3f *pos = MyLuaVec::checkVec(L, 1);
		const vector3f *dir = MyLuaVec::checkVec(L, 2);
		const float power = luaL_checknumber(L, 3);
		bool linear_only = false;
		if (lua_isboolean(L, 4)) {
			linear_only = lua_toboolean(L, 4) != 0;
		}
		s_curBuf->PushThruster(*pos, *dir, power, linear_only);
		return 0;
	}

	/*
	 * Function: xref_thruster
	 *
	 * Symmetry version of <thruster>. Result will be duplicated and mirrored
	 * along the X axis.
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int xref_thruster(lua_State *L)
	{
		vector3f pos = *MyLuaVec::checkVec(L, 1);
		const vector3f *dir = MyLuaVec::checkVec(L, 2);
		const float power = luaL_checknumber(L, 3);
		bool linear_only = false;
		if (lua_isboolean(L, 4)) {
			linear_only = lua_toboolean(L, 4) != 0;
		}
		s_curBuf->PushThruster(pos, *dir, power, linear_only);
		pos.x = -pos.x;
		s_curBuf->PushThruster(pos, *dir, power, linear_only);
		return 0;
	}

	/*
	 * Function: get_time
	 *
	 * Get the game time. Use this to run continuous animations.
	 * For example, blinking lights, rotating radar dishes and church tower
	 * clock hands.
	 *
	 * > local seconds, minutes, hours, days = get_time()
	 * > local seconds = get_time('SECONDS')
	 * > local minutes = get_time('MINUTES')
	 * > local hours = get_time('HOURS')
	 * > local days = get_time('DAYS')
	 *
	 * Parameters:
	 *
	 *   units - optional. If specified, there will be one return value, in
	 *           the specified units. Otherwise, all four units are returned.
	 *           available units are: 'SECONDS', 'MINUTES', 'HOURS', 'DAYS'
	 *
	 * Returns:
	 *
	 *   seconds - the time in seconds
	 *   hours   - the time in hours
	 *   minutes - the time in minutes
	 *   days    - the time in days
	 *
	 *   The above values include fractional components.
	 *
	 * Example:
	 *
	 * > local seconds = get_time('SECONDS')
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_time(lua_State *L)
	{
		assert(s_curParams != 0);
		double t = s_curParams->time;
		int nparams = lua_gettop(L);
		if (nparams == 0) {
			lua_pushnumber(L, t);
			lua_pushnumber(L, t / 60.0);
			lua_pushnumber(L, t / 3600.0);
			lua_pushnumber(L, t / (24*3600.0));
			return 4;
		} else if (nparams == 1) {
			const char *units = luaL_checkstring(L, 1);
			if (strcmp(units, "SECONDS") == 0)
				lua_pushnumber(L, t);
			else if (strcmp(units, "MINUTES") == 0)
				lua_pushnumber(L, t / 60.0);
			else if (strcmp(units, "HOURS") == 0)
				lua_pushnumber(L, t / 3600.0);
			else if (strcmp(units, "DAYS") == 0)
				lua_pushnumber(L, t / (24 * 3600.0));
			else
				return luaL_error(L,
					"Unknown unit type '%s' specified for get_time "
					"(expected 'SECONDS', 'MINUTES', 'HOURS' or 'DAYS').", units);
			return 1;
		} else {
			return luaL_error(L, "Expected 0 or 1 parameters, but got %d.", nparams);
		}
	}

	/*
	 * Function: get_equipment
	 *
	 * Get the type of equipment mounted in a particular slot.
	 * Only valid for ship models.
	 *
	 * > local equip_type = get_equipment(slot, index)
	 *
	 * Parameters:
	 *
	 *   slot - a slot name, from <Constants.EquipSlot>
	 *   index - the item index within that slot (optional; 1-based index)
	 *
	 * Returns:
	 *
	 *   equip_type - a <Constants.EquipType> string, or 'nil' if there is
	 *                no equipment in that slot.
	 *
	 *   If no index is specified, then all equipment in the specified slot
	 *   is returned (as separate return values)
	 *
	 * Example:
	 *
	 * > if get_equipment('FUELSCOOP')
	 * > local missile1, missile2, missile3, missile4 = get_equipment('MISSILE')
	 * > local missile2 = get_equipment('MISSILE', 2)
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_equipment(lua_State *L)
	{
		assert(s_curParams != 0);
		if (s_curParams->equipment) {
			const char *slotName = luaL_checkstring(L, 1);
			int index = luaL_optinteger(L, 2, 0);
			Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(L, "EquipSlot", slotName));

			if (index > 0) {
				// index - 1 because Lua uses 1-based indexing
				Equip::Type equip = s_curParams->equipment->Get(slot, index - 1);
				if (equip == Equip::NONE)
					lua_pushnil(L);
				else
					lua_pushstring(L, LuaConstants::GetConstantString(L, "EquipType", equip));
				return 1;
			} else {
				const EquipSet &es = *s_curParams->equipment;
				const int slotSize = es.GetSlotSize(slot);
				int i = 0, count = 0;
				Equip::Type equip = Equip::NONE;
				while (i < slotSize) {
					equip = es.Get(slot, i++);
					if (equip != Equip::NONE) {
						PiVerify(lua_checkstack(L, 1));
						lua_pushstring(L, LuaConstants::GetConstantString(L, "EquipType", equip));
						++count;
					}
				}
				return count;
			}
		} else
			return luaL_error(L, "Equipment is only valid for ships.");
	}

	/*
	 * Function: get_animation_stage
	 *
	 * Get the stage of an animation. The meaning of this depends on the animation.
	 *
	 * > local stage = get_animation_stage(animation)
	 *
	 * Parameters:
	 *
	 *   animation - an animation name, from <Constants.ShipAnimation> for ships
	 *               or from <Constants.SpaceStationAnimation> for space stations
	 *
	 * Returns:
	 *
	 *   stage - the stage of the animation (meaning is animation dependent)
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_animation_stage(lua_State *L)
	{
		assert(s_curParams != 0);
		if (s_curParams->animationNamespace) {
			const char *animName = luaL_checkstring(L, 1);
			int anim = LuaConstants::GetConstant(L, s_curParams->animationNamespace, animName);
			assert(anim >= 0 && anim < LmrObjParams::LMR_ANIMATION_MAX);
			lua_pushinteger(L, s_curParams->animStages[anim]);
			return 1;
		} else
			return luaL_error(L, "You can only use get_animation_stage for model types that are supposed to have animations.");
	}

	/*
	 * Function: get_animation_position
	 *
	 * Get the position of an animation.
	 *
	 * > local pos = get_animation_position(animation)
	 *
	 * Parameters:
	 *
	 *   animation - an animation name, from <Constants.ShipAnimation> for ships
	 *               or from <Constants.SpaceStationAnimation> for space stations
	 *
	 * Returns:
	 *
	 *   pos - the position of the animation (typically from 0 to 1)
	 *
	 * Example:
	 *
	 * > local pos = get_animation_position('WHEEL_STATE')
	 * > -- display landing gear in appropriate position
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_animation_position(lua_State *L)
	{
		assert(s_curParams != 0);
		if (s_curParams->animationNamespace) {
			const char *animName = luaL_checkstring(L, 1);
			int anim = LuaConstants::GetConstant(L, s_curParams->animationNamespace, animName);
			assert(anim >= 0 && anim < LmrObjParams::LMR_ANIMATION_MAX);
			lua_pushnumber(L, s_curParams->animValues[anim]);
			return 1;
		} else
			return luaL_error(L, "You can only use get_animation_position for model types that are supposed to have animations.");
	}

	/*
	 * Function: get_flight_state
	 *
	 * Get the flight state of the ship.
	 *
	 * > local state = get_flight_state()
	 *
	 * Returns:
	 *
	 *   state - one of the flight state constants from <Constants.ShipFlightState>
	 *
	 * Example:
	 *
	 * > local flight_state = get_flight_state()
	 * > if flight_state == 'LANDED' then
	 * >   -- enable rough landing lights
	 * > end
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_flight_state(lua_State *L)
	{
		assert(s_curParams != 0);
		// if there is equipment then there should also be a flightState
		if (s_curParams->equipment) {
			lua_pushstring(L, LuaConstants::GetConstantString(L, "ShipFlightState", s_curParams->flightState));
			return 1;
		} else
			return luaL_error(L, "Flight state is only valid for ships.");
	}

	/*
	 * Function: get_label
	 *
	 * Return the main label string to display on an object.
	 * For ships this is the registration ID, for stations it's the
	 * station name, for cargo pods it's the contents.
	 *
	 * > local label = get_label()
	 *
	 * Returns:
	 *
	 *   label - the main string to display on the object
	 *
	 * Example:
	 *
	 * > local regid = get_label()
	 * > text(regid, v(0,0,0), v(0,0,1), v(1,0,0), 10.0)
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_label(lua_State *L)
	{
		assert(s_curParams != 0);
		lua_pushstring(L, s_curParams->label ? s_curParams->label : "");
		return 1;
	}

	/*
	 * Function: get_arg_material
	 *
	 * Return material parameters passed from C++ code
	 *
	 * > get_arg_material(index)
	 *
	 * Parameters:
	 *
	 *   index - argument number. Used arguments are:
	 *           - 0, primary ship flavour material (shinyness is somewhat random)
	 *           - 1, secondary ship flavour material (shinyness is somewhat random)
	 *           - 2, completely white, shine-less material
	 *
	 * Example:
	 *
	 * > set_material('body', get_arg_material(0))
	 * > use_material('body')
	 * > load_obj('hull.obj')
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int get_arg_material(lua_State *L)
	{
		assert(s_curParams != 0);
		int n = luaL_checkinteger(L, 1);
		lua_createtable (L, 11, 0);

		const LmrMaterial &mat = s_curParams->pMat[n];

		for (int i=0; i<4; i++) {
			lua_pushinteger(L, 1+i);
			lua_pushnumber(L, mat.diffuse[i]);
			lua_settable(L, -3);
		}
		for (int i=0; i<3; i++) {
			lua_pushinteger(L, 5+i);
			lua_pushnumber(L, mat.specular[i]);
			lua_settable(L, -3);
		}
		lua_pushinteger(L, 8);
		lua_pushnumber(L, mat.shininess);
		lua_settable(L, -3);
		for (int i=0; i<3; i++) {
			lua_pushinteger(L, 9+i);
			lua_pushnumber(L, mat.emissive[i]);
			lua_settable(L, -3);
		}
		return 1;
	}

	/*
	 * Function: billboard
	 *
	 * Textured plane that always faces the camera.
	 * 
	 * Does not use materials, will not affect collisions.
	 *
	 * > billboard(texture, size, color, points)
	 *
	 * Parameters:
	 *
	 *   texture - texture file to use
	 *   size - billboard size
	 *   color - rgba vector
	 *   points - table of vertices to define several billboards and their
	 *            positions, supply at least one e.g. { v(0,0,0) }
	 *
	 * Example:
	 *
	 * > billboard('halo.png', 10, v(0,1,0), { v(0,0,0) }) --greenish light sprite
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int billboard(lua_State *L)
	{
//		billboard('texname', size, color, { p1, p2, p3, p4 })
		const char *texname = luaL_checkstring(L, 1);
		const float size = luaL_checknumber(L, 2);
		const vector3f color = *MyLuaVec::checkVec(L, 3);
		std::vector<vector3f> points;

		if (lua_istable(L, 4)) {
			for (int i=1;; i++) {
				lua_pushinteger(L, i);
				lua_gettable(L, 4);
				if (lua_isnil(L, -1)) {
					lua_pop(L, 1);
					break;
				}
				points.push_back(*MyLuaVec::checkVec(L, -1));
				lua_pop(L, 1);
			}
		}
		s_curBuf->PushBillboards(texname, size, color, points.size(), &points[0]);
		return 0;
	}
	////////////////////////////////////////////////////////////////
	
#define ICOSX	0.525731112119133f
#define ICOSZ	0.850650808352039f

	static const vector3f icosahedron_vertices[12] = {
		vector3f(-ICOSX, 0.0, ICOSZ), vector3f(ICOSX, 0.0, ICOSZ), vector3f(-ICOSX, 0.0, -ICOSZ), vector3f(ICOSX, 0.0, -ICOSZ),
		vector3f(0.0, ICOSZ, ICOSX), vector3f(0.0, ICOSZ, -ICOSX), vector3f(0.0, -ICOSZ, ICOSX), vector3f(0.0, -ICOSZ, -ICOSX),
		vector3f(ICOSZ, ICOSX, 0.0), vector3f(-ICOSZ, ICOSX, 0.0), vector3f(ICOSZ, -ICOSX, 0.0), vector3f(-ICOSZ, -ICOSX, 0.0)
	};

	static const int icosahedron_faces[20][3] = {
		{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
		{8,10,1}, {8,3,10},{5,3,8}, {5,2,3}, {2,7,3},
		{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
		{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
	};

	static void _sphere_subdivide (const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
			const int i1, const int i2, const int i3, int depth)
	{
		if (depth == 0) {
			s_curBuf->PushTri(i1, i3, i2);
			return;
		}

		const vector3f v12 = (v1+v2).Normalized();
		const vector3f v23 = (v2+v3).Normalized();
		const vector3f v31 = (v3+v1).Normalized();
		const int i12 = s_curBuf->PushVertex(trans * v12, trans.ApplyRotationOnly(v12));
		const int i23 = s_curBuf->PushVertex(trans * v23, trans.ApplyRotationOnly(v23));
		const int i31 = s_curBuf->PushVertex(trans * v31, trans.ApplyRotationOnly(v31));
		_sphere_subdivide(trans, v1, v12, v31, i1, i12, i31, depth-1);
		_sphere_subdivide(trans, v2, v23, v12, i2, i23, i12, depth-1);
		_sphere_subdivide(trans, v3, v31, v23, i3, i31, i23, depth-1);
		_sphere_subdivide(trans, v12, v23, v31, i12, i23, i31, depth-1);
	}
	static void _get_orientation(lua_State *l, int stackpos, matrix4x4f &trans)
	{
		if ((lua_gettop(l) < stackpos) || lua_isnil(l, stackpos)) {
			trans = matrix4x4f::Identity();
		} else {
			trans = *MyLuaMatrix::checkMatrix(l, stackpos);
		}
	}


	/*
	 * Function: sphere
	 *
	 * Icosahedron style sphere.
	 *
	 * > sphere(subdivisions, transform)
	 *
	 * Parameters:
	 *
	 *   subdivisions - times to subdivide the model, icosahedron has twenty sides
	 *   transform - optional transform matrix
	 *
	 * Example:
	 *
	 * > sphere(0) --standard 20 triangles
	 * > sphere(3) --a lot smoother (1280 triangles)
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int sphere (lua_State *l)
	{
		int i, subdivs;
		matrix4x4f trans;
		subdivs = luaL_checkinteger(l, 1);
		if ((subdivs < 0) || (subdivs > 4)) {
			luaL_error(l, "sphere(subdivs, transform): subdivs must be in range [0,4]");
		}
		_get_orientation(l, 2, trans);

		int vi[12];
		for (i=0; i<12; i++) {
			const vector3f &v = icosahedron_vertices[i];
			vi[i] = s_curBuf->PushVertex(trans * v, trans.ApplyRotationOnly(v));
		}
			
		for (i=0; i<20; i++) {
			_sphere_subdivide (trans, icosahedron_vertices[icosahedron_faces[i][0]],
					icosahedron_vertices[icosahedron_faces[i][1]],
					icosahedron_vertices[icosahedron_faces[i][2]],
					vi[icosahedron_faces[i][0]],
					vi[icosahedron_faces[i][1]],
					vi[icosahedron_faces[i][2]],
					subdivs);
		}
		return 0;
	}


	/*
	 * Function: sphere_slice
	 *
	 * Partially sliced sphere. For domes and such.
	 * 
	 * The resulting shape will be capped (closed).
	 *
	 * > sphere_slice(lat_segs, long_segs, angle1, angle2, transform)
	 *
	 * Parameters:
	 *
	 *   lat_segs - latitudinal subdivisions
	 *   long_segs - longitudinal subdivisions
	 *   angle1 - angle, or amount to slice from bottom, 0.5*pi would be halfway
	 *   angle2 - slice angle from top
	 *   transform - matrix transform to translate, rotate or scale the result
	 *
	 * Example:
	 *
	 * > sphere_slice(6,6,0.5*math.pi,0.0, Matrix.scale(v(2,2,2))) --slice off bottom half
	 * > sphere_slice(6,6,0.5*math.pi,0.2*math.pi, Matrix.scale(v(2,2,2))) --take off a bit from top as well
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	static int sphere_slice(lua_State *l)
	{
		int LAT_SEGS;
		int LONG_SEGS;
		float sliceAngle1, sliceAngle2;
		LONG_SEGS = luaL_checkinteger(l, 1);
		LAT_SEGS = luaL_checkinteger(l, 2);
		sliceAngle1 = luaL_checknumber(l, 3);
		sliceAngle2 = luaL_checknumber(l, 4);
			//luaL_error(l, "sphere(subdivs, transform): subdivs must be in range [0,4]");
		matrix4x4f trans;
		_get_orientation(l, 5, trans);
		const vector3f yaxis(trans[4], trans[5], trans[6]);
		float latDiff = (sliceAngle2-sliceAngle1) / float(LAT_SEGS);

		float rot = 0.0;
		float *sinTable = static_cast<float*>(alloca(sizeof(float)*(LONG_SEGS+1)));
		float *cosTable = static_cast<float*>(alloca(sizeof(float)*(LONG_SEGS+1)));
		for (int i=0; i<=LONG_SEGS; i++, rot += 2.0*M_PI/float(LONG_SEGS)) {
			sinTable[i] = float(sin(rot));
			cosTable[i] = float(cos(rot));
		}

		int *idx = new int[LONG_SEGS+2];
		int *idx2 = new int[LONG_SEGS+2];
		// cap the top
		float cosLat2 = cos(sliceAngle1);
		float sinLat2 = sin(sliceAngle1);
		vector3f cap_norm = yaxis.Normalized();
		for (int i=0; i<=LONG_SEGS; i++) {
			vector3f v0(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
			idx[i] = s_curBuf->PushVertex(trans * v0, cap_norm);
			idx2[i] = s_curBuf->PushVertex(trans * v0, trans.ApplyRotationOnly(v0)); // for later
		}
		for (int i=0; i<LONG_SEGS-1; i++) {
			s_curBuf->PushTri(idx[0], idx[i+2], idx[i+1]);
		}

		for (int j=1; j<=LAT_SEGS; j++) {
			cosLat2 = cos(sliceAngle1+latDiff*j);
			sinLat2 = sin(sliceAngle1+latDiff*j);
			for (int i=0; i<=LONG_SEGS; i++) {
				vector3f v1(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
				idx[i] = idx2[i];
				idx2[i] = s_curBuf->PushVertex(trans * v1, trans.ApplyRotationOnly(v1));
			}
			for (int i=0; i<LONG_SEGS; i++) {
				s_curBuf->PushTri(idx[i], idx2[i+1], idx2[i]);
				s_curBuf->PushTri(idx[i], idx[i+1], idx2[i+1]);
			}
		}
		// cap the bottom
		cap_norm = -cap_norm;
		for (int i=0; i<=LONG_SEGS; i++) {
			vector3f v1(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
			idx[i] = s_curBuf->PushVertex(trans * v1, cap_norm);
		}
		for (int i=0; i<LONG_SEGS-1; i++) {
			s_curBuf->PushTri(idx[0], idx[i+1], idx[i+2]);
		}
		delete [] idx;
		delete [] idx2;

		return 0;
	}



} /* namespace ModelFuncs */

namespace ObjLoader {
	static void trim(char *input) {
		char *output = input;
		char *end = output;
		char c;

		while(*input && isspace(*input))
			++input;

		while(*input) {
			c = *(output++) = *(input++);
			if( !isspace(c) )
				end = output;
		}

		*end = 0;
	}

	static std::map<std::string, std::string> load_mtl_file(lua_State *L, const char* mtl_file) {
		std::map<std::string, std::string> mtl_map;
		char buf[1024], name[1024] = "", file[1024];

		lua_getglobal(L, "CurrentDirectory");
		std::string curdir = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		snprintf(buf, sizeof(buf), "%s/%s", curdir.c_str(), mtl_file);
		FILE *f = fopen(buf, "r");
		if (!f) {
			printf("Could not open %s\n", buf);
			throw LmrUnknownMaterial();
		}

		for (int line_no=1; fgets(buf, sizeof(buf), f); line_no++) {
			trim(buf);
			if (!strncasecmp(buf, "newmtl ", 7)) {
				PiVerify(1 == sscanf(buf, "newmtl %s", name));
			}
			if (!strncasecmp(buf, "map_K", 5) && strlen(name) > 0) {
				PiVerify(1 == sscanf(buf, "map_Kd %s", file));
				mtl_map[name] = file;
			}
		}
		fclose(f);
		return mtl_map;
	}

	/*
	 * Function: load_obj
	 *
	 * Load a Wavefront OBJ model file.
	 * 
	 * If an associated .mtl material definition file is found, Pioneer will
	 * attempt to interpret it the best it can, including texture usage. Note that
	 * Pioneer supports only one texture per .obj file.
	 *
	 * > load_obj(modelname, transform)
	 *
	 * Parameters:
	 *
	 *   modelname - .obj file name to load
	 *   transform - optional transform matrix, for example Matrix.scale(v(2,2,2))
	 *               will double the model scale along all three axes
	 *
	 * Example:
	 *
	 * > load_obj_file('wing.obj')
   * > load_obj_file('wing.obj', Matrix.translate(v(-5,0,0)) --shift left
	 *
	 * Availability:
	 *
	 *   pre-alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 *
	 */
	struct objTriplet {	int v, n, uv; };
	const bool operator< (const objTriplet &t1, const objTriplet &t2) {
		if (t1.v < t2.v) return true; if (t1.v > t2.v) return false;
		if (t1.n < t2.n) return true; if (t1.n > t2.n) return false;
		if (t1.uv < t2.uv) return true; return false;
	}

	static int load_obj_file(lua_State *L)
	{
		const char *obj_name = luaL_checkstring(L, 1);
		int numArgs = lua_gettop(L);
		matrix4x4f *transform = 0;
		if (numArgs > 1) {
			transform = MyLuaMatrix::checkMatrix(L, 2);
		}

		lua_getglobal(L, "CurrentDirectory");
		std::string curdir = luaL_checkstring(L, -1);
		lua_pop(L, 1);
	
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s/%s", curdir.c_str(), obj_name);
		FILE *f = fopen(buf, "r");
		if (!f) {
			Error("Could not open %s\n", buf);
		}
		std::vector<vector3f> vertices;
		std::vector<vector3f> texcoords;
		std::vector<vector3f> normals;
		std::map<std::string, std::string> mtl_map;
		std::string texture;

		// maps obj file vtx_idx,norm_idx to a single GeomBuffer vertex index
		std::map<objTriplet, int> vtxmap;

		for (int line_no=1; fgets(buf, sizeof(buf), f); line_no++) {
			if ((buf[0] == 'v') && buf[1] == ' ') {
				// vertex
				vector3f v;
				PiVerify(3 == sscanf(buf, "v %f %f %f", &v.x, &v.y, &v.z));
				if (transform) v = (*transform) * v;
				vertices.push_back(v);
			}
			else if ((buf[0] == 'v') && (buf[1] == 'n') && (buf[2] == ' ')) {
				// normal
				vector3f v;
				PiVerify(3 == sscanf(buf, "vn %f %f %f", &v.x, &v.y, &v.z));
				if (transform) v = ((*transform) * v).Normalized();
				normals.push_back(v);
			}
			else if ((buf[0] == 'v') && (buf[1] == 't') && (buf[2] == ' ')) {
				// texture
				vector3f v;
				PiVerify(2 == sscanf(buf, "vt %f %f", &v.x, &v.y));
				//max, blender use 0,0 as lower left so flip vertical
				v.y = 1.0 - v.y;
				texcoords.push_back(v);
			}
			else if ((buf[0] == 'f') && (buf[1] == ' ')) {
				// how many vertices in this face?
				const int MAX_VTX_FACE = 64;
				char *bit[MAX_VTX_FACE];
				char *pos = &buf[2];
				int numBits = 0;
				while ((pos[0] != '\0') && (numBits < MAX_VTX_FACE)) {
					bit[numBits++] = pos;
					while(pos[0] && !isspace(pos[0])) pos++;
					while (isspace(pos[0])) pos++;
				}

				int realVtxIdx[MAX_VTX_FACE];
				int vi[MAX_VTX_FACE], ni[MAX_VTX_FACE], ti[MAX_VTX_FACE];
				bool build_normals = false;
				for (int i=0; i<numBits; i++) {
					if (3 == sscanf(bit[i], "%d/%d/%d", &vi[i], &ti[i], &ni[i])) {
						// good
					}
					else if (2 == sscanf(bit[i], "%d//%d", &vi[i], &ni[i])) {
						// good
					}
					else if (1 == sscanf(bit[i], "%d", &vi[i])) {
						build_normals = true;
					} else {
						puts(bit[i]);
						Error("Obj file has no normals or is otherwise too weird at line %d\n", line_no);
					}
					// indices start from 1 in obj file
					vi[i]--; ni[i]--;ti[i]--;
				}

				if (build_normals) {
					// not nice without normals
					for (int i=0; i<numBits-2; i++) {
						vector3f &a = vertices[vi[0]];
						vector3f &b = vertices[vi[i+1]];
						vector3f &c = vertices[vi[i+2]];
						vector3f n = (a-b).Cross(a-c).Normalized();
						int vtxStart = s_curBuf->AllocVertices(3);
						if (texcoords.empty()) {
							// no UV coords
							s_curBuf->SetVertex(vtxStart, a, n);
							s_curBuf->SetVertex(vtxStart+1, b, n);
							s_curBuf->SetVertex(vtxStart+2, c, n);
						} else {
							s_curBuf->SetVertex(vtxStart, a, n, texcoords[ti[i]].x, texcoords[ti[i]].y);
							s_curBuf->SetVertex(vtxStart+1, b, n, texcoords[ti[i+1]].x, texcoords[ti[i+1]].y);
							s_curBuf->SetVertex(vtxStart+2, c, n, texcoords[ti[i+2]].x, texcoords[ti[i+2]].y);
						}
						if (texture.size()) s_curBuf->SetTexture(texture.c_str());
						s_curBuf->PushTri(vtxStart, vtxStart+1, vtxStart+2);
					}
				} else {
					for (int i=0; i<numBits; i++) {
						// SHARE THE PAIN!
						objTriplet t = { vi[i], ni[i], ti[i] };
						std::map<objTriplet, int>::iterator it = vtxmap.find(t);
						if (it == vtxmap.end()) {
							// insert the horrible thing
							int vtxStart = s_curBuf->AllocVertices(1);
							if (texcoords.empty()) {
								// no UV coords
								s_curBuf->SetVertex(vtxStart, vertices[vi[i]], normals[ni[i]]);
							} else {
								s_curBuf->SetVertex(vtxStart, vertices[vi[i]], normals[ni[i]], texcoords[ti[i]].x, texcoords[ti[i]].y);
							}
							vtxmap[t] = vtxStart;
							realVtxIdx[i] = vtxStart;
						} else {
							realVtxIdx[i] = (*it).second;
						}
					}
					if (texture.size()) s_curBuf->SetTexture(texture.c_str());
					if (numBits == 3) {
						s_curBuf->PushTri(realVtxIdx[0], realVtxIdx[1], realVtxIdx[2]);
					} else if (numBits == 4) {
						s_curBuf->PushTri(realVtxIdx[0], realVtxIdx[1], realVtxIdx[2]);
						s_curBuf->PushTri(realVtxIdx[0], realVtxIdx[2], realVtxIdx[3]);
					} else {
						Error("Obj file must have faces with 3 or 4 vertices (quads or triangles)\n");
					}
				}
			}
			else if (strncmp("mtllib ", buf, 7) == 0) {
				char lib_name[128];
				if (1 == sscanf(buf, "mtllib %s", lib_name)) {
					mtl_map = load_mtl_file(L, lib_name);
				}
			}
			else if (strncmp("usemtl ", buf, 7) == 0) {
				char mat_name[128];
				if (1 == sscanf(buf, "usemtl %s", mat_name)) {
					if ( mtl_map.find(mat_name) != mtl_map.end() ) {
						try {
							char texfile[256];
							snprintf(texfile, sizeof(texfile), "%s/%s", curdir.c_str(), mtl_map[mat_name].c_str());
							texture = texfile;
						} catch (LmrUnknownMaterial) {
							printf("Warning: Missing material %s (%s) in %s\n", mtl_map[mat_name].c_str(), mat_name, obj_name);
						}
					}
				} else {
					Error("Obj file has no normals or is otherwise too weird at line %d\n", line_no);
				}
			}
		}
		fclose(f);
		return 0;
	}
}

namespace UtilFuncs {
	
	int noise(lua_State *L) {
		vector3d v;
		if (lua_isnumber(L, 1)) {
			v.x = lua_tonumber(L, 1);
			v.y = lua_tonumber(L, 2);
			v.z = lua_tonumber(L, 3);
		} else {
			v = vector3d(*MyLuaVec::checkVec(L, 1));
		}
		lua_pushnumber(L, noise(v));
		return 1;
	}

} /* UtilFuncs */

static int define_model(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) {
		luaL_error(L, "define_model takes 2 arguments");
		return 0;
	}

	const char *model_name = luaL_checkstring(L, 1);

	if (!lua_istable(L, 2)) {
		luaL_error(L, "define_model 2nd argument must be a table");
		return 0;
	}

	if (s_models.find(model_name) != s_models.end()) {
		fprintf(stderr, "attempt to redefine model %s\n", model_name);
		return 0;
	}

	// table is passed containing info, static and dynamic, which are
	// functions. we then stuff them into the globals, named
	// modelName_info, _static, etc.
	char buf[256];

	lua_pushstring(L, "info");
	lua_gettable(L, 2);
	snprintf(buf, sizeof(buf), "%s_info", model_name);
	lua_setglobal(L, buf);

	lua_pushstring(L, "static");
	lua_gettable(L, 2);
	snprintf(buf, sizeof(buf), "%s_static", model_name);
	lua_setglobal(L, buf);

	lua_pushstring(L, "dynamic");
	lua_gettable(L, 2);
	snprintf(buf, sizeof(buf), "%s_dynamic", model_name);
	lua_setglobal(L, buf);
	
	s_models[model_name] = new LmrModel(model_name);
	return 0;
}

static Uint32 s_allModelFilesCRC = 0;
static void _makeModelFilesCRC(const std::string &dir, const std::string &filename)
{
	struct stat info;
	if (stat(filename.c_str(), &info)) return;

	if (S_ISDIR(info.st_mode)) {
		foreach_file_in(filename, &_makeModelFilesCRC);
	} else if (S_ISREG(info.st_mode) && (filename.substr(filename.size()-4) != ".png")) {
		FILE *f = fopen(filename.c_str(), "rb");
		if (f) {
			fseek(f, 0, SEEK_END);
			int size = ftell(f);
			unsigned char *data = reinterpret_cast<unsigned char *>(malloc(size));
			fseek(f, 0, SEEK_SET);
			fread(data, 1, size, f);
			for (int c=0; c<size; c++) s_allModelFilesCRC += data[c];
			free(data);
			fclose(f);
		}
	}
}

static void _detect_model_changes()
{
	// do we need to rebuild the model cache?
	foreach_file_in(PIONEER_DATA_DIR "/models", &_makeModelFilesCRC);

	FILE *cache_sum_file = fopen(join_path(s_cacheDir.c_str(), "cache.sum", 0).c_str(), "rb");
	if (cache_sum_file) {
		if ((_fread_string(cache_sum_file) == PIONEER_VERSION) &&
		    (_fread_string(cache_sum_file) == PIONEER_EXTRAVERSION)) {
			Uint32 crc;
			fread_or_die(&crc, sizeof(crc), 1, cache_sum_file);
			if (crc == s_allModelFilesCRC) {
				s_recompileAllModels = false;
			}
		}
		fclose(cache_sum_file);
	}
	if (s_recompileAllModels) printf("Rebuilding model cache...\n");
}

static void _write_model_crc_file()
{
	if (s_recompileAllModels) {
		FILE *cache_sum_file = fopen(join_path(s_cacheDir.c_str(), "cache.sum", 0).c_str(), "wb");
		if (cache_sum_file) {
			_fwrite_string(PIONEER_VERSION, cache_sum_file);
			_fwrite_string(PIONEER_EXTRAVERSION, cache_sum_file);
			fwrite(&s_allModelFilesCRC, sizeof(s_allModelFilesCRC), 1, cache_sum_file);
			fclose(cache_sum_file);
		}
	}
}

void LmrModelCompilerInit(TextureCache *textureCache)
{
	s_textureCache = textureCache;

	s_cacheDir = GetPiUserDir("model_cache");
	_detect_model_changes();

	s_staticBufferPool = new BufferObjectPool<sizeof(Vertex)>();
	s_sunlightShader[0] = new LmrShader("model", "#define NUM_LIGHTS 1\n");
	s_sunlightShader[1] = new LmrShader("model", "#define NUM_LIGHTS 2\n");
	s_sunlightShader[2] = new LmrShader("model", "#define NUM_LIGHTS 3\n");
	s_sunlightShader[3] = new LmrShader("model", "#define NUM_LIGHTS 4\n");
	s_pointlightShader[0] = new LmrShader("model-pointlit", "#define NUM_LIGHTS 1\n");
	s_pointlightShader[1] = new LmrShader("model-pointlit", "#define NUM_LIGHTS 2\n");
	s_pointlightShader[2] = new LmrShader("model-pointlit", "#define NUM_LIGHTS 3\n");
	s_pointlightShader[3] = new LmrShader("model-pointlit", "#define NUM_LIGHTS 4\n");

	PiVerify(s_font = s_fontManager.GetVectorFont("WorldFont"));

	lua_State *L = lua_open();
	sLua = L;
	luaL_openlibs(L);

	LUA_DEBUG_START(sLua);

	LuaConstants::Register(L);

	MyLuaVec::Vec_register(L);
	lua_pop(L, 1); // why again?
	MyLuaMatrix::Matrix_register(L);
	lua_pop(L, 1); // why again?
	// shorthand for Vec.new(x,y,z)
	lua_register(L, "v", MyLuaVec::Vec_new);
	lua_register(L, "norm", MyLuaVec::Vec_newNormalized);
	lua_register(L, "define_model", define_model);
	lua_register(L, "set_material", ModelFuncs::set_material);
	lua_register(L, "use_material", ModelFuncs::use_material);
	lua_register(L, "get_arg_material", ModelFuncs::get_arg_material);
	lua_register(L, "sphere", ModelFuncs::sphere);
	lua_register(L, "sphere_slice", ModelFuncs::sphere_slice);
	lua_register(L, "invisible_tri", ModelFuncs::invisible_tri);
	lua_register(L, "tri", ModelFuncs::tri);
	lua_register(L, "xref_tri", ModelFuncs::xref_tri);
	lua_register(L, "quad", ModelFuncs::quad);
	lua_register(L, "xref_quad", ModelFuncs::xref_quad);
	lua_register(L, "cylinder", ModelFuncs::cylinder);
	lua_register(L, "xref_cylinder", ModelFuncs::xref_cylinder);
	lua_register(L, "tapered_cylinder", ModelFuncs::tapered_cylinder);
	lua_register(L, "xref_tapered_cylinder", ModelFuncs::xref_tapered_cylinder);
	lua_register(L, "lathe", ModelFuncs::lathe);
	lua_register(L, "tube", ModelFuncs::tube);
	lua_register(L, "xref_tube", ModelFuncs::xref_tube);
	lua_register(L, "ring", ModelFuncs::ring);
	lua_register(L, "xref_ring", ModelFuncs::xref_ring);
	lua_register(L, "circle", ModelFuncs::circle);
	lua_register(L, "xref_circle", ModelFuncs::xref_circle);
	lua_register(L, "text", ModelFuncs::text);
	lua_register(L, "texture", ModelFuncs::texture);
	lua_register(L, "texture_glow", ModelFuncs::texture_glow);
	lua_register(L, "quadric_bezier_quad", ModelFuncs::quadric_bezier_quad);
	lua_register(L, "xref_quadric_bezier_quad", ModelFuncs::xref_quadric_bezier_quad);
	lua_register(L, "cubic_bezier_quad", ModelFuncs::cubic_bezier_quad);
	lua_register(L, "xref_cubic_bezier_quad", ModelFuncs::xref_cubic_bezier_quad);
	lua_register(L, "cubic_bezier_tri", ModelFuncs::cubic_bezier_triangle);
	lua_register(L, "xref_cubic_bezier_tri", ModelFuncs::xref_cubic_bezier_triangle);
	lua_register(L, "quadric_bezier_tri", ModelFuncs::quadric_bezier_triangle);
	lua_register(L, "xref_quadric_bezier_tri", ModelFuncs::xref_quadric_bezier_triangle);
	lua_register(L, "extrusion", ModelFuncs::extrusion);
	lua_register(L, "thruster", ModelFuncs::thruster);
	lua_register(L, "xref_thruster", ModelFuncs::xref_thruster);
	lua_register(L, "get_time", ModelFuncs::get_time);
	lua_register(L, "get_equipment", ModelFuncs::get_equipment);
	lua_register(L, "get_animation_stage", ModelFuncs::get_animation_stage);
	lua_register(L, "get_animation_position", ModelFuncs::get_animation_position);
	lua_register(L, "get_flight_state", ModelFuncs::get_flight_state);
	lua_register(L, "get_label", ModelFuncs::get_label);
	lua_register(L, "flat", ModelFuncs::flat);
	lua_register(L, "xref_flat", ModelFuncs::xref_flat);
	lua_register(L, "billboard", ModelFuncs::billboard);
	lua_register(L, "geomflag", ModelFuncs::geomflag);
	lua_register(L, "zbias", ModelFuncs::zbias);
	lua_register(L, "call_model", ModelFuncs::call_model);
	lua_register(L, "noise", UtilFuncs::noise);
	lua_register(L, "load_obj", ObjLoader::load_obj_file);
	lua_register(L, "load_lua", pi_load_lua);
	lua_register(L, "set_insideout", ModelFuncs::insideout);
	lua_register(L, "set_local_lighting", ModelFuncs::set_local_lighting);
	lua_register(L, "set_light", ModelFuncs::set_light);
	lua_register(L, "use_light", ModelFuncs::use_light);

	s_buildDynamic = false;
	lua_pushstring(L, PIONEER_DATA_DIR);
	lua_setglobal(L, "CurrentDirectory");
	
	// same as luaL_dofile, except we can pass an error handler
	lua_pushcfunction(L, pi_lua_panic);
	if (luaL_loadfile(L, (std::string(PIONEER_DATA_DIR) + "/pimodels.lua").c_str())) {
		pi_lua_panic(L);
	} else {
		lua_pcall(L, 0, LUA_MULTRET, -2);
	}
	lua_pop(sLua, 1);  // remove panic func

	LUA_DEBUG_END(sLua, 0);
	
	_write_model_crc_file();
	s_buildDynamic = true;
}


void LmrModelCompilerUninit()
{
	for (int i=0; i<4; i++) {
		delete s_sunlightShader[i];
		delete s_pointlightShader[i];
	}
	// FontManager should be ok...

	std::map<std::string, LmrModel*>::iterator it_model;
	for (it_model=s_models.begin(); it_model != s_models.end(); ++it_model)	{
		delete (*it_model).second;
	}
	
	lua_close(sLua); sLua = 0;

	delete s_staticBufferPool;
}
