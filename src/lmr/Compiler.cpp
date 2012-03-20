#include "Compiler.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "VectorFont.h"
#include "FileSystem.h"
#include "EquipType.h"
#include "EquipSet.h"
#include "perlin.h"

#include "LmrModel.h"
#include "GeomBuffer.h"

namespace LMR {

static std::map<std::string, LmrModel*> s_models;
static VectorFont *s_font;

static GeomBuffer *GetCurrentGeomBuffer(lua_State *l)
{
	lua_getfield(l, LUA_REGISTRYINDEX, "PiLmrCurrentBuffer");
	GeomBuffer *geomBuffer = reinterpret_cast<GeomBuffer*>(lua_touserdata(l, -1));
	lua_pop(l, 1);
	assert(geomBuffer);
	return geomBuffer;
}

static const LmrObjParams *GetCurrentObjParams(lua_State *l)
{
	lua_getfield(l, LUA_REGISTRYINDEX, "PiLmrCurrentParams");
	const LmrObjParams *objParams = reinterpret_cast<const LmrObjParams*>(lua_touserdata(l, -1));
	lua_pop(l, 1);
	assert(objParams);
	return objParams;
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

			GetCurrentGeomBuffer(L)->PushCallModel(m, trans, scale);
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
		GetCurrentGeomBuffer(L)->SetLight(num, quadratic_attenuation, *pos, *col);
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
		GetCurrentGeomBuffer(L)->PushUseLight(num);
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
		GetCurrentGeomBuffer(L)->PushSetLocalLighting(doIt);
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
		GetCurrentGeomBuffer(L)->SetInsideOut(doIt);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int vtxStart = geomBuffer->AllocVertices(steps*(num-2));

		const vector3f dir = (*end-*start).Normalized();
		const vector3f axis1 = updir->Normalized();
		const vector3f axis2 = updir->Cross(dir).Normalized();
		const float inc = 2.0f*M_PI / float(steps);

		for (int i=0; i<num-3; i+=2) {
			const float rad1 = jizz[i+1];
			const float rad2 = jizz[i+3];
			const vector3f _start = *start + (*end-*start)*jizz[i];
			const vector3f _end = *start + (*end-*start)*jizz[i+2];
			bool shitty_normal = is_equal_absolute(jizz[i], jizz[i+2], 1e-4f);

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
				geomBuffer->SetVertex(basevtx + j, _start+p1, n);
				geomBuffer->SetVertex(basevtx + steps + j, _end+p2, n);
			}
			for (int j=0; j<steps-1; j++) {
				geomBuffer->PushTri(basevtx+j, basevtx+j+1, basevtx+j+steps);
				geomBuffer->PushTri(basevtx+j+1, basevtx+j+steps+1, basevtx+j+steps);
			}
			geomBuffer->PushTri(basevtx+steps-1, basevtx, basevtx+2*steps-1);
			geomBuffer->PushTri(basevtx, basevtx+steps, basevtx+2*steps-1);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int vtxStart = geomBuffer->AllocVertices(6*steps);

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
			geomBuffer->SetVertex(vtxStart + i, (*start) + p1, -zax);
			geomBuffer->SetVertex(vtxStart + i + steps, (*end) + p1, zax);
		}

		for (int i=0; i<steps-1; i++) {
			// top cap
			geomBuffer->PushTri(vtxStart, vtxStart+i+1, vtxStart+i);
			// bottom cap
			geomBuffer->PushTri(vtxStart+steps, vtxStart+steps+i, vtxStart+steps+i+1);
		}

		// sides
		for (int i=0; i<steps; i++) {
			const vector3f &v1 = geomBuffer->GetVertex(vtxStart + i);
			const vector3f &v2 = geomBuffer->GetVertex(vtxStart + (i + 1)%steps);
			const vector3f &v3 = geomBuffer->GetVertex(vtxStart + i + steps);
			const vector3f &v4 = geomBuffer->GetVertex(vtxStart + (i + 1)%steps + steps);
			const vector3f norm = (v2-v1).Cross(v3-v1).Normalized();

			const int idx = vtxStart + 2*steps + i*4;
			geomBuffer->SetVertex(idx, v1, norm);
			geomBuffer->SetVertex(idx+1, v2, norm);
			geomBuffer->SetVertex(idx+2, v3, norm);
			geomBuffer->SetVertex(idx+3, v4, norm);

			geomBuffer->PushTri(idx, idx+1, idx+3);
			geomBuffer->PushTri(idx, idx+3, idx+2);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int vtxStart = geomBuffer->AllocVertices(xref ? 2*numPoints : numPoints);
		int vtxPos = vtxStart;

		const vector3f *prevSegEnd = segvtx[seg-1].v[ segvtx[seg-1].nv-1 ];
		// evaluate segments
		int maxSeg = seg;
		for (seg=0; seg<maxSeg; seg++) {
			if (segvtx[seg].nv == 1) {
				if (xref) {
					vector3f p = *segvtx[seg].v[0]; p.x = -p.x;
					geomBuffer->SetVertex(vtxPos + numPoints, p, xrefnorm);
				}
				geomBuffer->SetVertex(vtxPos++, *segvtx[seg].v[0], *normal);
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
					geomBuffer->SetVertex(vtxPos, p, *normal);
					if (xref) {
						p.x = -p.x;
						geomBuffer->SetVertex(vtxPos+numPoints, p, xrefnorm);
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
					geomBuffer->SetVertex(vtxPos, p, *normal);
					if (xref) {
						p.x = -p.x;
						geomBuffer->SetVertex(vtxPos+numPoints, p, xrefnorm);
					}
					vtxPos++;
				}
				prevSegEnd = segvtx[seg].v[2];
			}
		}

		for (int i=1; i<numPoints-1; i++) {
			geomBuffer->PushTri(vtxStart, vtxStart+i, vtxStart+i+1);
			if (xref) {
				geomBuffer->PushTri(vtxStart+numPoints, vtxStart+numPoints+1+i, vtxStart+numPoints+i);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int numVertsInPatch = divs*(1+divs)/2;
		const int vtxStart = geomBuffer->AllocVertices(numVertsInPatch * (xref ? 2 : 1));
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
				geomBuffer->SetVertex(vtxPos, p, norm);

				if (xref) {
					norm.x = -norm.x;
					p.x = -p.x;
					geomBuffer->SetVertex(vtxPos + numVertsInPatch, p, norm);
				}
				vtxPos++;
			}
		}
		//assert((vtxPos - vtxStart) == numVertsInPatch);

		vtxPos = vtxStart;
		for (int y=0; y<divs-1; y++) {
			const int adv = divs-y;
			geomBuffer->PushTri(vtxPos, vtxPos+adv, vtxPos+1);
			for (int x=1; x<adv-1; x++) {
				geomBuffer->PushTri(vtxPos+x, vtxPos+x+adv-1, vtxPos+x+adv);
				geomBuffer->PushTri(vtxPos+x, vtxPos+x+adv, vtxPos+x+1);
			}
			if (xref) {
				const int refVtxPos = vtxPos + numVertsInPatch;
				geomBuffer->PushTri(refVtxPos, refVtxPos+1, refVtxPos+adv);
				for (int x=1; x<adv-1; x++) {
					geomBuffer->PushTri(refVtxPos+x, refVtxPos+x+adv, refVtxPos+x+adv-1);
					geomBuffer->PushTri(refVtxPos+x, refVtxPos+x+1, refVtxPos+x+adv);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int numVertsInPatch = (divs_u+1)*(divs_v+1);
		const int vtxStart = geomBuffer->AllocVertices(numVertsInPatch * (xref ? 2 : 1));

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

				geomBuffer->SetVertex(vtxStart + i*(divs_v+1) + j, p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					geomBuffer->SetVertex(vtxStart + numVertsInPatch + i*(divs_v+1) + j, p, norm);
				}
			}
		}

		for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+1);
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+(divs_v+1), baseVtx+j+1+(divs_v+1));
			}
		}
		if (xref) for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + numVertsInPatch + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs_v+1));
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+(divs_v+1));
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int numVertsInPatch = (divs_v+1)*(divs_u+1);
		const int vtxStart = geomBuffer->AllocVertices(numVertsInPatch * (xref ? 2 : 1));


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

				geomBuffer->SetVertex(vtxStart + i*(divs_v+1) + j, p, norm);
				if (xref) {
					p.x = -p.x;
					norm.x = -norm.x;
					geomBuffer->SetVertex(vtxStart + numVertsInPatch + i*(divs_v+1) + j, p, norm);
				}
			}
		}

		for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+1);
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+(divs_v+1), baseVtx+j+1+(divs_v+1));
			}
		}
		if (xref) for (int i=0; i<divs_u; i++) {
			int baseVtx = vtxStart + numVertsInPatch + i*(divs_v+1);
			for (int j=0; j<divs_v; j++) {
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1, baseVtx+j+1+(divs_v+1));
				geomBuffer->PushTri(baseVtx+j, baseVtx+j+1+(divs_v+1), baseVtx+j+(divs_v+1));
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
		GetCurrentGeomBuffer(L)->SetMaterial(mat_name, mat);
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
			GetCurrentGeomBuffer(L)->PushUseMaterial(mat_name);
		} catch (LMR::LmrUnknownMaterial) {
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		const int nargs = lua_gettop(L);
		if (lua_isnil(L, 1)) {
			geomBuffer->SetTexture(0);
		} else {
			lua_getglobal(L, "CurrentDirectory");
			std::string dir = luaL_optstring(L, -1, ".");
			lua_pop(L, 1);

			const char *texfile = luaL_checkstring(L, 1);
			std::string t = FileSystem::JoinPathBelow(dir, texfile);
			if (nargs == 4) {
				// texfile, pos, uaxis, vaxis
				vector3f pos = *MyLuaVec::checkVec(L, 2);
				vector3f uaxis = *MyLuaVec::checkVec(L, 3);
				vector3f vaxis = *MyLuaVec::checkVec(L, 4);
				vector3f waxis = uaxis.Cross(vaxis);

				matrix4x4f trans = matrix4x4f::MakeInvRotMatrix(uaxis, vaxis, waxis);
				trans[12] = -pos.x;
				trans[13] = -pos.y;
				geomBuffer->SetTexMatrix(trans);
			}

			geomBuffer->SetTexture(t.c_str());
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

		if (lua_isnil(L, 1)) {
			geomBuffer->SetGlowMap(0);
		} else {
			lua_getglobal(L, "CurrentDirectory");
			std::string dir = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			const char *texfile = luaL_checkstring(L, 1);
			std::string t = dir + std::string("/") + texfile;
			geomBuffer->SetGlowMap(t.c_str());
		}
		return 0;
	}

		static matrix4x4f _textTrans;
		static vector3f _textNorm;
		static void _text_index_callback(int num, Uint16 *vals, void *callbackData) {
			GeomBuffer *geomBuffer = reinterpret_cast<GeomBuffer*>(callbackData);
			const int base = geomBuffer->GetVerticesPos();
			for (int i=0; i<num; i+=3) {
				geomBuffer->PushTri(vals[i]+base, vals[i+1]+base, vals[i+2]+base);
			}
		}
		static void _text_vertex_callback(int num, float offsetX, float offsetY, float *vals, void *callbackData) {
			GeomBuffer *geomBuffer = reinterpret_cast<GeomBuffer*>(callbackData);
			for (int i=0; i<num*3; i+=3) {
				vector3f p = vector3f(offsetX+vals[i], offsetY+vals[i+1], vals[i+2]);
				p = _textTrans * p;
				geomBuffer->PushVertex(p, _textNorm);
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
		s_font->GetStringGeometry(str, &_text_index_callback, &_text_vertex_callback, GetCurrentGeomBuffer(L));
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
		GetCurrentGeomBuffer(L)->SetGeomFlag(flag);
		return 0;
	}

	/*
	 * Function: zbias
	 *
	 * Fine-tune depth range. Overlapping geometry can be rendered without 
	 * z-fighting using this parameter.
	 *
	 * > zbias(amount)
	 *
	 * Parameters:
	 *
	 *   amount - adjustment value, use 0 to restore normal operation
	 *
	 * Example:
	 *
	 * > quad(v(-1,-0.5,0),v(1,-0.5,0),v(1,0.5,0),v(-1,0.5,0))
	 * > zbias(1)
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		int amount = luaL_checkinteger(L, 1);
		if (! amount) {
			geomBuffer->PushZBias(0);
		} else {
			geomBuffer->PushZBias(float(amount));
		}
		return 0;
	}

	static void _circle(int steps, const vector3f &center, const vector3f &normal, const vector3f &updir, float radius, GeomBuffer *geomBuffer) {
		const int vtxStart = geomBuffer->AllocVertices(steps);

		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(normal).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5f*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = center + radius * (sin(ang)*axis1 + cos(ang)*axis2);
			geomBuffer->SetVertex(vtxStart+i, p, normal);
		}

		for (int i=2; i<steps; i++) {
			// top cap
			geomBuffer->PushTri(vtxStart, vtxStart+i-1, vtxStart+i);
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
		_circle(steps, *center, *normal, *updir, radius, GetCurrentGeomBuffer(L));
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		_circle(steps, center, normal, updir, radius, geomBuffer);
		center.x = -center.x;
		normal.x = -normal.x;
		updir.x = -updir.x;
		_circle(steps, center, normal, updir, radius, geomBuffer);
		return 0;
	}

	static void _tube(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float inner_radius, float outer_radius, GeomBuffer *geomBuffer) {
		const int vtxStart = geomBuffer->AllocVertices(8*steps);

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

			geomBuffer->SetVertex(vtxStart+i, start+p_outer, p);
			geomBuffer->SetVertex(vtxStart+i+steps, end+p_outer, p);
			geomBuffer->SetVertex(vtxStart+i+2*steps, start+p_inner, -p);
			geomBuffer->SetVertex(vtxStart+i+3*steps, end+p_inner, -p);

			geomBuffer->SetVertex(vtxStart+i+4*steps, start+p_outer, -dir);
			geomBuffer->SetVertex(vtxStart+i+5*steps, end+p_outer, dir);
			geomBuffer->SetVertex(vtxStart+i+6*steps, start+p_inner, -dir);
			geomBuffer->SetVertex(vtxStart+i+7*steps, end+p_inner, dir);
		}

		for (int i=0; i<steps-1; i++) {
			geomBuffer->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			geomBuffer->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
			geomBuffer->PushTri(vtxStart+i+2*steps, vtxStart+i+steps+2*steps, vtxStart+i+1+2*steps);
			geomBuffer->PushTri(vtxStart+i+1+2*steps, vtxStart+i+steps+2*steps, vtxStart+i+steps+1+2*steps);
		}
		geomBuffer->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		geomBuffer->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);
		
		geomBuffer->PushTri(vtxStart+3*steps-1, vtxStart+4*steps-1, vtxStart+2*steps);
		geomBuffer->PushTri(vtxStart+2*steps, vtxStart+4*steps-1, vtxStart+3*steps);

		for (int i=0; i<steps-1; i++) {
			// 'start' end
			geomBuffer->PushTri(vtxStart+4*steps+i, vtxStart+6*steps+i, vtxStart+4*steps+i+1);
			
			geomBuffer->PushTri(vtxStart+4*steps+i+1, vtxStart+6*steps+i, vtxStart+6*steps+i+1);
			// 'end' end *cough*
			geomBuffer->PushTri(vtxStart+5*steps+i, vtxStart+5*steps+i+1, vtxStart+7*steps+i);
			
			geomBuffer->PushTri(vtxStart+5*steps+i+1, vtxStart+7*steps+i+1, vtxStart+7*steps+i);
		}
		// 'start' end
		geomBuffer->PushTri(vtxStart+5*steps-1, vtxStart+7*steps-1, vtxStart+4*steps);
		geomBuffer->PushTri(vtxStart+4*steps, vtxStart+7*steps-1, vtxStart+6*steps);
		// 'end' end
		geomBuffer->PushTri(vtxStart+6*steps-1, vtxStart+5*steps, vtxStart+8*steps-1);
		geomBuffer->PushTri(vtxStart+5*steps, vtxStart+7*steps, vtxStart+8*steps-1);
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
		_tube(steps, *start, *end, *updir, inner_radius, outer_radius, GetCurrentGeomBuffer(L));
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		_tube(steps, start, end, updir, inner_radius, outer_radius, geomBuffer);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_tube(steps, start, end, updir, inner_radius, outer_radius, geomBuffer);
		return 0;
	}

	static void _tapered_cylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius1, float radius2, GeomBuffer *geomBuffer) {
		const int vtxStart = geomBuffer->AllocVertices(4*steps);

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

			geomBuffer->SetVertex(vtxStart+i, start+p1, n);
			geomBuffer->SetVertex(vtxStart+i+steps, end+p2, n);
			geomBuffer->SetVertex(vtxStart+i+2*steps, start+p1, -dir);
			geomBuffer->SetVertex(vtxStart+i+3*steps, end+p2, dir);
		}

		for (int i=0; i<steps-1; i++) {
			geomBuffer->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			geomBuffer->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		geomBuffer->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		geomBuffer->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);

		for (int i=2; i<steps; i++) {
			// bottom cap
			geomBuffer->PushTri(vtxStart+2*steps, vtxStart+2*steps+i, vtxStart+2*steps+i-1);
			// top cap
			geomBuffer->PushTri(vtxStart+3*steps, vtxStart+3*steps+i-1, vtxStart+3*steps+i);
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
		_tapered_cylinder(steps, *start, *end, *updir, radius1, radius2, GetCurrentGeomBuffer(L));
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		_tapered_cylinder(steps, start, end, updir, radius1, radius2, geomBuffer);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_tapered_cylinder(steps, start, end, updir, radius1, radius2, geomBuffer);
		return 0;
	}

	static void _cylinder(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius, GeomBuffer *geomBuffer) {
		const int vtxStart = geomBuffer->AllocVertices(4*steps);

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			geomBuffer->SetVertex(vtxStart+i, start+p, n);
			geomBuffer->SetVertex(vtxStart+i+steps, end+p, n);
			geomBuffer->SetVertex(vtxStart+i+2*steps, start+p, -dir);
			geomBuffer->SetVertex(vtxStart+i+3*steps, end+p, dir);
		}

		for (int i=0; i<steps-1; i++) {
			geomBuffer->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			geomBuffer->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		geomBuffer->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		geomBuffer->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);

		for (int i=2; i<steps; i++) {
			// bottom cap
			geomBuffer->PushTri(vtxStart+2*steps, vtxStart+2*steps+i, vtxStart+2*steps+i-1);
			// top cap
			geomBuffer->PushTri(vtxStart+3*steps, vtxStart+3*steps+i-1, vtxStart+3*steps+i);
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
		_cylinder(steps, *start, *end, *updir, radius, GetCurrentGeomBuffer(L));
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		_cylinder(steps, start, end, updir, radius, geomBuffer);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_cylinder(steps, start, end, updir, radius, geomBuffer);
		return 0;
	}

	static void _ring(int steps, const vector3f &start, const vector3f &end, const vector3f &updir, float radius, GeomBuffer *geomBuffer) {

		const vector3f dir = (end-start).Normalized();
		const vector3f axis1 = updir.Normalized();
		const vector3f axis2 = updir.Cross(dir).Normalized();

		const int vtxStart = geomBuffer->AllocVertices(2*steps);

		const float inc = 2.0f*M_PI / float(steps);
		float ang = 0.5*inc;
		radius /= cosf(ang);
		for (int i=0; i<steps; i++, ang += inc) {
			vector3f p = radius * (sin(ang)*axis1 + cos(ang)*axis2);
			vector3f n = p.Normalized();

			geomBuffer->SetVertex(vtxStart+i, start+p, n);
			geomBuffer->SetVertex(vtxStart+i+steps, end+p, n);
		}

		for (int i=0; i<steps-1; i++) {
			geomBuffer->PushTri(vtxStart+i, vtxStart+i+1, vtxStart+i+steps);
			geomBuffer->PushTri(vtxStart+i+1, vtxStart+i+steps+1, vtxStart+i+steps);
		}
		geomBuffer->PushTri(vtxStart+steps-1, vtxStart, vtxStart+2*steps-1);
		geomBuffer->PushTri(vtxStart, vtxStart+steps, vtxStart+2*steps-1);
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
		_ring(steps, *start, *end, *updir, radius, GetCurrentGeomBuffer(L));
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		_ring(steps, start, end, updir, radius, geomBuffer);
		start.x = -start.x;
		end.x = -end.x;
		updir.x = -updir.x;
		_ring(steps, start, end, updir, radius, geomBuffer);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = geomBuffer->PushVertex(*v1, n);
		int i2 = geomBuffer->PushVertex(*v2, n);
		int i3 = geomBuffer->PushVertex(*v3, n);
		geomBuffer->PushInvisibleTri(i1, i2, i3);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = geomBuffer->PushVertex(*v1, n);
		int i2 = geomBuffer->PushVertex(*v2, n);
		int i3 = geomBuffer->PushVertex(*v3, n);
		geomBuffer->PushTri(i1, i2, i3);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		
		vector3f n = (v1-v2).Cross(v1-v3).Normalized();
		int i1 = geomBuffer->PushVertex(v1, n);
		int i2 = geomBuffer->PushVertex(v2, n);
		int i3 = geomBuffer->PushVertex(v3, n);
		geomBuffer->PushTri(i1, i2, i3);
		v1.x = -v1.x; v2.x = -v2.x; v3.x = -v3.x; n.x = -n.x;
		i1 = geomBuffer->PushVertex(v1, n);
		i2 = geomBuffer->PushVertex(v2, n);
		i3 = geomBuffer->PushVertex(v3, n);
		geomBuffer->PushTri(i1, i3, i2);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		
		vector3f n = ((*v1)-(*v2)).Cross((*v1)-(*v3)).Normalized();
		int i1 = geomBuffer->PushVertex(*v1, n);
		int i2 = geomBuffer->PushVertex(*v2, n);
		int i3 = geomBuffer->PushVertex(*v3, n);
		int i4 = geomBuffer->PushVertex(*v4, n);
		geomBuffer->PushTri(i1, i2, i3);
		geomBuffer->PushTri(i1, i3, i4);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		
		vector3f n = (v1-v2).Cross(v1-v3).Normalized();
		int i1 = geomBuffer->PushVertex(v1, n);
		int i2 = geomBuffer->PushVertex(v2, n);
		int i3 = geomBuffer->PushVertex(v3, n);
		int i4 = geomBuffer->PushVertex(v4, n);
		geomBuffer->PushTri(i1, i2, i3);
		geomBuffer->PushTri(i1, i3, i4);
		v1.x = -v1.x; v2.x = -v2.x; v3.x = -v3.x; v4.x = -v4.x; n.x = -n.x;
		i1 = geomBuffer->PushVertex(v1, n);
		i2 = geomBuffer->PushVertex(v2, n);
		i3 = geomBuffer->PushVertex(v3, n);
		i4 = geomBuffer->PushVertex(v4, n);
		geomBuffer->PushTri(i1, i3, i2);
		geomBuffer->PushTri(i1, i4, i3);
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
		GetCurrentGeomBuffer(L)->PushThruster(*pos, *dir, power, linear_only);
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
		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);
		geomBuffer->PushThruster(pos, *dir, power, linear_only);
		pos.x = -pos.x;
		geomBuffer->PushThruster(pos, *dir, power, linear_only);
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		double t = objParams->time;
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		if (objParams->equipment) {
			const char *slotName = luaL_checkstring(L, 1);
			int index = luaL_optinteger(L, 2, 0);
			Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(L, "EquipSlot", slotName));

			if (index > 0) {
				// index - 1 because Lua uses 1-based indexing
				Equip::Type equip = objParams->equipment->Get(slot, index - 1);
				if (equip == Equip::NONE)
					lua_pushnil(L);
				else
					lua_pushstring(L, LuaConstants::GetConstantString(L, "EquipType", equip));
				return 1;
			} else {
				const EquipSet &es = *objParams->equipment;
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		if (objParams->animationNamespace) {
			const char *animName = luaL_checkstring(L, 1);
			int anim = LuaConstants::GetConstant(L, objParams->animationNamespace, animName);
			assert(anim >= 0 && anim < LmrObjParams::LMR_ANIMATION_MAX);
			lua_pushinteger(L, objParams->animStages[anim]);
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		if (objParams->animationNamespace) {
			const char *animName = luaL_checkstring(L, 1);
			int anim = LuaConstants::GetConstant(L, objParams->animationNamespace, animName);
			assert(anim >= 0 && anim < LmrObjParams::LMR_ANIMATION_MAX);
			lua_pushnumber(L, objParams->animValues[anim]);
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		// if there is equipment then there should also be a flightState
		if (objParams->equipment) {
			lua_pushstring(L, LuaConstants::GetConstantString(L, "ShipFlightState", objParams->flightState));
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		lua_pushstring(L, objParams->label ? objParams->label : "");
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
		const LmrObjParams *objParams = GetCurrentObjParams(L);

		int n = luaL_checkinteger(L, 1);
		lua_createtable (L, 11, 0);

		const LmrMaterial &mat = objParams->pMat[n];

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
		const vector3f cv = *MyLuaVec::checkVec(L, 3);
		const Color color(cv.x,cv.y,cv.z,1.0f);
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
		GetCurrentGeomBuffer(L)->PushBillboards(texname, size, color, points.size(), &points[0]);
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
			const int i1, const int i2, const int i3, int depth, GeomBuffer *geomBuffer)
	{
		if (depth == 0) {
			geomBuffer->PushTri(i1, i3, i2);
			return;
		}

		const vector3f v12 = (v1+v2).Normalized();
		const vector3f v23 = (v2+v3).Normalized();
		const vector3f v31 = (v3+v1).Normalized();
		const int i12 = geomBuffer->PushVertex(trans * v12, trans.ApplyRotationOnly(v12));
		const int i23 = geomBuffer->PushVertex(trans * v23, trans.ApplyRotationOnly(v23));
		const int i31 = geomBuffer->PushVertex(trans * v31, trans.ApplyRotationOnly(v31));
		_sphere_subdivide(trans, v1, v12, v31, i1, i12, i31, depth-1, geomBuffer);
		_sphere_subdivide(trans, v2, v23, v12, i2, i23, i12, depth-1, geomBuffer);
		_sphere_subdivide(trans, v3, v31, v23, i3, i31, i23, depth-1, geomBuffer);
		_sphere_subdivide(trans, v12, v23, v31, i12, i23, i31, depth-1, geomBuffer);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(l);

		int vi[12];
		for (i=0; i<12; i++) {
			const vector3f &v = icosahedron_vertices[i];
			vi[i] = geomBuffer->PushVertex(trans * v, trans.ApplyRotationOnly(v));
		}
			
		for (i=0; i<20; i++) {
			_sphere_subdivide (trans, icosahedron_vertices[icosahedron_faces[i][0]],
					icosahedron_vertices[icosahedron_faces[i][1]],
					icosahedron_vertices[icosahedron_faces[i][2]],
					vi[icosahedron_faces[i][0]],
					vi[icosahedron_faces[i][1]],
					vi[icosahedron_faces[i][2]],
					subdivs, geomBuffer);
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

		GeomBuffer *geomBuffer = GetCurrentGeomBuffer(l);

		int *idx = new int[LONG_SEGS+2];
		int *idx2 = new int[LONG_SEGS+2];
		// cap the top
		float cosLat2 = cos(sliceAngle1);
		float sinLat2 = sin(sliceAngle1);
		vector3f cap_norm = yaxis.Normalized();
		for (int i=0; i<=LONG_SEGS; i++) {
			vector3f v0(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
			idx[i] = geomBuffer->PushVertex(trans * v0, cap_norm);
			idx2[i] = geomBuffer->PushVertex(trans * v0, trans.ApplyRotationOnly(v0)); // for later
		}
		for (int i=0; i<LONG_SEGS-1; i++) {
			geomBuffer->PushTri(idx[0], idx[i+2], idx[i+1]);
		}

		for (int j=1; j<=LAT_SEGS; j++) {
			cosLat2 = cos(sliceAngle1+latDiff*j);
			sinLat2 = sin(sliceAngle1+latDiff*j);
			for (int i=0; i<=LONG_SEGS; i++) {
				vector3f v1(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
				idx[i] = idx2[i];
				idx2[i] = geomBuffer->PushVertex(trans * v1, trans.ApplyRotationOnly(v1));
			}
			for (int i=0; i<LONG_SEGS; i++) {
				geomBuffer->PushTri(idx[i], idx2[i+1], idx2[i]);
				geomBuffer->PushTri(idx[i], idx[i+1], idx2[i+1]);
			}
		}
		// cap the bottom
		cap_norm = -cap_norm;
		for (int i=0; i<=LONG_SEGS; i++) {
			vector3f v1(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
			idx[i] = geomBuffer->PushVertex(trans * v1, cap_norm);
		}
		for (int i=0; i<LONG_SEGS-1; i++) {
			geomBuffer->PushTri(idx[0], idx[i+1], idx[i+2]);
		}
		delete [] idx;
		delete [] idx2;

		return 0;
	}

} /* namespace ModelFuncs */

namespace ObjLoader {
	static std::map<std::string, std::string> load_mtl_file(lua_State *L, const char* mtl_file) {
		std::map<std::string, std::string> mtl_map;
		char name[1024] = "", file[1024];

		lua_getglobal(L, "CurrentDirectory");
		std::string curdir = luaL_optstring(L, -1, ".");
		lua_pop(L, 1);

		const std::string path = FileSystem::JoinPathBelow(curdir, mtl_file);
		RefCountedPtr<FileSystem::FileData> mtlfiledata = FileSystem::gameDataFiles.ReadFile(path);
		if (!mtlfiledata) {
			printf("Could not open %s\n", path.c_str());
			throw LMR::LmrUnknownMaterial();
		}

		std::string line;
		StringRange mtlfilerange = mtlfiledata->AsStringRange();
		for (int line_no=1; !mtlfilerange.Empty(); line_no++) {
			line = mtlfilerange.ReadLine().StripSpace().ToString();

			if (!strncasecmp(line.c_str(), "newmtl ", 7)) {
				PiVerify(1 == sscanf(line.c_str(), "newmtl %s", name));
			}
			if (!strncasecmp(line.c_str(), "map_K", 5) && strlen(name) > 0) {
				PiVerify(1 == sscanf(line.c_str(), "map_Kd %s", file));
				mtl_map[name] = file;
			}
		}

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
		const std::string curdir = luaL_optstring(L, -1, ".");
		lua_pop(L, 1);

		const std::string path = FileSystem::JoinPathBelow(curdir, obj_name);
		RefCountedPtr<FileSystem::FileData> objdata = FileSystem::gameDataFiles.ReadFile(path);
		if (!objdata) {
			Error("Could not open '%s'\n", path.c_str());
		}

		StringRange objdatabuf = objdata->AsStringRange();

		std::vector<vector3f> vertices;
		std::vector<vector3f> texcoords;
		std::vector<vector3f> normals;
		std::map<std::string, std::string> mtl_map;
		std::string texture;

		// maps obj file vtx_idx,norm_idx to a single GeomBuffer vertex index
		std::map<objTriplet, int> vtxmap;

		std::string line;
		for (int line_no=1; !objdatabuf.Empty(); line_no++) {
			line = objdatabuf.ReadLine().ToString();
			const char *buf = line.c_str();

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
				const char *bit[MAX_VTX_FACE];
				const char *pos = &buf[2];
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

				GeomBuffer *geomBuffer = GetCurrentGeomBuffer(L);

				if (build_normals) {
					// not nice without normals
					for (int i=0; i<numBits-2; i++) {
						vector3f &a = vertices[vi[0]];
						vector3f &b = vertices[vi[i+1]];
						vector3f &c = vertices[vi[i+2]];
						vector3f n = (a-b).Cross(a-c).Normalized();
						int vtxStart = geomBuffer->AllocVertices(3);
						if (texcoords.empty()) {
							// no UV coords
							geomBuffer->SetVertex(vtxStart, a, n);
							geomBuffer->SetVertex(vtxStart+1, b, n);
							geomBuffer->SetVertex(vtxStart+2, c, n);
						} else {
							geomBuffer->SetVertex(vtxStart, a, n, texcoords[ti[i]].x, texcoords[ti[i]].y);
							geomBuffer->SetVertex(vtxStart+1, b, n, texcoords[ti[i+1]].x, texcoords[ti[i+1]].y);
							geomBuffer->SetVertex(vtxStart+2, c, n, texcoords[ti[i+2]].x, texcoords[ti[i+2]].y);
						}
						if (texture.size()) geomBuffer->SetTexture(texture.c_str());
						geomBuffer->PushTri(vtxStart, vtxStart+1, vtxStart+2);
					}
				} else {
					for (int i=0; i<numBits; i++) {
						// SHARE THE PAIN!
						objTriplet t = { vi[i], ni[i], ti[i] };
						std::map<objTriplet, int>::iterator it = vtxmap.find(t);
						if (it == vtxmap.end()) {
							// insert the horrible thing
							int vtxStart = geomBuffer->AllocVertices(1);
							if (texcoords.empty()) {
								// no UV coords
								geomBuffer->SetVertex(vtxStart, vertices[vi[i]], normals[ni[i]]);
							} else {
								geomBuffer->SetVertex(vtxStart, vertices[vi[i]], normals[ni[i]], texcoords[ti[i]].x, texcoords[ti[i]].y);
							}
							vtxmap[t] = vtxStart;
							realVtxIdx[i] = vtxStart;
						} else {
							realVtxIdx[i] = (*it).second;
						}
					}
					if (texture.size()) geomBuffer->SetTexture(texture.c_str());
					if (numBits == 3) {
						geomBuffer->PushTri(realVtxIdx[0], realVtxIdx[1], realVtxIdx[2]);
					} else if (numBits == 4) {
						geomBuffer->PushTri(realVtxIdx[0], realVtxIdx[1], realVtxIdx[2]);
						geomBuffer->PushTri(realVtxIdx[0], realVtxIdx[2], realVtxIdx[3]);
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
						} catch (LMR::LmrUnknownMaterial) {
							printf("Warning: Missing material %s (%s) in %s\n", mtl_map[mat_name].c_str(), mat_name, obj_name);
						}
					}
				} else {
					Error("Obj file has no normals or is otherwise too weird at line %d\n", line_no);
				}
			}
		}
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
	
	s_models[model_name] = new LmrModel(L, model_name);
	return 0;
}

void ModelCompilerInit()
{
#if 0
	s_cacheDir = FileSystem::GetUserDir("model_cache");
	FileSystem::rawFileSystem.MakeDirectory(s_cacheDir);
	_detect_model_changes();
#endif

	s_font = new VectorFont(FontConfig(FileSystem::JoinPathBelow(FileSystem::GetDataDir(), "fonts/WorldFont.ini")));

	lua_State *L = lua_open();
	luaL_openlibs(L);

	LUA_DEBUG_START(L);

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

	pi_lua_dofile(L, "pimodels.lua");

	LUA_DEBUG_END(L, 0);

#if 0
	_write_model_crc_file();
#endif
}


void ModelCompilerUninit()
{
	std::map<std::string, LmrModel*>::iterator it_model;
	for (it_model=s_models.begin(); it_model != s_models.end(); ++it_model)	{
		delete (*it_model).second;
	}

#if 0
	lua_close(L);
#endif

	delete s_font;
}

LmrModel *LookupModelByName(const char *name)
{
	std::map<std::string, LmrModel*>::iterator i = s_models.find(name);

	if (i == s_models.end()) {
		throw LmrModelNotFoundException();
	}
	return (*i).second;
}	

void GetModelsWithTag(const char *tag, std::vector<LmrModel*> &outModels)
{
	for (std::map<std::string, LmrModel*>::iterator i = s_models.begin();
			i != s_models.end(); ++i) {
		if (i->second->HasTag(tag))
			outModels.push_back(i->second);
	}
}

}
