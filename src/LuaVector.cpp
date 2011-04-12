#include "LuaObject.h"
#include "LuaUtils.h"

int l_vector_new(lua_State *l)
{
	int x = luaL_checkinteger(l, 1);
	int y = luaL_checkinteger(l, 2);
	int z = luaL_checkinteger(l, 3);

	LuaVector::PushToLuaGC(new vector3d(x,y,z));
	return 1;
}

int l_vector_dot(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	vector3d *v2 = LuaVector::GetFromLua(2);
	lua_pushnumber(l, v1->Dot(*v2));
	return 1;
}

int l_vector_cross(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	vector3d *v2 = LuaVector::GetFromLua(2);
	LuaVector::PushToLuaGC(new vector3d(v1->Cross(*v2)));
	return 1;
}

int l_vector_norm(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	LuaVector::PushToLuaGC(new vector3d(v->Normalized()));
	return 1;
}

int l_vector_length(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	lua_pushnumber(l, v->Length());
	return 1;
}

int l_vector_get_x(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	lua_pushnumber(l, v->x);
	return 1;
}

int l_vector_get_y(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	lua_pushnumber(l, v->y);
	return 1;
}

int l_vector_get_z(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	lua_pushnumber(l, v->z);
	return 1;
}

int l_vector_meta_tostring(lua_State *l)
{
	vector3d *v = LuaVector::GetFromLua(1);
	LuaString::PushToLua(stringf(64, "v(%f,%f,%f)", v->x, v->y, v->z).c_str());
	return 1;
}

int l_vector_meta_add(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	vector3d *v2 = LuaVector::GetFromLua(2);
	LuaVector::PushToLuaGC(new vector3d(*v1 + *v2));
	return 1;
}

int l_vector_meta_sub(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	vector3d *v2 = LuaVector::GetFromLua(2);
	LuaVector::PushToLuaGC(new vector3d(*v1 - *v2));
	return 1;
}

int l_vector_meta_mul(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	double d = luaL_checknumber(l, 2);
	LuaVector::PushToLuaGC(new vector3d((*v1) * d));
	return 1;
}

int l_vector_meta_div(lua_State *l)
{
	vector3d *v1 = LuaVector::GetFromLua(1);
	double d = luaL_checknumber(l, 2);
	LuaVector::PushToLuaGC(new vector3d((*v1) / d));
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<vector3d> >::s_type = "Vector";

template <> void LuaObject<LuaUncopyable<vector3d> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New",    l_vector_new    },
		{ "Dot",    l_vector_dot    },
		{ "Cross",  l_vector_cross  },
		{ "Norm",   l_vector_norm   },
		{ "Length", l_vector_length },
		{ "GetX",   l_vector_get_x  },
		{ "GetY",   l_vector_get_y  },
		{ "GetZ",   l_vector_get_z  },
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ "__tostring", l_vector_meta_tostring },
		{ "__add",      l_vector_meta_add      },
		{ "__sub",      l_vector_meta_sub      },
		{ "__mul",      l_vector_meta_mul      },
		{ "__div",      l_vector_meta_div      },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_meta);
}
