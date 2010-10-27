project(pioneer)
cmake_minimum_required(VERSION 2.8)

# default to a Release build
if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
  "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# help CMake find local CMake modules
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/CMakeModules")

# help CMake find locally installed C/C++ libraries
include(GccSearchPaths)

set(INSTALL_IN_SOURCE_TREE TRUE CACHE BOOL "Output final targets to the source tree")

# for default builds, output goes into (sub-directories of) the source tree
# output executables, .so files and .a files to the bin/ and lib/ directories
if (INSTALL_IN_SOURCE_TREE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
endif()

# =============================================================================
# configure dependencies

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(Freetype REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGCXX REQUIRED "sigc++-2.0")
pkg_check_modules(OGGVORBIS REQUIRED ogg vorbis vorbisfile)

# =============================================================================
# set common build settings

add_definitions(-DLUA_USE_POSIX)
include_directories(
	${SDL_INCLUDE_DIR}
	${SIGCXX_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIRS}
	${OGGVORBIS_INCLUDE_DIRS}
)

# =============================================================================
# sub-project config

add_subdirectory(src/lua)
add_subdirectory(src/oolua)
add_subdirectory(src/collider)
add_subdirectory(src)
