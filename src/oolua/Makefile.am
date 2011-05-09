#  Process this file with automake to produce Makefile.in

noinst_LIBRARIES = liboolua.a
liboolua_a_SOURCES = class_from_stack.cpp  lua_stack_dump.cpp  oolua_check_result.cpp  oolua_error.cpp      oolua_storage.cpp \
	lua_function.cpp      lua_table.cpp       oolua.cpp               oolua_push_pull.cpp  push_pointer_internal.cpp

noinst_HEADERS = base_checker.h         fwd_push_pull.h   lvd_types.h           oolua_exception.h         oolua_storage.h   proxy_caller.h \
	class_from_stack.h     lua_function.h    member_func_helper.h  oolua.h                   oolua_typedefs.h  proxy_class.h \
	class_public_member.h  lua_includes.h    oolua_c_func.h        oolua_member_function.h   oolua_userdata.h  push_pointer_internal.h \
	cpp_constructor.h      lua_operator.h    oolua_char_arrays.h   oolua_paramater_macros.h  oolua_va_args.h   type_list.h \
	cpp_member_func.h      lua_ref.h         oolua_check_result.h  oolua_parameter_helper.h  oolua_version.h   typelist_structs.h \
	determin_qualifier.h   lua_stack_dump.h  oolua_config.h        oolua_push_pull.h         param_traits.h \
	export_func_to_lua.h   lua_table.h       oolua_error.h         oolua_registration.h      platform_check.h


INCLUDES = -I@top_srcdir@/src -I@top_srcdir@/src/lua
