cmake_minimum_required(VERSION 3.4)
project(pioneer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# If both libGL.so and libOpenGL.so are found, default to the latter
# (former is a legacy name).
# Set OpenGL_GL_PREFERENCE=LEGACY to force it to use the former.
if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

set(IS_TRAVIS $ENV{TRAVIS})

if (IS_TRAVIS)
    set(CMAKE_COLOR_MAKEFILE false)
endif()

include(GNUInstallDirs)

if (CMAKE_COMPILER_IS_GNUCXX)
    if (NOT IS_TRAVIS)
        add_compile_options(-fdiagnostics-color)
    endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif (CMAKE_COMPILER_IS_GNUCXX)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

# Get the GIT hash of the latest commit
include(FindGit OPTIONAL)
if (GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE PROJECT_VERSION_GIT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	add_definitions(-DPIONEER_EXTRAVERSION="${PROJECT_VERSION_GIT}")
endif()

string(TIMESTAMP PROJECT_VERSION "%Y%m%d")
add_definitions(-DPIONEER_VERSION="${PROJECT_VERSION}")

if (NOT PIONEER_DATA_DIR)
	file(TO_NATIVE_PATH ${CMAKE_INSTALL_FULL_DATADIR}/pioneer/data PIONEER_DATA_DIR)
endif(NOT PIONEER_DATA_DIR)
add_definitions(-DPIONEER_DATA_DIR="${PIONEER_DATA_DIR}")

if (MINGW)
	# Enable PRIxYY macros on MinGW
	add_definitions(-D__STDC_FORMAT_MACROS)
endif (MINGW)

list(APPEND SRC_FOLDERS
	src
	src/collider
	src/galaxy
	src/gameui
	src/graphics
	src/graphics/dummy
	src/graphics/opengl
	src/gui
	src/scenegraph
	src/terrain
	src/text
	src/ui
)

foreach (each IN LISTS SRC_FOLDERS)
	file(GLOB src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${each}/*.cpp)
	list(APPEND CXX_FILES ${src_files})
endforeach (each IN LISTS SRC_FOLDERS)

list(REMOVE_ITEM CXX_FILES
	src/main.cpp
	src/modelcompiler.cpp
	src/savegamedump.cpp
	src/tests.cpp
	src/textstress.cpp
	src/uitest.cpp
)

list(APPEND FILESYSTEM_CXX_FILES)

if (WIN32)
	list(APPEND FILESYSTEM_CXX_FILES
		src/win32/FileSystemWin32.cpp
		src/win32/OSWin32.cpp
		src/win32/TextUtils.cpp
	)
else (WIN32)
	list(APPEND FILESYSTEM_CXX_FILES
		src/posix/FileSystemPosix.cpp
		src/posix/OSPosix.cpp
	)
endif (WIN32)

LIST(APPEND CXX_FILES ${FILESYSTEM_CXX_FILES})
option(USE_PIONEER_THIRDPARTY "Use pioneer's thirdparty library repository." OFF)
if (USE_PIONEER_THIRDPARTY)
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr)
	include_directories(${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr/include)
	link_directories(${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr/lib)
    find_package(Threads)
endif()

option(USE_SYSTEM_LIBGLEW "Use the system's libglew" OFF)
if (USE_SYSTEM_LIBGLEW)
	add_library(GLEW::GLEW INTERFACE IMPORTED)
	find_package(GLEW REQUIRED)
endif (USE_SYSTEM_LIBGLEW)

option(USE_SYSTEM_LIBLUA "Use the system's liblua" OFF)
if (USE_SYSTEM_LIBLUA)
	find_package(Lua 5.2 EXACT REQUIRED)
	include_directories(${LUA_INCLUDE_DIR})
	if (WIN32)
		add_definitions(-DLUA_BUILD_AS_DLL)
	endif (WIN32)
endif (USE_SYSTEM_LIBLUA)

find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PNG REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

pkg_check_modules(ASSIMP REQUIRED assimp)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
pkg_check_modules(VORBISFILE REQUIRED vorbisfile)

include_directories(
	${CMAKE_SOURCE_DIR}/contrib
	${CMAKE_SOURCE_DIR}/src
	${ASSIMP_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SIGCPP_INCLUDE_DIRS}
	${VORBISFILE_INCLUDE_DIRS}
)

if (NOT USE_SYSTEM_LIBGLEW)
	add_subdirectory(contrib/glew)
	add_library(GLEW::GLEW ALIAS glew)
endif (NOT USE_SYSTEM_LIBGLEW)
add_subdirectory(contrib/imgui)
add_subdirectory(contrib/jenkins)
add_subdirectory(contrib/PicoDDS)
add_subdirectory(contrib/profiler)
if (NOT USE_SYSTEM_LIBLUA)
	add_subdirectory(contrib/lua)
	set(LUA_LIBRARIES lua)
	include_directories(contrib/lua)
endif (NOT USE_SYSTEM_LIBLUA)

add_library(pioneerLib STATIC ${CXX_FILES})

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)
add_executable(modelcompiler WIN32 src/modelcompiler.cpp)
add_executable(savegamedump WIN32
	src/savegamedump.cpp
	src/JsonUtils.cpp
	src/FileSystem.cpp
	src/utils.cpp
	src/StringF.cpp
	src/GZipFormat.cpp
	src/DateTime.cpp
	src/Lang.cpp
	${FILESYSTEM_CXX_FILES}
	# This one's in here because there's no such thing as separation in the codebase.
	src/PngWriter.cpp
)

if (USE_PIONEER_THIRDPARTY)
    set(SDL2_LIBRARIES "${SDL2_STATIC_LIBRARIES}")
    set(SDL2_IMAGE_LIBRARIES "${SDL2_IMAGE_STATIC_LIBRARIES}")
endif()

list(APPEND pioneerLibs
	pioneerLib
	${ASSIMP_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES}
	${SDL2_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${SIGCPP_LIBRARIES}
	${VORBISFILE_LIBRARIES}
	${LUA_LIBRARIES}
	GLEW::GLEW
	imgui
	jenkins
	picodds
	profiler
)

if(USE_PIONEER_THIRDPARTY)
    list(APPEND pioneerLibs Threads::Threads)
endif()

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${pioneerLibs})
target_link_libraries(modelcompiler LINK_PRIVATE ${pioneerLibs})
target_link_libraries(savegamedump LINK_PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

if (WIN32)
	target_link_libraries(${PROJECT_NAME} modelcompiler LINK_PRIVATE shlwapi)
endif (WIN32)

set_target_properties(${PROJECT_NAME} modelcompiler savegamedump pioneerLib PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS ON
)

install(TARGETS ${PROJECT_NAME} modelcompiler savegamedump
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY data
	DESTINATION ${CMAKE_INSTALL_DATADIR}/pioneer
	PATTERN "listdata.*" EXCLUDE
	PATTERN "Makefile.am" EXCLUDE
)
