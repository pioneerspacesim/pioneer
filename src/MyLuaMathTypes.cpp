#include "libs.h"
#include "MyLuaMathTypes.h"
#include "LuaUtils.h"


namespace MyLuaMatrix {
	matrix4x4f *checkMatrix (lua_State *L, int index)
	{
		matrix4x4f *v;
		luaL_checktype(L, index, LUA_TUSERDATA);
		v = static_cast<matrix4x4f *>(luaL_checkudata(L, index, MYLUA_MATRIX));
		if (v == NULL) luaL_typerror(L, index, MYLUA_MATRIX);
		return v;
	}


	matrix4x4f *pushMatrix(lua_State *L)
	{
		matrix4x4f *v = static_cast<matrix4x4f *>(lua_newuserdata(L, sizeof(matrix4x4f)));
		luaL_getmetatable(L, MYLUA_MATRIX);
		lua_setmetatable(L, -2);
		return v;
	}

	int Matrix_new_identity(lua_State *L)
	{
		matrix4x4f *v = pushMatrix(L);
		*v = matrix4x4f::Identity();
		return 1;
	}

	static int Matrix_inverse(lua_State *L)
	{
		matrix4x4f *v = checkMatrix(L, 1);
		matrix4x4f *w = pushMatrix(L);
		*w = v->InverseOf();
		return 1;
	}

	static int Matrix_rotate(lua_State *L)
	{
		float ang = luaL_checknumber(L, 1);
		vector3f v = *MyLuaVec::checkVec(L, 2);
		v = v.Normalized();
		matrix4x4f *out = pushMatrix(L);
		*out = matrix4x4f::RotateMatrix(ang, v.x, v.y, v.z);
		return 1;
	}

	static int Matrix_translate(lua_State *L)
	{
		const vector3f *v = MyLuaVec::checkVec(L, 1);
		matrix4x4f *out = pushMatrix(L);
		*out = matrix4x4f::Translation(v->x, v->y, v->z);
		return 1;
	}

	static int Matrix_orient(lua_State *L)
	{
		const vector3f *pos = MyLuaVec::checkVec(L, 1);
		const vector3f *_xaxis = MyLuaVec::checkVec(L, 2);
		const vector3f *_yaxis = MyLuaVec::checkVec(L, 3);

		vector3f zaxis = _xaxis->Cross(*_yaxis).Normalized();
		vector3f xaxis = _yaxis->Cross(zaxis).Normalized();
		vector3f yaxis = zaxis.Cross(xaxis);

		matrix4x4f *out = pushMatrix(L);
		*out = matrix4x4f::MakeInvRotMatrix(xaxis, yaxis, zaxis);
		(*out)[12] = pos->x;
		(*out)[13] = pos->y;
		(*out)[14] = pos->z;
		return 1;
	}

	static int Matrix_scale(lua_State *L)
	{
		const vector3f *v = MyLuaVec::checkVec(L, 1);
		matrix4x4f *out = pushMatrix(L);
		*out = matrix4x4f::ScaleMatrix(v->x, v->y, v->z);
		return 1;
	}

	static int Matrix_new(lua_State *L)
	{
		int n = lua_gettop(L);
		matrix4x4f *v = pushMatrix(L);
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

	static int Matrix_print(lua_State *L)
	{
		matrix4x4f *v = checkMatrix(L, 1);
		printf("[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n[%f,%f,%f,%f]\n\n",
				(*v)[0], (*v)[1], (*v)[2], (*v)[3],
				(*v)[4], (*v)[5], (*v)[6], (*v)[7],
				(*v)[8], (*v)[9], (*v)[10], (*v)[11],
				(*v)[12], (*v)[13], (*v)[14], (*v)[15]);
		return 0;
	}

	static int Matrix_add (lua_State *L)
	{
		matrix4x4f *v1 = checkMatrix(L, 1);
		matrix4x4f *v2 = checkMatrix(L, 2);
		matrix4x4f *sum = pushMatrix(L);
		*sum = (*v1) + (*v2);
		return 1;
	}

	static int Matrix_sub (lua_State *L)
	{
		matrix4x4f *v1 = checkMatrix(L, 1);
		matrix4x4f *v2 = checkMatrix(L, 2);
		matrix4x4f *sum = pushMatrix(L);
		*sum = (*v1) - (*v2);
		return 1;
	}

	static int Matrix_mul (lua_State *L)
	{
		matrix4x4f *m;
		vector3f *v;
		float num;
		if (lua_isnumber(L,1)) {
			// number * mat4x4
			num = lua_tonumber(L, 1);
			m = checkMatrix(L, 2);

			matrix4x4f *out = pushMatrix(L);
			*out = num * (*m);
			return 1;
		} else if (lua_isnumber(L, 2)) {
			// mat4x4 * number
			m = checkMatrix(L, 1);
			num = lua_tonumber(L, 2);

			matrix4x4f *out = pushMatrix(L);
			*out = num * (*m);
			return 1;
		} else {
			m = checkMatrix(L, 1);
			luaL_checktype(L, 2, LUA_TUSERDATA);

			void *p = lua_touserdata(L, 2);
			assert(p);

			LUA_DEBUG_START(L);

			lua_getmetatable(L, 2);
			assert(lua_istable(L, -1));

			lua_getfield(L, LUA_REGISTRYINDEX, MYLUA_VEC);
			bool vector = lua_rawequal(L, -1, -2);
			lua_pop(L, 2);

			if (vector) {
				v = static_cast<vector3f*>(p);
				// mat4x4 * vec
				vector3f *out = MyLuaVec::pushVec(L);
				*out = (*m) * (*v);
			} else {
				// mat4x4 * mat4x4
				matrix4x4f *m2 = static_cast<matrix4x4f*>(luaL_checkudata(L, 2, MYLUA_MATRIX));
				if (!m2) luaL_typerror(L, 2, MYLUA_MATRIX);
				matrix4x4f *out = pushMatrix(L);
				*out = (*m) * (*m2);
			}

			LUA_DEBUG_END(L, 1);

			return 1;
		}
	}

	static const luaL_reg Matrix_methods[] = {
		{ "new", Matrix_new },
		{ "identity", Matrix_new_identity },
		{ "inverse", Matrix_inverse },
		{ "rotate", Matrix_rotate },
		{ "scale", Matrix_scale },
		{ "translate", Matrix_translate },
		{ "orient", Matrix_orient },
		{ "print", Matrix_print },
		{ 0, 0 }
	};

	static const luaL_reg Matrix_meta[] = {
		//  {"__gc",       Foo_gc},
		//  {"__tostring", Foo_tostring},
		{"__add",      Matrix_add},
		{"__sub",      Matrix_sub},
		{"__mul",      Matrix_mul},
		{0, 0}
	};

	int Matrix_register (lua_State *L)
	{
		luaL_openlib(L, MYLUA_MATRIX, Matrix_methods, 0);  /* create methods table,
						    add it to the globals */
		luaL_newmetatable(L, MYLUA_MATRIX);          /* create metatable for Matrix,
						 and add it to the Lua registry */
		luaL_openlib(L, 0, Matrix_meta, 0);    /* fill metatable */
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
		v = static_cast<vector3f *>(luaL_checkudata(L, index, MYLUA_VEC));
		if (v == NULL) luaL_typerror(L, index, MYLUA_VEC);
		return v;
	}


	vector3f *pushVec(lua_State *L)
	{
		vector3f *v = static_cast<vector3f *>(lua_newuserdata(L, sizeof(vector3f)));
		luaL_getmetatable(L, MYLUA_VEC);
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
		*out = v1->Cross(*v2);
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
		lua_pushnumber(L, v1->Dot(*v2));
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
		luaL_openlib(L, MYLUA_VEC, Vec_methods, 0);  /* create methods table,
						    add it to the globals */
		luaL_newmetatable(L, MYLUA_VEC);          /* create metatable for Vec,
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

