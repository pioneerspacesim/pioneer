
list(APPEND CORE_SRC_FOLDERS
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Creates variables CORE_CXX_FILES and CORE_HXX_FILES
add_source_folders(CORE CORE_SRC_FOLDERS)

# Creates a library, adds it to the build, and sets C++ target properties on it
define_pioneer_library(pioneer-core CORE_CXX_FILES CORE_HXX_FILES)
target_include_directories(pioneer-core PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(pioneer-core PUBLIC
	${SDL2_LIBRARIES}
	${SIGCPP_LIBRARIES}
	profiler
	lz4
	fmt::fmt
	nanosockets)
