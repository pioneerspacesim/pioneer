// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "LuaMatrix.h"
#include "LuaVector.h"
#include "LuaUtils.h"

static int l_matrix_new(lua_State *L)
{
	int n = lua_gettop(L);
	matrix4x4f *out = LuaMatrix::PushNewToLua(L);
	if (n == 0) {
		*out = matrix4x4f(0.0);
	} else if (n == 3) {
		vector3f v1 = LuaVector::CheckFromLuaF(L, 1);
		vector3f v2 = LuaVector::CheckFromLuaF(L, 2);
		vector3f v3 = LuaVector::CheckFromLuaF(L, 3);
		*out = matrix4x4f::MakeRotMatrix(v1, v2, v3);
	} else if (n == 4) {
		vector3f v1 = LuaVector::CheckFromLuaF(L, 1);
		vector3f v2 = LuaVector::CheckFromLuaF(L, 2);
		vector3f v3 = LuaVector::CheckFromLuaF(L, 3);
		vector3f v4 = LuaVector::CheckFromLuaF(L, 4);
		*out = matrix4x4f::MakeRotMatrix(v1, v2, v3);
		(*out)[12] = v4.x;
		(*out)[13] = v4.y;
		(*out)[14] = v4.z;
	} else if (n == 16) {
		for (int i=0; i<16; i++) {
			(*out)[i] = luaL_checknumber(L, i+1);
		}
	} else {
		luaL_error(L, "bad arguments to matrix.new");
	}
	return 1;
}

static int l_matrix_new_identity(lua_State *L)
{
	LuaMatrix::PushToLua(L, matrix4x4f::Identity());
	return 1;
}

static int l_matrix_new_rotate(lua_State *L)
{
	float ang = luaL_checknumber(L, 1);
	vector3f v = LuaVector::CheckFromLuaF(L, 2);
	v = v.Normalized();
	LuaMatrix::PushToLua(L, matrix4x4f::RotateMatrix(ang, v.x, v.y, v.z));
	return 1;
}

static int l_matrix_new_translate(lua_State *L)
{
	const vector3f v = LuaVector::CheckFromLuaF(L, 1);
	LuaMatrix::PushToLua(L, matrix4x4f::Translation(v));
	return 1;
}

static int l_matrix_new_pose(lua_State *L)
{
	const vector3f pos = LuaVector::CheckFromLuaF(L, 1);
	const vector3f _xaxis = LuaVector::CheckFromLuaF(L, 2);
	const vector3f _yaxis = LuaVector::CheckFromLuaF(L, 3);

	vector3f zaxis = _xaxis.Cross(_yaxis).Normalized();
	vector3f xaxis = _yaxis.Cross(zaxis).Normalized();
	vector3f yaxis = zaxis.Cross(xaxis);

	matrix4x4f *out = LuaMatrix::PushNewToLua(L);
	*out = matrix4x4f::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	(*out)[12] = pos.x;
	(*out)[13] = pos.y;
	(*out)[14] = pos.z;
	return 1;
}

static int l_matrix_new_scale(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		double s = lua_tonumber(L, 1);
		LuaMatrix::PushToLua(L, matrix4x4f::ScaleMatrix(s));
	} else {
		const vector3f v = LuaVector::CheckFromLuaF(L, 1);
		LuaMatrix::PushToLua(L, matrix4x4f::ScaleMatrix(v.x, v.y, v.z));
	}
	return 1;
}

#if 0
static int l_matrix_print(lua_State *L)
{
	matrix4x4f *v = checkMatrix(L, 1);
	Output("[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n\n",
			(*v)[0], (*v)[1], (*v)[2], (*v)[3],
			(*v)[4], (*v)[5], (*v)[6], (*v)[7],
			(*v)[8], (*v)[9], (*v)[10], (*v)[11],
			(*v)[12], (*v)[13], (*v)[14], (*v)[15]);
	return 0;
}
#endif

static int l_matrix_add (lua_State *L)
{
	const matrix4x4f *a = LuaMatrix::CheckFromLua(L, 1);
	const matrix4x4f *b = LuaMatrix::CheckFromLua(L, 2);
	LuaMatrix::PushToLua(L, *a + *b);
	return 1;
}

static int l_matrix_sub (lua_State *L)
{
	const matrix4x4f *a = LuaMatrix::CheckFromLua(L, 1);
	const matrix4x4f *b = LuaMatrix::CheckFromLua(L, 2);
	LuaMatrix::PushToLua(L, *a - *b);
	return 1;
}

static int l_matrix_unm (lua_State *L)
{
	const matrix4x4f *m = LuaMatrix::CheckFromLua(L, 1);
	LuaMatrix::PushToLua(L, -(*m));
	return 1;
}

static int l_matrix_mul (lua_State *L)
{
	if (lua_isnumber(L,1)) {
		double scale = lua_tonumber(L, 1);
		const matrix4x4f *m = LuaMatrix::CheckFromLua(L, 2);
		LuaMatrix::PushToLua(L, scale * *m);
		return 1;
	} else if (lua_isnumber(L, 2)) {
		const matrix4x4f *m = LuaMatrix::CheckFromLua(L, 1);
		double scale = lua_tonumber(L, 2);
		LuaMatrix::PushToLua(L, *m * scale);
		return 1;
	} else {
		const matrix4x4f *a = LuaMatrix::CheckFromLua(L, 1);
		const vector3d *bvec = LuaVector::GetFromLua(L, 2);
		if (bvec) {
			// matrix * vector
			LuaVector::PushToLuaF(L, (*a) * vector3f(*bvec));
		} else {
			// matrix * matrix
			const matrix4x4f *b = LuaMatrix::CheckFromLua(L, 2);
			LuaMatrix::PushToLua(L, (*a) * (*b));
		}
		return 1;
	}
}

static int l_matrix_div (lua_State *L)
{
	const matrix4x4f *a = LuaMatrix::CheckFromLua(L, 1);
	double scale = luaL_checknumber(L, 2);
	LuaMatrix::PushToLua(L, *a * (1.0 / scale));
	return 1;
}

static int l_matrix_inverse(lua_State *L)
{
	const matrix4x4f *m = LuaMatrix::CheckFromLua(L, 1);
	LuaMatrix::PushToLua(L, m->InverseOf());
	return 1;
}

static const luaL_Reg l_matrix_lib[] = {
	{ "new", &l_matrix_new },
	{ "identity", &l_matrix_new_identity },
	{ "rotate", &l_matrix_new_rotate },
	{ "scale", &l_matrix_new_scale },
	{ "translate", &l_matrix_new_translate },
	{ "pose", &l_matrix_new_pose },
	{ "inverse", &l_matrix_inverse },
	{ 0, 0 }
};

static const luaL_Reg l_matrix_meta[] = {
	//{ "__tostring", &l_matrix_tostring },
	{ "__add", &l_matrix_add },
	{ "__sub", &l_matrix_sub },
	{ "__mul", &l_matrix_mul },
	{ "__div", &l_matrix_div },
	{ "__unm", &l_matrix_unm },
	{ "inverse", &l_matrix_inverse },
	{ 0, 0 }
};

const char LuaMatrix::LibName[] = "matrix";
const char LuaMatrix::TypeName[] = "matrix";

void LuaMatrix::Register(lua_State *L)
{
	luaL_newlib(L, l_matrix_lib);
	lua_setglobal(L, LuaMatrix::LibName);

	luaL_newmetatable(L, LuaMatrix::TypeName);
	luaL_setfuncs(L, l_matrix_meta, 0);

	// map index back to the metatable
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");

	// pop the metatable
	lua_pop(L, 1);
}

matrix4x4f *LuaMatrix::PushNewToLua(lua_State *L)
{
	matrix4x4f *v = static_cast<matrix4x4f*>(lua_newuserdata(L, sizeof(matrix4x4f)));
	luaL_setmetatable(L, LuaMatrix::TypeName);
	return v;
}

const matrix4x4f *LuaMatrix::GetFromLua(lua_State *L, int idx)
{
	return static_cast<matrix4x4f*>(luaL_testudata(L, idx, LuaMatrix::TypeName));
}

const matrix4x4f *LuaMatrix::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<matrix4x4f*>(luaL_checkudata(L, idx, LuaMatrix::TypeName));
}
