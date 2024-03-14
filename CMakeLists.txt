cmake_minimum_required(VERSION 3.13)
project(pioneer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_VERSION "20240314"
	CACHE STRING "Version identifier for current compiled build")

set(PROJECT_VERSION_INFO ""
	CACHE STRING "Additional version information (optional)")

# If both libGL.so and libOpenGL.so are found, default to the latter
# (former is a legacy name).
# Set OpenGL_GL_PREFERENCE=LEGACY to force it to use the former.
if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

include(cmake/InstallPioneer.cmake)

if (MINGW)
	# Fix build errors on AppVeyor with MinGW due to a broken GLEW config script
	list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)
endif (MINGW)

# Put the output into the root dir so it can be run from Visual Studio
if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
endif (MSVC)

if (MSVC)
	# Avoid annoying warnings from Visual Studio
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	# Use M_PI/M_E macros from math.h
	add_definitions(-D_USE_MATH_DEFINES -DHAVE_M_PI)

    # Disable warning C4506 so that src/lua/LuaObject.h: template <> void LuaObject<SystemPath>::PushToLua(const SystemPath &o);
    # doesn't spew multiple warnings

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /wd4506")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4506")

	# Disable warnings to avoid MSVC spam (moved from libs.h):
	# #pragma warning(disable : 4244) // "conversion from x to x: possible loss of data"
	# #pragma warning(disable : 4800) // int-to-bool "performance warning"
	# #pragma warning(disable : 4355) // 'this' used in base member initializer list
	# #pragma warning(disable : 4351) // new behavior [after vs2003!]: elements of array 'array' will be default initialized
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4800 /wd4355 /wd4351")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4800 /wd4355 /wd4351")

	add_definitions(-DNOMINMAX)
endif (MSVC)

if (APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu")
endif(APPLE)

option(USE_SSE42 "Compile for SSE4.2 compatible microarchitectures (enables optimizations)" ON)

if (USE_SSE42)
	if (NOT MSVC)
		add_compile_options("-msse4.2" "-mlzcnt" "-mpopcnt")
	endif()
endif (USE_SSE42)

option(USE_AVX2 "Compile for AVX2 compatible microarchitectures (Haswell and newer)" OFF)

if (USE_AVX2)
	if (MSVC)
		add_compile_options("/arch:AVX2")
	else()
		add_compile_options("-mavx2")
	endif()
endif(USE_AVX2)

option(USE_LLD_LINKER "Use the LLVM lld linker instead of gcc's linker" OFF)
if (CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(
		-fdiagnostics-color=auto
		-Wall
		-Wextra
		-Wno-unused-parameter
		-Wno-unused-but-set-parameter
		-Wno-implicit-fallthrough
	)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
	if (USE_LLD_LINKER)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld")
	endif(USE_LLD_LINKER)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
endif (CMAKE_COMPILER_IS_GNUCXX)

option(USE_TIME_TRACE "Use -ftime-trace to profile compile times (requires Clang)" OFF)
if (USE_TIME_TRACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
endif()

include(CheckSymbolExists)
check_symbol_exists(feclearexcept "fenv.h" HAS_FECLEAREXCEPT)
check_symbol_exists(feenableexcept "fenv.h" HAS_FEENABLEEXCEPT)
check_symbol_exists(fedisableexcept "fenv.h" HAS_FEDISABLEEXCEPT)
if (HAS_FECLEAREXCEPT AND HAS_FEENABLEEXCEPT AND HAS_FEDISABLEEXCEPT)
	set(HAS_FPE_OPS ON)
endif()

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

# Get the GIT hash of the latest commit
include(FindGit OPTIONAL)
if (GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE PROJECT_VERSION_GIT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if(DEFINED PROJECT_VERSION_GIT)
	string(JOIN " " PROJECT_VERSION_INFO ${PROJECT_VERSION_GIT} ${PROJECT_VERSION_INFO})
endif()

if (MINGW)
	# Enable PRIxYY macros on MinGW
	add_definitions(-D__STDC_FORMAT_MACROS)
endif (MINGW)

option(WITH_OBJECTVIEWER "Include the object viewer in the build" ON)
option(WITH_DEVKEYS "Include various extra keybindings for dev functions" ON)
option(USE_SYSTEM_LIBGLEW "Use the system's libglew" OFF)
option(USE_SYSTEM_LIBLUA "Use the system's liblua" OFF)
option(PROFILER_ENABLED "Build pioneer with profiling support built-in." OFF)
option(REMOTE_LUA_REPL "Enable remote LUA console" OFF)

if (REMOTE_LUA_REPL)
	set(REMOTE_LUA_REPL_PORT 12345 CACHE STRING "TCP port for remote LUA console")
endif (REMOTE_LUA_REPL)

list(APPEND SRC_FOLDERS
	src/
	src/collider
	src/galaxy
	src/graphics
	src/graphics/dummy
	src/graphics/opengl
	src/gui
	src/pigui
	src/scenegraph
	src/ship
	src/sound
	src/terrain
	src/text
)

macro(add_source_folders TARGET SRC_FOLDERS)
foreach (each IN LISTS ${SRC_FOLDERS})
	file(GLOB header_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${each}/*.h)
	list(APPEND ${TARGET}_HXX_FILES ${header_files})
	file(GLOB src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${each}/*.cpp)
	list(APPEND ${TARGET}_CXX_FILES ${src_files})
endforeach ()
endmacro()

add_source_folders(PIONEER SRC_FOLDERS)

list(REMOVE_ITEM PIONEER_CXX_FILES
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

configure_file(buildopts.h.cmakein buildopts.h @ONLY)

LIST(APPEND PIONEER_CXX_FILES ${FILESYSTEM_CXX_FILES})

option(USE_PIONEER_THIRDPARTY "Use pioneer's thirdparty library repository." OFF)
if (USE_PIONEER_THIRDPARTY)
	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr)
	include_directories(${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr/include)
	link_directories(${CMAKE_SOURCE_DIR}/pioneer-thirdparty/usr/lib)
	find_package(Threads)
endif()

if (USE_SYSTEM_LIBGLEW)
	add_library(GLEW::GLEW INTERFACE IMPORTED)
	find_package(GLEW REQUIRED)
endif (USE_SYSTEM_LIBGLEW)

if (USE_SYSTEM_LIBLUA)
	find_package(Lua 5.2 EXACT REQUIRED)
	include_directories(${LUA_INCLUDE_DIR})
	if (WIN32)
		add_definitions(-DLUA_BUILD_AS_DLL)
	endif (WIN32)
endif (USE_SYSTEM_LIBLUA)

if (PROFILER_ENABLED)
	add_definitions(-DPIONEER_PROFILER=1)
endif(PROFILER_ENABLED)

if (WIN32)
	add_definitions(-DPSAPI_VERSION=1)
endif (WIN32)

macro(set_cxx_properties)
	set_target_properties(${ARGN} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS ON
	)
endmacro()

macro(define_pioneer_library library_name _src _header)
	add_library(${library_name} STATIC ${${_src}} ${${_header}})
	set_cxx_properties(${library_name})
endmacro()

if (MSVC)
	include(msvc-defaults.cmake)
else (MSVC)
	find_package(PkgConfig REQUIRED)

	pkg_check_modules(SDL2 REQUIRED sdl2)
	pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

	pkg_check_modules(ASSIMP REQUIRED assimp>=5.0)
	pkg_check_modules(SIGCPP REQUIRED sigc++-2.0)
	pkg_check_modules(VORBISFILE REQUIRED vorbisfile)
endif (MSVC)

find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(contrib/lz4)
add_subdirectory(contrib/fmt)

set(NANOSOCKETS_STATIC "1")
add_subdirectory(contrib/nanosockets)

set(PIONEER_SRC ${CMAKE_SOURCE_DIR}/src)
set(PIONEER_CONTRIB ${CMAKE_SOURCE_DIR}/contrib)

include_directories(
	${PIONEER_SRC}
	${PIONEER_CONTRIB}
	${PIONEER_CONTRIB}/doctest
	${PIONEER_CONTRIB}/fmt/include
	${ASSIMP_INCLUDE_DIRS}
	${FREETYPE_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SIGCPP_INCLUDE_DIRS}
	${VORBISFILE_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
)

# We don't want/need the GLU dependency
add_definitions(-DGLEW_NO_GLU)

if (NOT USE_SYSTEM_LIBGLEW)
	add_subdirectory(contrib/glew)
	add_library(GLEW::GLEW ALIAS glew)
	include_directories(contrib/glew)

	# Specify that we compile against a static build of Glew
	# (required on Windows)
	add_definitions(-DGLEW_STATIC)
endif (NOT USE_SYSTEM_LIBGLEW)

add_subdirectory(contrib/imgui)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

add_subdirectory(contrib/jenkins)
add_subdirectory(contrib/PicoDDS)
add_subdirectory(contrib/profiler)
if (NOT USE_SYSTEM_LIBLUA)
	add_subdirectory(contrib/lua)
	set(LUA_LIBRARIES lua)
	include_directories(contrib/lua)
endif (NOT USE_SYSTEM_LIBLUA)

add_subdirectory(src/core)

define_pioneer_library(pioneer-lib PIONEER_CXX_FILES PIONEER_HXX_FILES)
target_link_libraries(pioneer-lib PUBLIC pioneer-core)

add_subdirectory(src/lua)

if (WIN32)
	string(TIMESTAMP BUILD_YEAR "%Y")
	string(TIMESTAMP BUILD_DATE "%Y%m%d")
	set(RESOURCES ${CMAKE_BINARY_DIR}/pioneer.rc)
	configure_file(pioneer.rc.cmakein ${RESOURCES} @ONLY)
endif()

link_directories(
	${ASSIMP_LIBRARY_DIRS}
	${SDL2_LIBRARY_DIRS}
	${SDL2_IMAGE_LIBRARY_DIRS}
	${SIGCPP_LIBRARY_DIRS}
	${VORBISFILE_LIBRARY_DIRS}
)

list(APPEND UNITTEST_SRC_FOLDERS
	src/test)
add_source_folders(UNITTEST UNITTEST_SRC_FOLDERS)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp ${RESOURCES})
add_executable(unittest ${UNITTEST_CXX_FILES})
add_executable(modelcompiler src/modelcompiler.cpp)
add_executable(savegamedump
	src/savegamedump.cpp
	src/JsonUtils.cpp
	src/FileSystem.cpp
	src/StringF.cpp
	src/DateTime.cpp
	src/Lang.cpp
	${FILESYSTEM_CXX_FILES}
)

find_program(NATURALDOCS NAMES naturaldocs)
if (NATURALDOCS)
	add_custom_target(codedoc
		${CMAKE_COMMAND} -E make_directory codedoc
		COMMAND naturaldocs -i src/ -i data/libs/ -xi src/data/ -o HTML codedoc/ -p nd/ -do -ro -s Default Local
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

find_program(PYTHON NAMES python)
if (PYTHON)
	add_custom_target(enums
		COMMAND scripts/scan_enums.py -o src/enum_table.cpp --pattern='*.h' -r src
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

target_link_libraries(pioneer-lib PUBLIC lz4 fmt::fmt)

list(APPEND pioneerLibs
	pioneer-core
	pioneer-lib
	pioneer-lua
	${ASSIMP_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${OPENGL_LIBRARIES}
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
	Threads::Threads
)

if (WIN32)
	list(APPEND winLibs shlwapi psapi)
endif (WIN32)

add_subdirectory(src/editor)

target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${pioneerLibs} ${winLibs})
target_link_libraries(unittest LINK_PRIVATE ${pioneerLibs} ${winLibs})
target_link_libraries(modelcompiler LINK_PRIVATE ${pioneerLibs} ${winLibs})
target_link_libraries(savegamedump LINK_PRIVATE pioneer-core ${SDL2_IMAGE_LIBRARIES} ${winLibs})

set_cxx_properties(${PROJECT_NAME} unittest modelcompiler savegamedump)

if(MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND xcopy ..\\pioneer-thirdparty\\win32\\bin\\${MSVC_ARCH}\\vs2019\\*.dll ${TargetDir}*.dll /Y /C
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "copy the dlls into the directory"
		VERBATIM
	)
endif(MSVC)

if (CMAKE_CROSSCOMPILING)
	find_program(MODELCOMPILER modelcompiler DOC "modelcompiler executable for the host")
else (CMAKE_CROSSCOMPILING)
	set(MODELCOMPILER $<TARGET_FILE:modelcompiler>)
endif (CMAKE_CROSSCOMPILING)

add_custom_target(build-data)

if (MODELCOMPILER)
	# Optimize the models.
	# This really shouldn't be done inside the source tree...
	if (NOT MSVC)
		add_custom_target(build-models
			COMMAND ${CMAKE_COMMAND} -E env SDL_VIDEODRIVER=dummy PIONEER_LOCAL_DATA_ONLY=1
			${MODELCOMPILER} -b inplace
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "Optimizing models" VERBATIM
		)
		add_dependencies(build-data build-models)
	endif (NOT MSVC)
else (MODELCOMPILER)
	message(WARNING "No modelcompiler provided, models won't be optimized!")
endif(MODELCOMPILER)

install(TARGETS ${PROJECT_NAME} editor modelcompiler savegamedump
	RUNTIME DESTINATION ${PIONEER_INSTALL_BINDIR}
)
install(DIRECTORY data/
	DESTINATION ${PIONEER_INSTALL_DATADIR}/data
	REGEX "/models" EXCLUDE
	PATTERN ".gitignore" EXCLUDE
	PATTERN "listdata.*" EXCLUDE
	PATTERN "Makefile.am" EXCLUDE
)
install(DIRECTORY data/models/
	DESTINATION ${PIONEER_INSTALL_DATADIR}/data/models
	FILES_MATCHING PATTERN "*.sgm" PATTERN "*.dds" PATTERN "*.png"
)

list(APPEND install_txt
	"AUTHORS.txt"
	"Changelog.txt"
	"editor.txt"
	"Quickstart.txt"
	"README.md"
	"NEWS.md"
	)

install(FILES ${install_txt} DESTINATION ${PIONEER_INSTALL_DATADIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/licenses
	DESTINATION ${PIONEER_INSTALL_DATADIR})

if(APPIMAGE_BUILD)
    set(PIONEER_EXECUTABLE init_pioneer.sh)
    set(PIONEER_SH_FILE ${CMAKE_BINARY_DIR}/init_pioneer.sh)
    configure_file(init_pioneer.sh.cmakein ${PIONEER_SH_FILE} @ONLY)
    install(FILES ${PIONEER_SH_FILE}
        DESTINATION ${PIONEER_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
    )
else(APPIMAGE_BUILD)
    set(PIONEER_EXECUTABLE ${PIONEER_INSTALL_BINDIR}/pioneer)
endif(APPIMAGE_BUILD)

if (WIN32)
	configure_file(pioneer.iss.cmakein pioneer.iss @ONLY)
	file(GLOB win_libs ../pioneer-thirdparty/win32/bin/${MSVC_ARCH}/vs2019/*.dll)
	install(FILES ${win_libs} DESTINATION ${CMAKE_INSTALL_PREFIX})
	if(NOT ISCC)
		set(ISCC "C:/Program Files (x86)/Inno Setup 6/ISCC.exe")
	endif(NOT ISCC)
	add_custom_target(win-installer COMMAND ${ISCC} /Q pioneer.iss)
endif (WIN32)

if (UNIX AND NOT PIONEER_INSTALL_INPLACE)
	set(PIONEER_DESKTOP_FILE ${CMAKE_BINARY_DIR}/metadata/net.pioneerspacesim.Pioneer.desktop)
	configure_file(metadata/net.pioneerspacesim.Pioneer.desktop.cmakein ${PIONEER_DESKTOP_FILE} @ONLY)
	install(FILES ${PIONEER_DESKTOP_FILE}
		DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
	)
	install(FILES metadata/net.pioneerspacesim.Pioneer.metainfo.xml
		DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
	)

	foreach(_i IN ITEMS 16 22 24 32 40 48 64 128 256)
		install(FILES application-icon/pngs/pioneer-${_i}x${_i}.png
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${_i}x${_i}/apps
			RENAME net.pioneerspacesim.Pioneer.png
		)
	endforeach()
endif (UNIX AND NOT PIONEER_INSTALL_INPLACE)
