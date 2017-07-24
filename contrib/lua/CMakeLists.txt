project(lua LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)

file(GLOB SRC_FILES *.c)

add_definitions(-DLUA_USE_APICHECK)
if (NOT WIN32)
	add_definitions(-DLUA_USE_POSIX)
endif (NOT WIN32)

add_library(${PROJECT_NAME} ${SRC_FILES})
