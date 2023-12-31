// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaVector.h"
#include "JsonUtils.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaVector2.h"

void pi_lua_generic_pull(lua_State *l, int index, vector3d *&out)
{
	out = LuaVector::CheckFromLua(l, index);
}

vector3d construct_vec3(lua_State *L, int index)
{
	const vector2d *vec2 = LuaVector2::GetFromLua(L, index);
	double x, y, z;
	if (vec2 != nullptr) {
		x = vec2->x, y = vec2->y;
		z = luaL_optnumber(L, index + 1, 0.0);
	} else {
		x = luaL_checknumber(L, index);
		if (lua_gettop(L) == index)
			y = x, z = x;
		else {
			y = luaL_checknumber(L, index + 1);
			z = luaL_checknumber(L, index + 2);
		}
	}
	return vector3d(x, y, z);
}

static int l_vector_new(lua_State *L)
{
	LuaVector::PushToLua(L, construct_vec3(L, 1));
	return 1;
}

/*
	Construct a new vector from:
	- one double
	- three doubles x, y, z
	- vector2 and an optional double
*/
static int l_vector_call(lua_State *L)
{
	LuaVector::PushToLua(L, construct_vec3(L, 2));
	return 1;
}

// set all three values of a Vector3 without allocating new memory.
static int l_vector_set(lua_State *L)
{
	LUA_DEBUG_START(L);
	*LuaVector::CheckFromLua(L, 1) = construct_vec3(L, 2);
	lua_pushvalue(L, 1);
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_vector_tostring(lua_State *L)
{
	const vector3d *v = LuaVector::CheckFromLua(L, 1);
	lua_pushfstring(L, "Vector3(%f, %f, %f)", v->x, v->y, v->z);
	return 1;
}

static int l_vector_add(lua_State *L)
{
	const vector3d *a = LuaVector::CheckFromLua(L, 1);
	const vector3d *b = LuaVector::CheckFromLua(L, 2);
	LuaVector::PushToLua(L, *a + *b);
	return 1;
}

static int l_vector_sub(lua_State *L)
{
	const vector3d *a = LuaVector::CheckFromLua(L, 1);
	const vector3d *b = LuaVector::CheckFromLua(L, 2);
	LuaVector::PushToLua(L, *a - *b);
	return 1;
}

static int l_vector_mul(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		const double s = lua_tonumber(L, 1);
		const vector3d *v = LuaVector::CheckFromLua(L, 2);
		LuaVector::PushToLua(L, s * *v);
	} else if (lua_isnumber(L, 2)) {
		const vector3d *v = LuaVector::CheckFromLua(L, 1);
		const double s = lua_tonumber(L, 2);
		LuaVector::PushToLua(L, *v * s);
	} else {
		const vector3d *v1 = LuaVector::CheckFromLua(L, 1);
		const vector3d *v2 = LuaVector::CheckFromLua(L, 2);
		LuaVector::PushToLua(L, *v1 * *v2);
	}
	return 1;
}

static int l_vector_div(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		const vector3d *v = LuaVector::CheckFromLua(L, 1);
		const double s = lua_tonumber(L, 2);
		LuaVector::PushToLua(L, *v / s);
		return 1;
	} else if (lua_isnumber(L, 1)) {
		return luaL_error(L, "cannot divide a scalar by a vector");
	} else {
		return luaL_error(L, "Vector3 div not involving a vector (huh?)");
	}
}

static int l_vector_unm(lua_State *L)
{
	const vector3d *v = LuaVector::CheckFromLua(L, 1);
	LuaVector::PushToLua(L, -*v);
	return 1;
}

static int l_vector_new_index(lua_State *L)
{
	vector3d *v = LuaVector::CheckFromLua(L, 1);
	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *attr = luaL_checkstring(L, 2);
		if (attr[0] == 'x') {
			v->x = luaL_checknumber(L, 3);
		} else if (attr[0] == 'y') {
			v->y = luaL_checknumber(L, 3);
		} else if (attr[0] == 'z') {
			v->z = luaL_checknumber(L, 3);
		} else {
			luaL_error(L, "Index '%s' is not available: use 'x', 'y' or 'z'", attr);
		}

	} else {
		luaL_error(L, "Expected Vector3, but type is '%s'", luaL_typename(L, 2));
	}
	LuaVector::PushToLua(L, *v);
	return 1;
}

static int l_vector_index(lua_State *L)
{
	const vector3d *v = LuaVector::CheckFromLua(L, 1);
	size_t len = 0;
	const char *attr = nullptr;
	if (lua_type(L, 2) == LUA_TSTRING) {
		attr = lua_tolstring(L, 2, &len);
		if (attr != nullptr && len == 1) {
			if (attr[0] == 'x') {
				lua_pushnumber(L, v->x);
				return 1;
			} else if (attr[0] == 'y') {
				lua_pushnumber(L, v->y);
				return 1;
			} else if (attr[0] == 'z') {
				lua_pushnumber(L, v->z);
				return 1;
			}
		};
	} else {
		luaL_error(L, "Expected Vector, but type is '%s'", luaL_typename(L, 2));
	}
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "methods");
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);

	return 1;
}

static luaL_Reg l_vector_meta[] = {
	{ "__tostring", &l_vector_tostring },
	{ "__add", &l_vector_add },
	{ "__sub", &l_vector_sub },
	{ "__mul", &l_vector_mul },
	{ "__div", &l_vector_div },
	{ "__unm", &l_vector_unm },
	{ "__index", &l_vector_index },
	{ "__newindex", &l_vector_new_index },
	{ "__call", &l_vector_set },
	{ 0, 0 }
};

static bool _serialize_vector3(lua_State *l, Json &out)
{
	const auto *vec = LuaVector::GetFromLua(l, -1);
	if (!vec) return false;

	VectorToJson(out, *vec);
	return true;
}

static bool _deserialize_vector3(lua_State *l, const Json &out)
{
	vector3d *vec = LuaVector::PushNewToLua(l);
	JsonToVector(vec, out);
	return true;
}

const char LuaVector::LibName[] = "Vector3";
const char LuaVector::TypeName[] = "Vector3";

void LuaVector::Register(lua_State *L)
{
	LUA_DEBUG_START(L);

	LuaMetaType<vector3d> metaType(LuaVector::TypeName);

	metaType.CreateMetaType(L);
	metaType.StartRecording()
		.AddNewCtor(&l_vector_new)
		.AddCallCtor(&l_vector_call)
		.AddFunction("normalized", &vector3d::NormalizedSafe)
		.AddFunction("normalised", &vector3d::NormalizedSafe)
		.AddFunction("lengthSqr", &vector3d::LengthSqr)
		.AddFunction("length", &vector3d::Length)
		.AddFunction("cross", &vector3d::Cross)
		.AddFunction("dot", &vector3d::Dot)
		.StopRecording();

	// set the meta functions
	metaType.GetMetatable();
	luaL_setfuncs(L, l_vector_meta, 0);
	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");

	lua_getfield(L, -1, "methods");
	lua_setglobal(L, LuaVector::LibName);
	lua_pop(L, 1);

	LuaObjectBase::RegisterSerializer(LuaVector::TypeName, { _serialize_vector3, _deserialize_vector3 });

	LUA_DEBUG_END(L, 0);
}

vector3d *LuaVector::PushNewToLua(lua_State *L)
{
	vector3d *ptr = static_cast<vector3d *>(lua_newuserdata(L, sizeof(vector3d)));
	LuaMetaTypeBase::GetMetatableFromName(L, LuaVector::TypeName);
	lua_setmetatable(L, -2);
	return ptr;
}

const vector3d *LuaVector::GetFromLua(lua_State *L, int idx)
{
	return static_cast<vector3d *>(LuaMetaTypeBase::TestUserdata(L, idx, LuaVector::TypeName));
}

vector3d *LuaVector::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<vector3d *>(LuaMetaTypeBase::CheckUserdata(L, idx, LuaVector::TypeName));
}
