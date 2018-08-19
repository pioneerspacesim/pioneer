project(glew LANGUAGES C)

add_library(${PROJECT_NAME} STATIC glew.c)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DGLEW_STATIC)
target_include_directories(${PROJECT_NAME}
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/GL"
	INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
