#include "libs.h"
#include <map>
#include "glfreetype.h"
#include "lua_model_compiler.h"
#include "collider/collider.h"
#include "perlin.h"

/*
 * TODO

	~ model parameters
done	material(dr, dg, db, sr, sg, sb, shininess, er, eg, eb)
done	triangle(vector v1, vector v2, vector v3)
done	quad(vector v1, vector v2, vector v3, vector v4)
done	circle(int circumference_steps, vector center, vector normal, vector up, radius=float radius)
done	cylinder(int steps, vector start, vector end, vector up, radius=float radius)
done	tube(int steps, vector start, vector end, vector up, innerrad=float, outerrad=float)
done	extrusion(vector start, vector end, vector up, radius, v1, v2, v3, ... )
~diff	smooth(int steps, 
	flat(int steps, vector normal,
done	text("some literal string", vector pos, vector norm, vector xaxis, [xoff=, yoff=, scale=, onflag=])
done	subobject(object_name, vector pos, vector xaxis, vector yaxis, scale=float)
	thruster(direction, vector position, float size)
	geomflag(int)
done	zbias(vector position, vector normal, int level)
done	nozbias()
*/

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#define VEC "Vec"
#define MODEL "Model"

namespace MyLuaVec {
	static vector3f *checkVec (lua_State *L, int index)
	{
		vector3f *v;
		luaL_checktype(L, index, LUA_TUSERDATA);
		v = (vector3f *)luaL_checkudata(L, index, VEC);
		if (v == NULL) luaL_typerror(L, index, VEC);
		return v;
	}


	static vector3f *pushVec(lua_State *L)
	{
		vector3f *v = (vector3f *)lua_newuserdata(L, sizeof(vector3f));
		luaL_getmetatable(L, VEC);
		lua_setmetatable(L, -2);
		return v;
	}


	static int Vec_new(lua_State *L)
	{
		float x = lua_tonumber(L, 1);
		float y = lua_tonumber(L, 2);
		float z = lua_tonumber(L, 3);
		vector3f *v = pushVec(L);
		v->x = x;
		v->y = y;
		v->z = z;
		return 1;
	}

	static int Vec_print(lua_State *L)
	{
		vector3f *v = checkVec(L, 1);
		printf("%f,%f,%f\n", v->x, v->y, v->z);
		return 0;
	}

	static int Vec_add (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		vector3f *v2 = checkVec(L, 2);
		vector3f *sum = pushVec(L);
		*sum = (*v1) + (*v2);
		return 1;
	}

	static int Vec_sub (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		vector3f *v2 = checkVec(L, 2);
		vector3f *sum = pushVec(L);
		*sum = (*v1) - (*v2);
		return 1;
	}

	static int Vec_cross (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		vector3f *v2 = checkVec(L, 2);
		vector3f *out = pushVec(L);
		*out = vector3f::Cross(*v1, *v2);
		return 1;
	}

	static int Vec_mul (lua_State *L)
	{
		vector3f *v;
		float m;
		if (lua_isnumber(L,1)) {
			m = lua_tonumber(L, 1);
			v = checkVec(L, 2);
		} else {
			v = checkVec(L, 1);
			m = lua_tonumber(L, 2);
		}
		vector3f *out = pushVec(L);
		*out = m * (*v);
		return 1;
	}

	static int Vec_index (lua_State *L)
	{
		printf("Fuckme\n");
		vector3f *v = checkVec(L, 1);
		unsigned int i = luaL_checkint(L, 2);
		if (i>i) {
			luaL_error(L, "vector index must be in range 0-2");
		}
		lua_pushnumber(L, (*v)[i]);
		return 1;
	}

	static int Vec_div (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		float d = lua_tonumber(L, 2);
		vector3f *out = pushVec(L);
		*out = (1.0/d) * (*v1);
		return 1;
	}

	static int Vec_norm (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		vector3f *out = pushVec(L);
		*out = (*v1).Normalized();
		return 1;
	}

	static int Vec_dot (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		vector3f *v2 = checkVec(L, 2);
		lua_pushnumber(L, vector3f::Dot(*v1, *v2));
		return 1;
	}

	static int Vec_len (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		lua_pushnumber(L, v1->Length());
		return 1;
	}

	static int Vec_getx (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		lua_pushnumber(L, v1->x);
		return 1;
	}

	static int Vec_gety (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		lua_pushnumber(L, v1->y);
		return 1;
	}

	static int Vec_getz (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		lua_pushnumber(L, v1->z);
		return 1;
	}

	static const luaL_reg Vec_methods[] = {
		{ "new", Vec_new },
		{ "print", Vec_print },
		{ "dot", Vec_dot },
		{ "cross", Vec_cross },
		{ "norm", Vec_norm },
		{ "len", Vec_len },
		{ "x",      Vec_getx},
		{ "y",      Vec_gety},
		{ "z",      Vec_getz},
		{ 0, 0 }
	};

	static const luaL_reg Vec_meta[] = {
		//  {"__gc",       Foo_gc},
		//  {"__tostring", Foo_tostring},
		{"__add",      Vec_add},
		{"__sub",      Vec_sub},
		{"__mul",      Vec_mul},
		{"__div",      Vec_div},
		{0, 0}
	};

	int Vec_register (lua_State *L)
	{
		luaL_openlib(L, VEC, Vec_methods, 0);  /* create methods table,
						    add it to the globals */
		luaL_newmetatable(L, VEC);          /* create metatable for Vec,
						 and add it to the Lua registry */
		luaL_openlib(L, 0, Vec_meta, 0);    /* fill metatable */
		lua_pushliteral(L, "__index");
		lua_pushvalue(L, -3);               /* dup methods table*/
		lua_rawset(L, -3);                  /* metatable.__index = methods */
		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, -3);               /* dup methods table*/
		lua_rawset(L, -3);                  /* hide metatable:
						 metatable.__metatable = methods */
		lua_pop(L, 1);                      /* drop metatable */
		return 1;                           /* return methods on the stack */
	}
} /* namespace MyLuaVec */

class LmrGeomBuffer;


static bool s_buildDynamic;
static FontFace *s_font;
static float NEWMODEL_ZBIAS = 0.0002f;
static LmrGeomBuffer *s_curBuf;
static std::map<std::string, LmrModel*> s_models;
static lua_State *sLua;
static int s_numTrisRendered;

static void LmrModelRender(LmrModel *m, const vector3f &cameraPos, const matrix4x4f &pos);

int LmrModelGetStatsTris() { return s_numTrisRendered; }
void LmrModelClearStatsTris() { s_numTrisRendered = 0; }

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
//#define USE_VBO 0
	
class LmrGeomBuffer {
public:
	LmrGeomBuffer(LmrModel *model) {
		curOp.type = OP_NONE;
		m_vertexBuffer = 0;
		m_indexBuffer = 0;
		m_model = model;
	}
	~LmrGeomBuffer() {
		if (m_vertexBuffer) glDeleteBuffersARB(1, &m_vertexBuffer);
		if (m_indexBuffer) glDeleteBuffersARB(1, &m_indexBuffer);
	}
	int GetIndicesPos() const {
		return m_indices.size();
	}
	int GetVerticesPos() const {
		return m_vertices.size();
	}
	void Build() {
		PushCurOp();
		// we are using Uint16 index arrays
		assert(m_indices.size() < 65536);
	//		printf("%d vertices, %d indices, %d ops\n", m_vertices.size(), m_indices.size(), m_ops.size());
		if (USE_VBO) {
			if (m_vertexBuffer == 0) glGenBuffersARB(1, &m_vertexBuffer);
			if (m_indexBuffer == 0) glGenBuffersARB(1, &m_indexBuffer);
			
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint16)*m_indices.size(),
					0, GL_STATIC_DRAW);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint16)*m_indices.size(),
					&m_indices[0], GL_STATIC_DRAW);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
		
			glBindBufferARB(GL_ARRAY_BUFFER, m_vertexBuffer);
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(Vertex)*m_vertices.size(), 0, GL_STATIC_DRAW);
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(Vertex)*m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);
			glBindBufferARB(GL_ARRAY_BUFFER, 0);
		}
		curOp.type = OP_NONE;
	}
	void FreeGeometry() {
		m_vertices.clear();
		m_indices.clear();
		m_ops.clear();
	}
	void Render(const vector3f &cameraPos) {
		s_numTrisRendered += m_indices.size()/3;
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
		glEnable(GL_LIGHTING);

		BindBuffers();

		glDepthRange(0.0, 1.0);

		const unsigned int opEndIdx = m_ops.size();
		for (unsigned int i=0; i<opEndIdx; i++) {
			const Op &op = m_ops[i];
			switch (op.type) {
			case OP_DRAW_ELEMENTS:
				if (USE_VBO) {
					glDrawRangeElements(GL_TRIANGLES, op.elems.elemMin, op.elems.elemMax, op.elems.count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(op.elems.start*sizeof(Uint16)));
				} else {
					glDrawElements(GL_TRIANGLES, op.elems.count, GL_UNSIGNED_SHORT, &m_indices[op.elems.start]);
				}
				break;
			case OP_SET_MATERIAL:
				{
					const LmrModel::Material &m = m_model->m_materials[op.col.material_idx];
					glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, m.diffuse);
					glMaterialfv (GL_FRONT, GL_SPECULAR, m.specular);
					glMaterialfv (GL_FRONT, GL_EMISSION, m.emissive);
					glMaterialf (GL_FRONT, GL_SHININESS, m.shininess);
					if (m.diffuse[3] == 1.0) {
						glDisable(GL_BLEND);
						glDepthMask(GL_TRUE);
					} else {
						glEnable(GL_BLEND);
						glDepthMask(GL_FALSE);
					}
				}
				break;
			case OP_ZBIAS:
				if (op.zbias.amount == 0) {
					glDepthRange(0.0, 1.0);
				} else {
					vector3f tv = cameraPos - vector3f(op.zbias.pos);
					if (vector3f::Dot(tv, vector3f(op.zbias.norm)) > 0.0f) {
						glDepthRange(0.0, 1.0 - op.zbias.amount*NEWMODEL_ZBIAS);
					}
				}
				break;
			case OP_CALL_MODEL:
#warning materials will be messed up after this...
				{
				const matrix4x4f trans = matrix4x4f(op.callmodel.transform);
				vector3f cam_pos = cameraPos - vector3f(trans[12], trans[13], trans[14]);
				op.callmodel.model->Render(cam_pos, trans);
#warning re-binding buffers may be unnecessary if no geometry is to follow
				BindBuffers();
				}
				break;
			case OP_NONE:
				break;
			}
		}
		
		glDisableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_NORMAL_ARRAY);

		if (USE_VBO) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
		}
	}
	int PushVertex(const vector3f &pos, const vector3f &normal) {
		m_vertices.push_back(Vertex(pos, normal));
		return m_vertices.size() - 1;
	}
	void SetVertex(int idx, const vector3f &pos, const vector3f &normal) {
		m_vertices[idx] = Vertex(pos, normal);
	}
	void PushTri(int i1, int i2, int i3) {
		OpDrawElements(3);
		PushIdx(i1);
		PushIdx(i2);
		PushIdx(i3);
	}
	
	void PushZBias(float amount, const vector3f &pos, const vector3f &norm) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_ZBIAS;
		curOp.zbias.amount = amount;
		memcpy(curOp.zbias.pos, &pos.x, 3*sizeof(float));
		memcpy(curOp.zbias.norm, &norm.x, 3*sizeof(float));
	}

	void PushCallModel(LmrModel *m, const matrix4x4f &transform) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_CALL_MODEL;
		memcpy(curOp.callmodel.transform, &transform[0], 16*sizeof(float));
		curOp.callmodel.model = m;
	}

	void DeclareMaterial(const char *mat_name) {
		assert(!s_buildDynamic);
		m_model->m_materialLookup[mat_name] = m_model->m_materials.size();
		m_model->m_materials.push_back(LmrModel::Material());
	}
	
	void SetMaterial(const char *mat_name, const float mat[11]) {
		std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
		if (i != m_model->m_materialLookup.end()) {
			LmrModel::Material &m = m_model->m_materials[(*i).second];
			m.diffuse[0] = mat[0];
			m.diffuse[1] = mat[1];
			m.diffuse[2] = mat[2];
			m.diffuse[3] = mat[3];
			m.specular[0] = mat[4];
			m.specular[1] = mat[5];
			m.specular[2] = mat[6];
			m.specular[3] = 1.0f;
			m.shininess = mat[7];
			m.emissive[0] = mat[8];
			m.emissive[1] = mat[9];
			m.emissive[2] = mat[10];
			m.emissive[3] = 1.0f;
		} else {
			luaL_error(sLua, "Unknown material name.");
			exit(0);
		}
	}

	void PushUseMaterial(const char *mat_name) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_SET_MATERIAL;
		
		std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
		if (i != m_model->m_materialLookup.end()) {
			curOp.col.material_idx = (*i).second;
		} else {
			printf("Unknown material name.");
			exit(0);
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

	void GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform) {
		const int vtxBase = c->nv;
		const int idxBase = c->ni;
		const int flagBase = c->nf;
		c->nv += m_vertices.size();
		c->ni += m_indices.size();
		c->nf += m_indices.size()/3;
		c->m_numTris += m_indices.size()/3;

		if (m_vertices.size()) {
			c->pVertex = (float*)realloc(c->pVertex, 3*sizeof(float)*c->nv);
		
			for (unsigned int i=0; i<m_vertices.size(); i++) {
				const vector3f v = transform * m_vertices[i].v;
				c->pVertex[3*vtxBase + 3*i] = v.x;
				c->pVertex[3*vtxBase + 3*i+1] = v.y;
				c->pVertex[3*vtxBase + 3*i+2] = v.z;
				c->m_aabb.Update(v);
			}
		}
		if (m_indices.size()) {
			c->pIndex = (int*)realloc(c->pIndex, sizeof(int)*c->ni);
			c->pFlag = (int*)realloc(c->pFlag, sizeof(int)*c->nf);
			for (unsigned int i=0; i<m_indices.size(); i++) {
				c->pIndex[idxBase + i] = vtxBase + m_indices[i];
			}
			for (int i=0; i<m_indices.size()/3; i++) {
				c->pFlag[flagBase + i] = 0;
			}
		}
		
		// go through Ops to see if we call other models
		const unsigned int opEndIdx = m_ops.size();
		for (unsigned int i=0; i<opEndIdx; i++) {
			const Op &op = m_ops[i];
			if (op.type == OP_CALL_MODEL) {
				matrix4x4f _trans = transform * matrix4x4f(op.callmodel.transform);
				op.callmodel.model->GetCollMeshGeometry(c, _trans);
			}
		}
	}

private:
	void BindBuffers() {
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_NORMAL_ARRAY);
		if (USE_VBO) {
			glBindBufferARB(GL_ARRAY_BUFFER, m_vertexBuffer);
			glVertexPointer(3, GL_FLOAT, sizeof(Vertex), 0);
			glNormalPointer(GL_FLOAT, sizeof(Vertex), (void *)(3*sizeof(float)));
			glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		} else {
			glNormalPointer(GL_FLOAT, 2*sizeof(vector3f), &m_vertices[0].n);
			glVertexPointer(3, GL_FLOAT, 2*sizeof(vector3f), &m_vertices[0].v);
		}
	}
	void OpDrawElements(int numIndices) {
		if (curOp.type != OP_DRAW_ELEMENTS) {
			if (curOp.type) m_ops.push_back(curOp);
			curOp.type = OP_DRAW_ELEMENTS;
			curOp.elems.start = m_indices.size();
			curOp.elems.count = 0;
			curOp.elems.elemMin = 1<<30;
			curOp.elems.elemMax = 0;
		}
		curOp.elems.count += numIndices;
	}
	void PushCurOp() {
		m_ops.push_back(curOp);
	}
	void PushIdx(Uint16 v) {
		curOp.elems.elemMin = MIN(v, curOp.elems.elemMin);
		curOp.elems.elemMax = MAX(v, curOp.elems.elemMax);
		m_indices.push_back(v);
	}

	enum OpType { OP_NONE, OP_DRAW_ELEMENTS, OP_SET_MATERIAL, OP_ZBIAS,
			OP_CALL_MODEL };

	struct Op {
		enum OpType type;
		union {
			struct { int start, count, elemMin, elemMax; } elems;
			struct { int material_idx; } col;
			struct { float amount; float pos[3]; float norm[3]; } zbias;
			struct { LmrModel *model; float transform[16]; } callmodel;
		};
	};
	struct Vertex {
		Vertex() {}
		Vertex(const vector3f &v, const vector3f &n): v(v), n(n) {}
		vector3f v, n;
	};

	Op curOp;
	std::vector<Vertex> m_vertices;
	std::vector<Uint16> m_indices;
	std::vector<Op> m_ops;
	GLuint m_vertexBuffer;
	GLuint m_indexBuffer;
	LmrModel *m_model;
};

LmrModel::LmrModel(const char *model_name)
{
	m_name = model_name;

	char buf[256];
	snprintf(buf, sizeof(buf), "%s_dynamic", model_name);
	lua_getglobal(sLua, buf);
	m_hasDynamicFunc = lua_isfunction(sLua, -1);

	m_staticGeometry = new LmrGeomBuffer(this);
	m_dynamicGeometry = new LmrGeomBuffer(this);

	s_curBuf = m_staticGeometry;
	// call model static building function
	lua_getfield(sLua, LUA_GLOBALSINDEX, (m_name+"_static").c_str());
	lua_call(sLua, 0, 0);
	s_curBuf = 0;
	m_staticGeometry->Build();
}

LmrModel::~LmrModel()
{
	delete m_staticGeometry;
	delete m_dynamicGeometry;
}

void LmrModel::Render(const matrix4x4f &trans)
{
	Render(vector3f(-trans[12], -trans[13], -trans[14]), trans);
}

void LmrModel::Render(const vector3f &cameraPos, const matrix4x4f &trans)
{
	glPushMatrix();
	glMultMatrixf(&trans[0]);

	Build();

	m_staticGeometry->Render(cameraPos);
	if (m_hasDynamicFunc) {
		m_dynamicGeometry->Render(cameraPos);
	}
	s_curBuf = 0;
	glPopMatrix();
}

void LmrModel::Build()
{
	if (m_hasDynamicFunc) {
		s_curBuf = m_dynamicGeometry;
		s_curBuf->FreeGeometry();
		// call model dynamic bits
		lua_getfield(sLua, LUA_GLOBALSINDEX, (m_name+"_dynamic").c_str());
		lua_call(sLua, 0, 0);
		s_curBuf = 0;
		m_dynamicGeometry->Build();
	}
}

void LmrModel::GetCollMeshGeometry(LmrCollMesh *mesh, const matrix4x4f &transform)
{
	Build();
	m_staticGeometry->GetCollMeshGeometry(mesh, transform);
	if (m_hasDynamicFunc) m_dynamicGeometry->GetCollMeshGeometry(mesh, transform);
}

LmrCollMesh::LmrCollMesh(LmrModel *m)
{
	memset(this, 0, sizeof(LmrCollMesh));
	m_aabb.min = vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
	m_aabb.max = vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	m->GetCollMeshGeometry(this, matrix4x4f::Identity());
	m_radius = MAX(m_aabb.min.Length(), m_aabb.max.Length());
	geomTree = new GeomTree(nv, m_numTris, pVertex, pIndex, pFlag);
}


LmrModel *LmrLookupModelByName(const char *name) throw (LmrModelNotFoundException)
{
	std::map<std::string, LmrModel*>::iterator i = s_models.find(name);

	if (i == s_models.end()) {
		throw LmrModelNotFoundException();
	}
	return (*i).second;
}	

namespace ModelFuncs {
	static int callmodel(lua_State *L)
	{
		const char *obj_name = luaL_checkstring(L, 1);
//	subobject(object_name, vector pos, vector xaxis, vector yaxis [, scale=float, onflag=])
		LmrModel *m = s_models[obj_name];
		if (!m) {
			luaL_error(L, "callmodel() to undefined model. Referenced model must be registered before calling model");
		} else {
			vector3f *pos = MyLuaVec::checkVec(L, 2);
			vector3f *_xaxis = MyLuaVec::checkVec(L, 3);
			vector3f *_yaxis = MyLuaVec::checkVec(L, 4);
			float scale = luaL_checknumber(L, 5);

			vector3f zaxis = vector3f::Cross(*_xaxis, *_yaxis).Normalized();
			vector3f xaxis = vector3f::Cross(*_yaxis, zaxis).Normalized();
			vector3f yaxis = vector3f::Cross(zaxis, xaxis);

			matrix4x4f trans = matrix4x4f::MakeRotMatrix(scale*xaxis, scale*yaxis, scale*zaxis);
			trans[12] = pos->x;
			trans[13] = pos->y;
			trans[14] = pos->z;

			s_curBuf->PushCallModel(m, trans);
		}
		return 0;
	}

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
		xax = vector3f::Cross(yax, zax);

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
			const vector3f norm = vector3f::Cross(v2-v1, v3-v1).Normalized();

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

	static vector3f eval_quadric_bezier3d(const vector3f p[9], float u, float v)
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

	static void _quadric_bezier(lua_State *L, bool xref)
	{
		vector3f pts[9];
		const int divs = luaL_checkint(L, 1);
		for (int i=0; i<9; i++) {
			pts[i] = *MyLuaVec::checkVec(L, i+2);
		}

		const int numVertsInPatch = (divs+1)*(divs+1);
		const int vtxStart = s_curBuf->AllocVertices(numVertsInPatch * (xref ? 2 : 1));

		float inc = 1.0f / (float)divs;
		float u,v;
		u = v = 0;
		for (int i=0; i<=divs; i++, u += inc) {
			v = 0;
			for (int j=0; j<=divs; j++, v += inc) {
				vector3f p = eval_quadric_bezier3d(pts, u, v);
				// this is a very inefficient way of
				// calculating normals...
				vector3f pu = eval_quadric_bezier3d(pts, u+0.5f, v);
				vector3f pv = eval_quadric_bezier3d(pts, u, v+0.5f);
				vector3f norm = vector3f::Cross(pu-p, pv-p).Normalized();

				s_curBuf->SetVertex(vtxStart + i + j*(divs+1), p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					s_curBuf->SetVertex(vtxStart + numVertsInPatch + i + j*(divs+1), p, norm);
				}
			}
		}

		for (int i=0; i<divs; i++) {
			int baseVtx = vtxStart + (divs+1)*i;
			for (int j=0; j<divs; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs+1));
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs+1), baseVtx+j+(divs+1));
			}
		}
		if (xref) for (int i=0; i<divs; i++) {
			int baseVtx = vtxStart + numVertsInPatch + (divs+1)*i;
			for (int j=0; j<divs; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs+1), baseVtx+j+1);
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+(divs+1), baseVtx+j+1+(divs+1));
			}
		}
	}
	
	static int quadric_bezier(lua_State *L) { _quadric_bezier(L, false); return 0; }
	static int xref_quadric_bezier(lua_State *L) { _quadric_bezier(L, true); return 0; }

	static vector3f eval_cubic_bezier3d(const vector3f p[16], float u, float v)
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

	static void _cubic_bezier(lua_State *L, bool xref)
	{
		vector3f pts[16];
		const int divs = luaL_checkint(L, 1);
		for (int i=0; i<16; i++) {
			pts[i] = *MyLuaVec::checkVec(L, i+2);
		}

		const int numVertsInPatch = (divs+1)*(divs+1);
		const int vtxStart = s_curBuf->AllocVertices(numVertsInPatch * (xref ? 2 : 1));


		float inc = 1.0f / (float)divs;
		float u,v;
		u = v = 0;
		for (int i=0; i<=divs; i++, u += inc) {
			v = 0;
			for (int j=0; j<=divs; j++, v += inc) {
				vector3f p = eval_cubic_bezier3d(pts, u, v);
				// this is a very inefficient way of
				// calculating normals...
				vector3f pu = eval_cubic_bezier3d(pts, u+0.5f, v);
				vector3f pv = eval_cubic_bezier3d(pts, u, v+0.5f);
				vector3f norm = vector3f::Cross(pu-p, pv-p).Normalized();

				s_curBuf->SetVertex(vtxStart + i + j*(divs+1), p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					s_curBuf->SetVertex(vtxStart + numVertsInPatch + i + j*(divs+1), p, norm);
				}
			}
		}

		for (int i=0; i<divs; i++) {
			int baseVtx = vtxStart + (divs+1)*i;
			for (int j=0; j<divs; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs+1));
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs+1), baseVtx+j+(divs+1));
			}
		}
		if (xref) for (int i=0; i<divs; i++) {
			int baseVtx = vtxStart + numVertsInPatch + (divs+1)*i;
			for (int j=0; j<divs; j++) {
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+1+(divs+1), baseVtx+j+1);
				s_curBuf->PushTri(baseVtx+j, baseVtx+j+(divs+1), baseVtx+j+1+(divs+1));
			}
		}
	}

	static int cubic_bezier(lua_State *L) { _cubic_bezier(L, false); return 0; }
	static int xref_cubic_bezier(lua_State *L) { _cubic_bezier(L, true); return 0; }

	static int dec_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		s_curBuf->DeclareMaterial(mat_name);
		return 0;
	}

	static int set_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		float mat[11];
		for (int i=0; i<11; i++) {
			mat[i] = lua_tonumber(L, i+2);
		}
		s_curBuf->SetMaterial(mat_name, mat);
		return 0;
	}

	static int use_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		s_curBuf->PushUseMaterial(mat_name);
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
	static int text(lua_State *L)
	{
		const char *str = luaL_checkstring(L, 1);
		vector3f *pos = MyLuaVec::checkVec(L, 2);
		vector3f *norm = MyLuaVec::checkVec(L, 3);
		vector3f *textdir = MyLuaVec::checkVec(L, 4);
		float scale = luaL_checknumber(L, 5);
		vector3f yaxis = vector3f::Cross(*norm, *textdir).Normalized();
		vector3f zaxis = vector3f::Cross(*textdir, yaxis).Normalized();
		vector3f xaxis = vector3f::Cross(yaxis, zaxis);

		_textTrans = matrix4x4f::MakeRotMatrix(scale*xaxis, scale*yaxis, scale*zaxis);
		_textTrans[12] = pos->x;
		_textTrans[13] = pos->y;
		_textTrans[14] = pos->z;
		_textNorm = *norm;
		s_font->GetStringGeometry(str, &_text_index_callback, &_text_vertex_callback);
//text("some literal string", vector pos, vector norm, vector textdir, [xoff=, yoff=, scale=, onflag=])
		return 0;
	}

	static int zbias(lua_State *L)
	{
		float amount = luaL_checknumber(L, 1);
		if (amount == 0) {
			s_curBuf->PushZBias(0, vector3f(0.0), vector3f(0.0));
		} else {
			vector3f *pos = MyLuaVec::checkVec(L, 2);
			vector3f *norm = MyLuaVec::checkVec(L, 3);
			s_curBuf->PushZBias(amount, *pos, *norm);
		}
		return 0;
	}

	static void _circle(int steps, const vector3f &center, const vector3f &normal, const vector3f &updir, float radius) {
		const int vtxStart = s_curBuf->AllocVertices(steps);

		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = vector3f::Cross(updir, normal).Normalized();

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = center + radius * (sin(ang)*axis1 + cos(ang)*axis2);
			s_curBuf->SetVertex(vtxStart+i, p, normal);
		}

		for (int i=2; i<steps; i++) {
			// top cap
			s_curBuf->PushTri(vtxStart, vtxStart+i-1, vtxStart+i);
		}
	}

	static int circle(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		const vector3f *center = MyLuaVec::checkVec(L, 2);
		const vector3f *normal = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_circle(steps, *center, *normal, *updir, radius);
		return 0;
	}

	static int xref_circle(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
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
		const vector3f axis2 = vector3f::Cross(updir, dir).Normalized();

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f p_inner = inner_radius * p;
			vector3f p_outer = outer_radius * p;

			s_curBuf->SetVertex(vtxStart+i, start+p_outer, p);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p_outer, p);
			s_curBuf->SetVertex(vtxStart+i+2*steps, start+p_inner, p);
			s_curBuf->SetVertex(vtxStart+i+3*steps, end+p_inner, p);

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
	
	static int tube(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float inner_radius = lua_tonumber(L, 5);
		float outer_radius = lua_tonumber(L, 6);
		_tube(steps, *start, *end, *updir, inner_radius, outer_radius);
		return 0;
	}

	static int xref_tube(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
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

	static void _cylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius) {
		const int vtxStart = s_curBuf->AllocVertices(4*steps);

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = vector3f::Cross(updir, dir).Normalized();

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			s_curBuf->SetVertex(vtxStart+i, start+p, n);
			s_curBuf->SetVertex(vtxStart+i+steps, end+p, n);
			s_curBuf->SetVertex(vtxStart+i+2*steps, start+p, -dir);
			s_curBuf->SetVertex(vtxStart+i+3*steps, end+p, -dir);
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
	
	static int cylinder(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_cylinder(steps, *start, *end, *updir, radius);
		return 0;
	}

	static int xref_cylinder(lua_State *L)
	{
		/* could optimise for x-reflection but fuck it */
		int steps = luaL_checkint(L, 1);
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
		const vector3f axis2 = vector3f::Cross(updir, dir).Normalized();

		const int vtxStart = s_curBuf->AllocVertices(2*steps);

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
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

	/* Cylinder with no top or bottom caps */
	static int ring(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		const vector3f *start = MyLuaVec::checkVec(L, 2);
		const vector3f *end = MyLuaVec::checkVec(L, 3);
		const vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		_ring(steps, *start, *end, *updir, radius);
		return 0;
	}

	static int xref_ring(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
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

	static int tri(lua_State *L)
	{
		const vector3f *v1 = MyLuaVec::checkVec(L, 1);
		const vector3f *v2 = MyLuaVec::checkVec(L, 2);
		const vector3f *v3 = MyLuaVec::checkVec(L, 3);
		
		vector3f n = vector3f::Cross((*v1)-(*v2), (*v1)-(*v3)).Normalized();
		int i1 = s_curBuf->PushVertex(*v1, n);
		int i2 = s_curBuf->PushVertex(*v2, n);
		int i3 = s_curBuf->PushVertex(*v3, n);
		s_curBuf->PushTri(i1, i2, i3);
		return 0;
	}
	
	static int xref_tri(lua_State *L)
	{
		vector3f v1 = *MyLuaVec::checkVec(L, 1);
		vector3f v2 = *MyLuaVec::checkVec(L, 2);
		vector3f v3 = *MyLuaVec::checkVec(L, 3);
		
		vector3f n = vector3f::Cross((v1)-(v2), (v1)-(v3)).Normalized();
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
	
	static int quad(lua_State *L)
	{
		const vector3f *v1 = MyLuaVec::checkVec(L, 1);
		const vector3f *v2 = MyLuaVec::checkVec(L, 2);
		const vector3f *v3 = MyLuaVec::checkVec(L, 3);
		const vector3f *v4 = MyLuaVec::checkVec(L, 4);
		
		vector3f n = vector3f::Cross((*v1)-(*v2), (*v1)-(*v3)).Normalized();
		int i1 = s_curBuf->PushVertex(*v1, n);
		int i2 = s_curBuf->PushVertex(*v2, n);
		int i3 = s_curBuf->PushVertex(*v3, n);
		int i4 = s_curBuf->PushVertex(*v4, n);
		s_curBuf->PushTri(i1, i2, i3);
		s_curBuf->PushTri(i1, i3, i4);
		return 0;
	}
	
	static int xref_quad(lua_State *L)
	{
		vector3f v1 = *MyLuaVec::checkVec(L, 1);
		vector3f v2 = *MyLuaVec::checkVec(L, 2);
		vector3f v3 = *MyLuaVec::checkVec(L, 3);
		vector3f v4 = *MyLuaVec::checkVec(L, 4);
		
		vector3f n = vector3f::Cross((v1)-(v2), (v1)-(v3)).Normalized();
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

} /* namespace ModelFuncs */

namespace UtilFuncs {
	
	int noise(lua_State *L) {
		vector3f v;
		if (lua_isnumber(L, 1)) {
			v.x = lua_tonumber(L, 1);
			v.y = lua_tonumber(L, 2);
			v.z = lua_tonumber(L, 3);
		} else {
			v = *MyLuaVec::checkVec(L, 1);
		}
		lua_pushnumber(L, noise(v));
		return 1;
	}
} /* UtilFuncs */

static int register_models(lua_State *L)
{
	int n = lua_gettop(L);

	for (int i = 1; i <= n; i++) {
		const char *model_name = luaL_checkstring(L, i);
		s_models[model_name] = new LmrModel(model_name);
	}
	return 0;
}

void LmrModelCompilerInit()
{
	assert(s_font = new FontFace ("font.ttf"));

	lua_State *L = lua_open();
	sLua = L;
	luaL_openlibs(L);

	MyLuaVec::Vec_register(L);
	lua_pop(L, 1); // why again?
	// shorthand for Vec.new(x,y,z)
	lua_register(L, "v", MyLuaVec::Vec_new);
	lua_register(L, "register_models", register_models);
	lua_register(L, "dec_material", ModelFuncs::dec_material);
	lua_register(L, "set_material", ModelFuncs::set_material);
	lua_register(L, "use_material", ModelFuncs::use_material);
	
	lua_register(L, "tri", ModelFuncs::tri);
	lua_register(L, "xref_tri", ModelFuncs::xref_tri);
	lua_register(L, "quad", ModelFuncs::quad);
	lua_register(L, "xref_quad", ModelFuncs::xref_quad);
	lua_register(L, "cylinder", ModelFuncs::cylinder);
	lua_register(L, "xref_cylinder", ModelFuncs::xref_cylinder);
	lua_register(L, "tube", ModelFuncs::tube);
	lua_register(L, "xref_tube", ModelFuncs::xref_tube);
	lua_register(L, "ring", ModelFuncs::ring);
	lua_register(L, "xref_ring", ModelFuncs::xref_ring);
	lua_register(L, "circle", ModelFuncs::circle);
	lua_register(L, "xref_circle", ModelFuncs::xref_circle);
	lua_register(L, "text", ModelFuncs::text);
	lua_register(L, "bezier_3x3", ModelFuncs::quadric_bezier);
	lua_register(L, "xref_bezier_3x3", ModelFuncs::xref_quadric_bezier);
	lua_register(L, "bezier_4x4", ModelFuncs::cubic_bezier);
	lua_register(L, "xref_bezier_4x4", ModelFuncs::xref_cubic_bezier);
	lua_register(L, "extrusion", ModelFuncs::extrusion);
	
	lua_register(L, "zbias", ModelFuncs::zbias);
	lua_register(L, "callmodel", ModelFuncs::callmodel);
	lua_register(L, "noise", UtilFuncs::noise);

	s_buildDynamic = false;
	if (luaL_dofile(L, "models.lua")) {
		printf("%s\n", lua_tostring(L, -1));
	}
	s_buildDynamic = true;
	//lua_close(L);
}
