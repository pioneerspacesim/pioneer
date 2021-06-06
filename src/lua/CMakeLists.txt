
list(APPEND LUA_SRC_FOLDERS
	${CMAKE_CURRENT_SOURCE_DIR}
	core
)

# Creates variables LUA_CXX_FILES and LUA_HXX_FILES
add_source_folders(LUA LUA_SRC_FOLDERS)

# Creates a library, adds it to the build, and sets C++ target properties on it
define_pioneer_library(pioneer-lua LUA_CXX_FILES LUA_HXX_FILES)
target_include_directories(pioneer-lua PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(pioneer-lua pioneer-lib ${LUA_LIBRARIES})
