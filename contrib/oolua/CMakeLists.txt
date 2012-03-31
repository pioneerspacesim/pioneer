set(TARGETNAME "OOLUA")		# In capital letters to stick with Lua

INCLUDE_DIRECTORIES("../lua")		# OOLua directly includes "lua.h" instead of "lua/lua.h"

add_library(${TARGETNAME} STATIC
	class_from_stack.cpp
	lua_function.cpp
	lua_stack_dump.cpp
	lua_table.cpp
	oolua.cpp
	oolua_check_result.cpp
	oolua_error.cpp
	oolua_push_pull.cpp
	oolua_storage.cpp
	push_pointer_internal.cpp

	base_checker.h
	class_from_stack.h
	class_public_member.h
	cpp_constructor.h
	cpp_member_func.h
	determin_qualifier.h
	export_func_to_lua.h
	fwd_push_pull.h
	lua_function.h
	lua_includes.h
	lua_operator.h
	lua_ref.h
	lua_stack_dump.h
	lua_table.h
	lvd_types.h
	member_func_helper.h
	oolua.h
	oolua_char_arrays.h
	oolua_check_result.h
	oolua_config.h
	oolua_c_func.h
	oolua_error.h
	oolua_exception.h
	oolua_member_function.h
	oolua_paramater_macros.h
	oolua_parameter_helper.h
	oolua_push_pull.h
	oolua_registration.h
	oolua_storage.h
	oolua_typedefs.h
	oolua_userdata.h
	oolua_va_args.h
	oolua_version.h
	param_traits.h
	platform_check.h
	proxy_caller.h
	proxy_class.h
	push_pointer_internal.h
	typelist_structs.h
	type_list.h
)

target_link_libraries(${TARGETNAME} LUA)

