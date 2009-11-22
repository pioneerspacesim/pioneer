#include "libs.h"
#include <map>
#include "glfreetype.h"

/*
 * TODO

	~ model parameters
done	material(dr, dg, db, sr, sg, sb, shininess, er, eg, eb)
done	triangle(vector v1, vector v2, vector v3)
done	quad(vector v1, vector v2, vector v3, vector v4)
done	circle(int circumference_steps, vector center, vector normal, vector up, radius=float radius)
done	cylinder(int steps, vector start, vector end, vector up, radius=float radius)
	tube(int steps, vector start, vector end, vector up, innerrad=float, outerrad=float)
	extrusion(vector start, vector end, vector up, radius=float, {v1, v2, v3, ... })
	smooth(int steps, 
	flat(int steps, vector normal,
done	text("some literal string", vector pos, vector norm, vector xaxis, [xoff=, yoff=, scale=, onflag=])
	subobject(object_name, vector pos, vector up, vector zaxis [, scale=float, onflag=])
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

static int average(lua_State *L)
{
	/* get number of arguments */
	int n = lua_gettop(L);
	double sum = 0;
	int i;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{
		/* total the arguments */
		sum += lua_tonumber(L, i);
	}

	/* push the average */
	lua_pushnumber(L, sum / n);

	/* push the sum */
	lua_pushnumber(L, sum);

	printf("Sum %f, average %f\n", sum, sum/n);

	/* return the number of results */
	return 2;
}

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
		double x = lua_tonumber(L, 1);
		double y = lua_tonumber(L, 2);
		double z = lua_tonumber(L, 3);
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
		double m;
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

	static int Vec_div (lua_State *L)
	{
		vector3f *v1 = checkVec(L, 1);
		double d = lua_tonumber(L, 2);
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

	static const luaL_reg Vec_methods[] = {
		{ "new", Vec_new },
		{ "print", Vec_print },
		{ "dot", Vec_dot },
		{ "cross", Vec_cross },
		{ "norm", Vec_norm },
		{ "len", Vec_len },
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

class NewModel;

static bool s_buildDynamic;
static FontFace *s_font;
static float NEWMODEL_ZBIAS = 0.0002f;
static NewModel *s_curModel;
static std::map<std::string, NewModel*> s_models;
static lua_State *sLua;

static void LuaModelRender(NewModel *m, const matrix4x4d &pos);

class NewModel {
public:
	NewModel(std::string name) {
		curOp.type = OP_NONE;
		m_name = name;
	}
	const std::string &GetName() const { return m_name; }
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
		if (!s_buildDynamic) {
			m_verticesEndStatic = m_vertices.size();
			m_indicesEndStatic = m_indices.size();
			m_opsEndStatic = m_ops.size();
			printf("%d vertices, %d indices, %d ops\n", m_vertices.size(), m_indices.size(), m_ops.size());
		}
		curOp.type = OP_NONE;
	}
	void FreeDynamicGeometry() {
		m_vertices.resize(m_verticesEndStatic);
		m_indices.resize(m_indicesEndStatic);
		m_ops.resize(m_opsEndStatic);
	}
	void Render(const vector3f &cameraPos) {
		glEnable(GL_LIGHTING);
		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 2*sizeof(vector3f), &m_vertices[0].n);
		glVertexPointer(3, GL_FLOAT, 2*sizeof(vector3f), &m_vertices[0].v);

		glDepthRange(0.0, 1.0);

		for (unsigned int i=0; i<m_ops.size(); i++) {
			const Op &op = m_ops[i];
			switch (op.type) {
			case OP_DRAW_ELEMENTS:
				//printf("Draw %d elems, start %d\n", op.elems.count, op.elems.start);
				glDrawElements(GL_TRIANGLES, op.elems.count, GL_UNSIGNED_SHORT, &m_indices[op.elems.start]);
				break;
			case OP_SET_MATERIAL:
				{
					const Material &m = m_materials[op.col.material_idx];
					glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, m.diffuse);
					glMaterialfv (GL_FRONT, GL_SPECULAR, m.specular);
					glMaterialfv (GL_FRONT, GL_EMISSION, m.emissive);
					glMaterialf (GL_FRONT, GL_SHININESS, m.shininess);
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
#warning only works after other geom...
				{
				s_curModel = op.callmodel.model;
				matrix4x4d m = matrix4x4d::Identity();
				LuaModelRender(op.callmodel.model, m);
				s_curModel = this;
				}
				break;
			case OP_NONE:
				break;
			}
		}
		
		glDisableClientState (GL_VERTEX_ARRAY);
		glDisableClientState (GL_NORMAL_ARRAY);
	}
	int PushVertex(const vector3f &pos, const vector3f &normal) {
		m_vertices.push_back(Vertex(pos, normal));
		return m_vertices.size() - 1;
	}
	void PushTri(int i1, int i2, int i3) {
		OpDrawElements(3);
		m_indices.push_back(i1);
		m_indices.push_back(i2);
		m_indices.push_back(i3);
	}
	
	void PushZBias(float amount, const vector3f &pos, const vector3f &norm) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_ZBIAS;
		curOp.zbias.amount = amount;
		memcpy(curOp.zbias.pos, &pos.x, 3*sizeof(float));
		memcpy(curOp.zbias.norm, &norm.x, 3*sizeof(float));
	}

	void PushRing(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius) {
		const int vtxStart = m_vertices.size();

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = vector3f::Cross(updir, dir).Normalized();

		m_vertices.resize(m_vertices.size() + 2*steps);

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			m_vertices[vtxStart+i] = Vertex(start+p, n);
			m_vertices[vtxStart+i+steps] = Vertex(end+p, n);
		}

		OpDrawElements(steps*2*3);
		for (int i=0; i<steps-1; i++) {
			PushIdx(vtxStart+i); PushIdx(vtxStart+i+1); PushIdx(vtxStart+i+steps);
			PushIdx(vtxStart+i+1); PushIdx(vtxStart+i+steps+1); PushIdx(vtxStart+i+steps);
		}
		PushIdx(vtxStart+steps-1); PushIdx(vtxStart); PushIdx(vtxStart+2*steps-1);
		PushIdx(vtxStart); PushIdx(vtxStart+steps); PushIdx(vtxStart+2*steps-1);
	}

	void PushCylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius) {
		const int vtxStart = m_vertices.size();

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = vector3f::Cross(updir, dir).Normalized();

		m_vertices.resize(m_vertices.size() + 4*steps);

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			m_vertices[vtxStart+i] = Vertex(start+p, n);
			m_vertices[vtxStart+i+steps] = Vertex(end+p, n);
			m_vertices[vtxStart+i+2*steps] = Vertex(start+p, -dir);
			m_vertices[vtxStart+i+3*steps] = Vertex(end+p, -dir);
		}

		OpDrawElements(steps*2*3);
		for (int i=0; i<steps-1; i++) {
			PushIdx(vtxStart+i); PushIdx(vtxStart+i+1); PushIdx(vtxStart+i+steps);
			PushIdx(vtxStart+i+1); PushIdx(vtxStart+i+steps+1); PushIdx(vtxStart+i+steps);
		}
		PushIdx(vtxStart+steps-1); PushIdx(vtxStart); PushIdx(vtxStart+2*steps-1);
		PushIdx(vtxStart); PushIdx(vtxStart+steps); PushIdx(vtxStart+2*steps-1);

		OpDrawElements((steps-2)*6);
		for (int i=2; i<steps; i++) {
			// bottom cap
			PushIdx(vtxStart+2*steps);
			PushIdx(vtxStart+2*steps+i);
			PushIdx(vtxStart+2*steps+i-1);
			// top cap
			PushIdx(vtxStart+3*steps);
			PushIdx(vtxStart+3*steps+i-1);
			PushIdx(vtxStart+3*steps+i);
		}
	}
	
	void PushCircle(int steps, const vector3f &center, const vector3f &normal, const vector3f &updir, float radius) {
		const int vtxStart = m_vertices.size();

		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = vector3f::Cross(updir, normal).Normalized();

		m_vertices.resize(m_vertices.size() + steps);

		float ang = 0.0;
		const float inc = 2.0f*M_PI / (float)steps;
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = center + radius * (sin(ang)*axis1 + cos(ang)*axis2);
			m_vertices[vtxStart+i] = Vertex(p, normal);
		}

		OpDrawElements((steps-2)*3);
		for (int i=2; i<steps; i++) {
			// top cap
			PushIdx(vtxStart);
			PushIdx(vtxStart+i-1);
			PushIdx(vtxStart+i);
		}
	}

	void PushCallModel(NewModel *m) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_CALL_MODEL;
		curOp.callmodel.model = m;
	}

	void DeclareMaterial(const char *mat_name) {
		assert(!s_buildDynamic);
		m_materialLookup[mat_name] = m_materials.size();
		m_materials.push_back(Material());
	}
	
	void SetMaterial(const char *mat_name, const float mat[10]) {
		std::map<std::string, int>::iterator i = m_materialLookup.find(mat_name);
		if (i != m_materialLookup.end()) {
			Material &m = m_materials[(*i).second];
			m.diffuse[0] = mat[0];
			m.diffuse[1] = mat[1];
			m.diffuse[2] = mat[2];
			m.diffuse[3] = 1.0f;
			m.specular[0] = mat[3];
			m.specular[1] = mat[4];
			m.specular[2] = mat[5];
			m.specular[3] = 1.0f;
			m.shininess = mat[6];
			m.emissive[0] = mat[7];
			m.emissive[1] = mat[8];
			m.emissive[2] = mat[9];
			m.emissive[3] = 1.0f;
		} else {
			printf("Unknown material name.");
			exit(0);
		}
	}

	void UseMaterial(const char *mat_name) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_SET_MATERIAL;
		
		std::map<std::string, int>::iterator i = m_materialLookup.find(mat_name);
		if (i != m_materialLookup.end()) {
			curOp.col.material_idx = (*i).second;
		} else {
			printf("Unknown material name.");
			exit(0);
		}
	}

	enum OpType { OP_NONE, OP_DRAW_ELEMENTS, OP_SET_MATERIAL, OP_ZBIAS,
			OP_CALL_MODEL };

	struct Op {
		enum OpType type;
		union {
			struct { int start, count; } elems;
			struct { int material_idx; } col;
			struct { float amount; float pos[3]; float norm[3]; } zbias;
			struct { NewModel *model; } callmodel;
		};
	};
private:
	void OpDrawElements(int numIndices) {
		if (curOp.type != OP_DRAW_ELEMENTS) {
			if (curOp.type) m_ops.push_back(curOp);
			curOp.type = OP_DRAW_ELEMENTS;
			curOp.elems.start = m_indices.size();
			curOp.elems.count = 0;
		}
		curOp.elems.count += numIndices;
	}
	void PushCurOp() {
		m_ops.push_back(curOp);
	}
	void PushIdx(Uint16 v) {
		m_indices.push_back(v);
	}

	struct Vertex {
		Vertex() {}
		Vertex(const vector3f &v, const vector3f &n): v(v), n(n) {}
		vector3f v, n;
	};

	struct Material {
		Material() {}
		float diffuse[4];
		float specular[4];
		float shininess;
		float emissive[4];
	};
	Op curOp;
	std::vector<Vertex> m_vertices;
	std::vector<Uint16> m_indices;
	std::vector<Op> m_ops;
	// index into m_materials
	std::map<std::string, int> m_materialLookup;
	std::vector<Material> m_materials;
	int m_verticesEndStatic;
	int m_indicesEndStatic;
	int m_opsEndStatic;
	std::string m_name;
};

static void LuaModelRender(NewModel *m, const matrix4x4d &pos)
{
	s_curModel = m;
	glPushMatrix();
	glMultMatrixd(&pos[0]);

	// call model dynamic bits
	char buf[256];
	snprintf(buf, sizeof(buf), "%s_dynamic", m->GetName().c_str());
	lua_getfield(sLua, LUA_GLOBALSINDEX, buf);
	lua_call(sLua, 0, 0);
	s_curModel->Build();

	const vector3f cam_pos(-pos[12], -pos[13], -pos[14]);
#warning cam_pos will be wrong for submodels
	s_curModel->Render(cam_pos);
	s_curModel->FreeDynamicGeometry();
	s_curModel = 0;
	glPopMatrix();
}

void LuaModelRender(const char *name, const matrix4x4d &pos)
{
	LuaModelRender(s_models[name], pos);
}

namespace ModelFuncs {
	static int callmodel(lua_State *L)
	{
		const char *obj_name = luaL_checkstring(L, 1);
//	subobject(object_name, vector pos, vector up, vector zaxis [, scale=float, onflag=])
		NewModel *m = s_models[obj_name];
		if (!m) {
			luaL_error(L, "callmodel() to undefined model. Referenced model must be registered before calling model");
		} else {
			s_curModel->PushCallModel(m);
		}
		return 0;
	}

	static int dec_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		s_curModel->DeclareMaterial(mat_name);
		return 0;
	}

	static int set_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		float mat[10];
		for (int i=0; i<10; i++) {
			mat[i] = lua_tonumber(L, i+2);
		}
		s_curModel->SetMaterial(mat_name, mat);
		return 0;
	}

	static int use_material(lua_State *L)
	{
		const char *mat_name = luaL_checkstring(L, 1);
		s_curModel->UseMaterial(mat_name);
		return 0;
	}

		static matrix4x4f _textTrans;
		static vector3f _textNorm;
		static void _text_index_callback(int num, Uint16 *vals) {
			const int base = s_curModel->GetVerticesPos();
			for (int i=0; i<num; i+=3) {
				s_curModel->PushTri(vals[i]+base, vals[i+1]+base, vals[i+2]+base);
			}
		}
		static void _text_vertex_callback(int num, float offsetX, float offsetY, float *vals) {
			for (int i=0; i<num*3; i+=3) {
				vector3f p = vector3f(offsetX+vals[i], offsetY+vals[i+1], vals[i+2]);
				p = _textTrans * p;
				s_curModel->PushVertex(p, _textNorm);
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
			s_curModel->PushZBias(0, vector3f(0.0), vector3f(0.0));
		} else {
			vector3f *pos = MyLuaVec::checkVec(L, 2);
			vector3f *norm = MyLuaVec::checkVec(L, 3);
			s_curModel->PushZBias(amount, *pos, *norm);
		}
		return 0;
	}

	static int circle(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		vector3f *center = MyLuaVec::checkVec(L, 2);
		vector3f *normal = MyLuaVec::checkVec(L, 3);
		vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		s_curModel->PushCircle(steps, *center, *normal, *updir, radius);
		return 0;
	}

	static int cylinder(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		vector3f *start = MyLuaVec::checkVec(L, 2);
		vector3f *end = MyLuaVec::checkVec(L, 3);
		vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		s_curModel->PushCylinder(steps, *start, *end, *updir, radius);
		return 0;
	}

	/* Cylinder with no top or bottom caps */
	static int ring(lua_State *L)
	{
		int steps = luaL_checkint(L, 1);
		vector3f *start = MyLuaVec::checkVec(L, 2);
		vector3f *end = MyLuaVec::checkVec(L, 3);
		vector3f *updir = MyLuaVec::checkVec(L, 4);
		float radius = lua_tonumber(L, 5);
		s_curModel->PushRing(steps, *start, *end, *updir, radius);
		return 0;
	}

	static int tri(lua_State *L)
	{
		vector3f *v1 = MyLuaVec::checkVec(L, 1);
		vector3f *v2 = MyLuaVec::checkVec(L, 2);
		vector3f *v3 = MyLuaVec::checkVec(L, 3);
		
		vector3f n = vector3f::Cross((*v1)-(*v2), (*v1)-(*v3)).Normalized();
		int i1 = s_curModel->PushVertex(*v1, n);
		int i2 = s_curModel->PushVertex(*v2, n);
		int i3 = s_curModel->PushVertex(*v3, n);
		s_curModel->PushTri(i1, i2, i3);
		return 0;
	}
	
	static int quad(lua_State *L)
	{
		vector3f *v1 = MyLuaVec::checkVec(L, 1);
		vector3f *v2 = MyLuaVec::checkVec(L, 2);
		vector3f *v3 = MyLuaVec::checkVec(L, 3);
		vector3f *v4 = MyLuaVec::checkVec(L, 4);
		
		vector3f n = vector3f::Cross((*v1)-(*v2), (*v1)-(*v3)).Normalized();
		int i1 = s_curModel->PushVertex(*v1, n);
		int i2 = s_curModel->PushVertex(*v2, n);
		int i3 = s_curModel->PushVertex(*v3, n);
		int i4 = s_curModel->PushVertex(*v4, n);
		s_curModel->PushTri(i1, i2, i3);
		s_curModel->PushTri(i1, i3, i4);
		return 0;
	}

} /* namespace ModelFuncs */

static int register_models(lua_State *L)
{
	int n = lua_gettop(L);

	for (int i = 1; i <= n; i++) {
		const char *model_name = luaL_checkstring(L, i);
		s_curModel = new NewModel(model_name);
		s_models[model_name] = s_curModel;
		printf("Model %s\n", model_name);
		char buf[256];
		snprintf(buf, sizeof(buf), "%s_static", model_name);
		// call model static building function
		lua_getfield(L, LUA_GLOBALSINDEX, buf);
		lua_call(L, 0, 0);
		s_curModel->Build();
	}
	return 0;
}

void LuaModelCompilerInit()
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
	lua_register(L, "tri", ModelFuncs::tri);
	lua_register(L, "quad", ModelFuncs::quad);
	lua_register(L, "dec_material", ModelFuncs::dec_material);
	lua_register(L, "set_material", ModelFuncs::set_material);
	lua_register(L, "use_material", ModelFuncs::use_material);
	lua_register(L, "cylinder", ModelFuncs::cylinder);
	lua_register(L, "ring", ModelFuncs::ring);
	lua_register(L, "circle", ModelFuncs::circle);
	lua_register(L, "text", ModelFuncs::text);
	lua_register(L, "zbias", ModelFuncs::zbias);
	lua_register(L, "callmodel", ModelFuncs::callmodel);

	s_buildDynamic = false;
	if (luaL_dofile(L, "models.lua")) {
		printf("%s\n", lua_tostring(L, -1));
	}
	s_buildDynamic = true;
	//lua_close(L);
}
