#  Process this file with automake to produce Makefile.in

noinst_LIBRARIES = liboolua.a
liboolua_a_SOURCES = class_from_stack.cpp  lua_stack_dump.cpp  oolua_check_result.cpp  oolua_error.cpp \
	lua_function.cpp      lua_table.cpp       oolua.cpp               oolua_storage.cpp

include_HEADERS = base_checker.h         fwd_push_pull.h   lvd_types.h           oolua_member_function.h   oolua_version.h          type_list.h \
	class_from_stack.h     lua_function.h    member_func_helper.h  oolua_parameter_helper.h  param_traits.h           typelist_structs.h \
	class_public_member.h  lua_includes.h    oolua_char_arrays.h   oolua_push_pull.h         platform_check.h \
	cpp_constructor.h      lua_operator.h    oolua_check_result.h  oolua_registration.h      proxy_class.h \
	cpp_member_func.h      lua_ref.h         oolua_error.h         oolua_storage.h           \
	determin_qualifier.h   lua_stack_dump.h  oolua_exception.h     oolua_typedefs.h          push_pointer_internal.h \
	export_func_to_lua.h   lua_table.h       oolua.h               oolua_userdata.h          static_assert.h \
	oolua_c_func.h oolua_paramater_macros.h proxy_caller.h

INCLUDES = -I@top_srcdir@/src/lua
