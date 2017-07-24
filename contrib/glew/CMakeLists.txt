project(glew LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)
add_definitions(-DGLEW_STATIC)

add_library(${PROJECT_NAME} glew.c)
