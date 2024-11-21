
list(APPEND RENDER_SRC_FOLDERS
	${CMAKE_CURRENT_SOURCE_DIR}
	generators/
)

# Creates variables RENDER_CXX_FILES and RENDER_HXX_FILES
add_source_folders(RENDER RENDER_SRC_FOLDERS)

# Creates a library, adds it to the build, and sets C++ target properties on it
define_pioneer_library(pioneer-render RENDER_CXX_FILES RENDER_HXX_FILES)
target_link_libraries(pioneer-render PUBLIC
	pioneer-core
	pioneer-lib)
