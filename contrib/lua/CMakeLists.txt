project(lua LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)

file(GLOB SRC_FILES src/*.c)

add_definitions(-DLUA_USE_APICHECK)
if (NOT WIN32)
	add_definitions(-DLUA_USE_POSIX)
endif (NOT WIN32)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

# make debug configurations reasonably fast
if (NOT MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-g -O2>)
endif()
