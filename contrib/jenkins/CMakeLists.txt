project(jenkins LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)

add_library(${PROJECT_NAME} lookup3.c)
