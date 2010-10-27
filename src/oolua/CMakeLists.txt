include_directories(../lua)
set(SOURCES_OOLUA
	class_from_stack.cpp  lua_stack_dump.cpp  oolua_check_result.cpp  oolua_error.cpp      oolua_storage.cpp
	lua_function.cpp      lua_table.cpp       oolua.cpp               oolua_push_pull.cpp  push_pointer_internal.cpp
)

add_library(oolua STATIC ${SOURCES_OOLUA})
set_target_properties(oolua PROPERTIES OUTPUT_NAME_DEBUG oolua-dbg)
