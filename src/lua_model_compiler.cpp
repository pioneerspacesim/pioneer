#include "libs.h"

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
	static vector3d *checkVec (lua_State *L, int index)
	{
		vector3d *v;
		luaL_checktype(L, index, LUA_TUSERDATA);
		v = (vector3d *)luaL_checkudata(L, index, VEC);
		if (v == NULL) luaL_typerror(L, index, VEC);
		return v;
	}


	static vector3d *pushVec(lua_State *L)
	{
		vector3d *v = (vector3d *)lua_newuserdata(L, sizeof(vector3d));
		luaL_getmetatable(L, VEC);
		lua_setmetatable(L, -2);
		return v;
	}


	static int Vec_new(lua_State *L)
	{
		double x = lua_tonumber(L, 1);
		double y = lua_tonumber(L, 2);
		double z = lua_tonumber(L, 3);
		vector3d *v = pushVec(L);
		v->x = x;
		v->y = y;
		v->z = z;
		return 1;
	}

	static int Vec_print(lua_State *L)
	{
		vector3d *v = checkVec(L, 1);
		printf("%f,%f,%f\n", v->x, v->y, v->z);
		return 0;
	}

	static int Vec_add (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		vector3d *v2 = checkVec(L, 2);
		vector3d *sum = pushVec(L);
		*sum = (*v1) + (*v2);
		return 1;
	}

	static int Vec_sub (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		vector3d *v2 = checkVec(L, 2);
		vector3d *sum = pushVec(L);
		*sum = (*v1) - (*v2);
		return 1;
	}

	static int Vec_cross (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		vector3d *v2 = checkVec(L, 2);
		vector3d *out = pushVec(L);
		*out = vector3d::Cross(*v1, *v2);
		return 1;
	}

	static int Vec_mul (lua_State *L)
	{
		vector3d *v;
		double m;
		if (lua_isnumber(L,1)) {
			m = lua_tonumber(L, 1);
			v = checkVec(L, 2);
		} else {
			v = checkVec(L, 1);
			m = lua_tonumber(L, 2);
		}
		vector3d *out = pushVec(L);
		*out = m * (*v);
		return 1;
	}

	static int Vec_div (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		double d = lua_tonumber(L, 2);
		vector3d *out = pushVec(L);
		*out = (1.0/d) * (*v1);
		return 1;
	}

	static int Vec_norm (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		vector3d *out = pushVec(L);
		*out = (*v1).Normalized();
		return 1;
	}

	static int Vec_dot (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
		vector3d *v2 = checkVec(L, 2);
		lua_pushnumber(L, vector3d::Dot(*v1, *v2));
		return 1;
	}

	static int Vec_len (lua_State *L)
	{
		vector3d *v1 = checkVec(L, 1);
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

namespace MyLuaModel {

	class ModelBuilder {
	public:
		ModelBuilder(const std::string &name) {
			m_name = name;
			m_scale = 1.0f;
			m_radius = 1.0f;
		}

		std::string m_name;
		float m_scale;
		float m_radius;
	};

	static ModelBuilder **checkModel (lua_State *L, int index)
	{
		ModelBuilder **m;
		luaL_checktype(L, index, LUA_TUSERDATA);
		m = (ModelBuilder **)luaL_checkudata(L, index, "Model");
		if (m == NULL) luaL_typerror(L, index, "Model");
		return m;
	}


	/* Why is the userdata a ModelBuilder* rather than a ModelBuilder?
	 * Because we need to be able to call the C++ constructor, not have
	 * a chuck of memory just allocated for us... */
	static ModelBuilder **pushModel(lua_State *L)
	{
		ModelBuilder **m = (ModelBuilder **)lua_newuserdata(L, sizeof(ModelBuilder*));
		*m = 0;
		luaL_getmetatable(L, "Model");
		lua_setmetatable(L, -2);
		return m;
	}

	static int Model_new(lua_State *L)
	{
		ModelBuilder **m = pushModel(L);
		if (!lua_istable(L, 1)) {
			luaL_error(L, "Model.new takes named arguments of form {name=\"myModel\", radius=1.2, scale=2.0}");
		}
		lua_pushstring(L, "name");
		lua_gettable(L, 1);
		if (lua_isstring(L, -1)) {
			const char *model_name = lua_tostring(L, -1);
			printf("Model name %s\n", model_name);
			*m = new ModelBuilder(model_name);
			lua_pop(L, 1);
		} else {
			luaL_error(L, "Model.new must have {name=\"string\", argument");
		}

		lua_pushstring(L, "radius");
		lua_gettable(L, 1);
		if (lua_isnumber(L, -1)) (*m)->m_radius = lua_tonumber(L, -1);
		lua_pop(L, 1);
			
		lua_pushstring(L, "scale");
		lua_gettable(L, 1);
		if (lua_isnumber(L, -1)) (*m)->m_scale = lua_tonumber(L, -1);
		lua_pop(L, 1);
		printf("%s Radius %f, scale %f\n", (*m)->m_name.c_str(), (*m)->m_radius, (*m)->m_scale);

		exit(0);
		return 1;
	}

	static int Model_tostring(lua_State *L)
	{
		ModelBuilder **m = checkModel(L, 1);
		std::string s = "Model: "+(*m)->m_name;
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	static const luaL_reg Model_methods[] = {
		{ "new", Model_new },
		{ 0, 0 }
	};

	static const luaL_reg Model_meta[] = {
	//	{ "__gc",       Model_gc },
		{ "__tostring", Model_tostring },
		{0, 0}
	};

	int Model_register (lua_State *L)
	{
		luaL_openlib(L, "Model", Model_methods, 0);  /* create methods table,
						    add it to the globals */
		luaL_newmetatable(L, "Model");          /* create metatable for Model,
						 and add it to the Lua registry */
		luaL_openlib(L, 0, Model_meta, 0);    /* fill metatable */
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
} /* namespace MyLuaModel */


void LuaModelCompilerInit()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	lua_register(L, "average", average);

	MyLuaVec::Vec_register(L);
	lua_pop(L, 1); // why again?
	MyLuaModel::Model_register(L);
	lua_pop(L, 1); // why again?
	// shorthand for Vec.new(x,y,z)
	lua_register(L, "v", MyLuaVec::Vec_new);

	if (luaL_dofile(L, "models.lua")) {
		printf("%s\n", lua_tostring(L, -1));
	}
	lua_close(L);
}
