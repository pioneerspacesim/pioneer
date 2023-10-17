
list(APPEND EDITOR_SRC_FOLDERS
	${CMAKE_CURRENT_SOURCE_DIR}
	mfd/
	system/
)

# Creates variables EDITOR_CXX_FILES and EDITOR_HXX_FILES
add_source_folders(EDITOR EDITOR_SRC_FOLDERS)

list(REMOVE_ITEM EDITOR_CXX_FILES
	editormain.cpp
)

# Creates a library, adds it to the build, and sets C++ target properties on it
define_pioneer_library(pioneer-editor EDITOR_CXX_FILES EDITOR_HXX_FILES)
target_include_directories(pioneer-editor PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(pioneer-editor LINK_PRIVATE
	pioneer-core
	pioneer-lib
)

list(APPEND EDITOR_LIBRARIES
	pioneer-editor
	pioneer-core
)

add_executable(editor WIN32 editormain.cpp ${RESOURCES})
set_cxx_properties(editor)
target_link_libraries(editor LINK_PRIVATE ${EDITOR_LIBRARIES} ${pioneerLibs} ${winLibs})
set_target_properties(editor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
