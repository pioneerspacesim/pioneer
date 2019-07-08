project(profiler LANGUAGES CXX)

set(BUILD_SHARED_LIBS OFF)

include_directories(${CMAKE_BINARY_DIR})

add_library(${PROJECT_NAME} Profiler.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
