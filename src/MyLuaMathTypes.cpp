#include "MyLuaMathTypes.h"

// Copy of:
// LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname)
// with typeerror commented out
void *mylua_checkudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return p;
      }
    }
  }
//  luaL_typerror(L, ud, tname);  /* else error */
  return NULL;  /* to avoid warnings */
}


namespace MyLuaMatrix {
	matrix4x4f *checkMat4x4 (lua_State *L, int index)
	{
		matrix4x4f *v;
		luaL_checktype(L, index, LUA_TUSERDATA);
		v = (matrix4x4f *)luaL_checkudata(L, index, MAT4X4);
		if (v == NULL) luaL_typerror(L, index, MAT4X4);
		return v;
	}


	matrix4x4f *pushMat4x4(lua_State *L)
	{
		matrix4x4f *v = (matrix4x4f *)lua_newuserdata(L, sizeof(matrix4x4f));
		luaL_getmetatable(L, MAT4X4);
		lua_setmetatable(L, -2);
		return v;
	}

	int Mat4x4_new_identity(lua_State *L)
	{
		matrix4x4f *v = pushMat4x4(L);
		*v = matrix4x4f::Identity();
		return 1;
	}

	static int Mat4x4_inverse(lua_State *L)
	{
		matrix4x4f *v = checkMat4x4(L, 1);
		matrix4x4f *w = pushMat4x4(L);
		*w = v->InverseOf();
		return 1;
	}

	static int Mat4x4_rotate(lua_State *L)
	{
		float ang = luaL_checknumber(L, 1);
		vector3f v = *MyLuaVec::checkVec(L, 2);
		v = v.Normalized();
		matrix4x4f *out = pushMat4x4(L);
		*out = matrix4x4f::RotateMatrix(ang, v.x, v.y, v.z);
		return 1;
	}

	static int Mat4x4_translate(lua_State *L)
	{
		const vector3f *v = MyLuaVec::checkVec(L, 1);
		matrix4x4f *out = pushMat4x4(L);
		*out = matrix4x4f::Translation(v->x, v->y, v->z);
		return 1;
	}

	static int Mat4x4_scale(lua_State *L)
	{
		const vector3f *v = MyLuaVec::checkVec(L, 1);
		matrix4x4f *out = pushMat4x4(L);
		*out = matrix4x4f::ScaleMatrix(v->x, v->y, v->z);
		return 1;
	}

	static int Mat4x4_new(lua_State *L)
	{
		int n = lua_gettop(L);
		matrix4x4f *v = pushMat4x4(L);
		if (n == 0) {
			*v = matrix4x4f(0.0);
		} else if (n == 3) {
			vector3f *v1 = MyLuaVec::checkVec(L, 1);
			vector3f *v2 = MyLuaVec::checkVec(L, 2);
			vector3f *v3 = MyLuaVec::checkVec(L, 3);
			*v = matrix4x4f::MakeRotMatrix(*v1, *v2, *v3);
		} else if (n == 4) {
			vector3f *v1 = MyLuaVec::checkVec(L, 1);
			vector3f *v2 = MyLuaVec::checkVec(L, 2);
			vector3f *v3 = MyLuaVec::checkVec(L, 3);
			vector3f *v4 = MyLuaVec::checkVec(L, 4);
			*v = matrix4x4f::MakeRotMatrix(*v1, *v2, *v3);
			(*v)[12] = v4->x;
			(*v)[13] = v4->y;
			(*v)[14] = v4->z;
		} else if (n == 16) {
			for (int i=0; i<16; i++) {
				(*v)[i] = luaL_checknumber(L, i+1);
			}
		} else {
			luaL_error(L, "bad arguments to mat4x4:new()");
		}
		return 1;
	}

	static int Mat4x4_print(lua_State *L)
	{
		matrix4x4f *v = checkMat4x4(L, 1);
		printf("[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n\n",
				(*v)[0], (*v)[1], (*v)[2], (*v)[3],
				(*v)[4], (*v)[5], (*v)[6], (*v)[7],
				(*v)[8], (*v)[9], (*v)[10], (*v)[11],
				(*v)[12], (*v)[13], (*v)[14], (*v)[15]);
		return 0;
	}

	static int Mat4x4_add (lua_State *L)
	{
		matrix4x4f *v1 = checkMat4x4(L, 1);
		matrix4x4f *v2 = checkMat4x4(L, 2);
		matrix4x4f *sum = pushMat4x4(L);
		*sum = (*v1) + (*v2);
		return 1;
	}

	static int Mat4x4_sub (lua_State *L)
	{
		matrix4x4f *v1 = checkMat4x4(L, 1);
		matrix4x4f *v2 = checkMat4x4(L, 2);
		matrix4x4f *sum = pushMat4x4(L);
		*sum = (*v1) - (*v2);
		return 1;
	}

	static int Mat4x4_mul (lua_State *L)
	{
		matrix4x4f *m;
		vector3f *v;
		float num;
		if (lua_isnumber(L,1)) {
			// number * mat4x4
			num = lua_tonumber(L, 1);
			m = checkMat4x4(L, 2);

			matrix4x4f *out = pushMat4x4(L);
			*out = num * (*m);
			return 1;
		} else if (lua_isnumber(L, 2)) {
			// mat4x4 * number
			m = checkMat4x4(L, 1);
			num = lua_tonumber(L, 2);

			matrix4x4f *out = pushMat4x4(L);
			*out = num * (*m);
			return 1;
		} else {
			m = checkMat4x4(L, 1);
			luaL_checktype(L, 2, LUA_TUSERDATA);
			v = (vector3f*)mylua_checkudata(L, 2, VEC);
			if (v) {
				// mat4x4 * vec
				vector3f *out = MyLuaVec::pushVec(L);
				*out = (*m) * (*v);
				return 1;
			} else {
				// mat4x4 * mat4x4
				matrix4x4f *m2 = (matrix4x4f*)luaL_checkudata(L, 2, MAT4X4);
				if (!m2) luaL_typerror(L, 2, MAT4X4);
				matrix4x4f *out = pushMat4x4(L);
				*out = (*m) * (*m2);
				return 1;
			}
		}
	}

	static const luaL_reg Mat4x4_methods[] = {
		{ "new", Mat4x4_new },
		{ "identity", Mat4x4_new_identity },
		{ "inverse", Mat4x4_inverse },
		{ "rotate", Mat4x4_rotate },
		{ "scale", Mat4x4_scale },
		{ "translate", Mat4x4_translate },
		{ "print", Mat4x4_print },
		{ 0, 0 }
	};

	static const luaL_reg Mat4x4_meta[] = {
		//  {"__gc",       Foo_gc},
		//  {"__tostring", Foo_tostring},
		{"__add",      Mat4x4_add},
		{"__sub",      Mat4x4_sub},
		{"__mul",      Mat4x4_mul},
		{0, 0}
	};

	int Mat4x4_register (lua_State *L)
	{
		luaL_openlib(L, MAT4X4, Mat4x4_methods, 0);  /* create methods table,
						    add it to the globals */
		luaL_newmetatable(L, MAT4X4);          /* create metatable for Mat4x4,
						 and add it to the Lua registry */
		luaL_openlib(L, 0, Mat4x4_meta, 0);    /* fill metatable */
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
} /* namespace MyLuaMatrix */

namespace MyLuaVec {
	vector3f *checkVec (lua_State *L, int index)
	{
		vector3f *v;
		luaL_checktype(L, index, LUA_TUSERDATA);
		v = (vector3f *)luaL_checkudata(L, index, VEC);
		if (v == NULL) luaL_typerror(L, index, VEC);
		return v;
	}


	vector3f *pushVec(lua_State *L)
	{
		vector3f *v = (vector3f *)lua_newuserdata(L, sizeof(vector3f));
		luaL_getmetatable(L, VEC);
		lua_setmetatable(L, -2);
		return v;
	}


	int Vec_new(lua_State *L)
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

	int Vec_newNormalized(lua_State *L)
	{
		float x = lua_tonumber(L, 1);
		float y = lua_tonumber(L, 2);
		float z = lua_tonumber(L, 3);
		vector3f *v = pushVec(L);
		v->x = x;
		v->y = y;
		v->z = z;
		*v = v->Normalized();
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
		{ "newNormalized", Vec_newNormalized },
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

