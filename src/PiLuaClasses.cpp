#include "PiLuaClasses.h"

EXPORT_OOLUA_FUNCTIONS_1_CONST(pi_fixed, tonumber)
EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(pi_fixed)

EXPORT_OOLUA_FUNCTIONS_8_CONST(pi_vector, x, y, z, norm, dot, cross, len, print)
EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(pi_vector)

EXPORT_OOLUA_FUNCTIONS_2_CONST(pi_matrix, inverse, print)
EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(pi_matrix)

namespace static_matrix {
	static int identity(lua_State *l) {
		OOLUA_C_FUNCTION_0(pi_matrix, pi_matrix::identity) 
	}
	static int rotation(lua_State *l) {
		OOLUA_C_FUNCTION_2(pi_matrix, pi_matrix::rotation, float, const pi_vector&) 
	}
	static int translation(lua_State *l) {
		OOLUA_C_FUNCTION_1(pi_matrix, pi_matrix::translation, const pi_vector&) 
	}
	static int scale(lua_State *l) {
		OOLUA_C_FUNCTION_1(pi_matrix, pi_matrix::scale, const pi_vector&) 
	}
	static int orient(lua_State *l) {
		OOLUA_C_FUNCTION_3(pi_matrix, pi_matrix::orient, const pi_vector&, const pi_vector&, const pi_vector&) 
	}
}

void PiLuaClasses::RegisterClasses(lua_State *l)
{
	OOLUA::setup_user_lua_state(l);

	OOLUA::register_class<pi_vector>(l);
	OOLUA::register_class<pi_fixed>(l);
	OOLUA::register_class<pi_matrix>(l);

	OOLUA::register_class_static<pi_matrix>(l, "identity",    &static_matrix::identity);
	OOLUA::register_class_static<pi_matrix>(l, "rotation",    &static_matrix::rotation);
	OOLUA::register_class_static<pi_matrix>(l, "translation", &static_matrix::translation);
	OOLUA::register_class_static<pi_matrix>(l, "scale",       &static_matrix::scale);
	OOLUA::register_class_static<pi_matrix>(l, "orient",      &static_matrix::orient);
}
