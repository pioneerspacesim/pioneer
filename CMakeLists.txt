# Pioneer CMake script
#
# By Sukender (Benoit Neil), from PVLE and OSG CMake scripts.
# Licensing terms of all CMake scripts in the project are compatible with GPL v3:
#   - For Sukender's part (new code, and code coming from PVLE): WTFPL v2
#   - For code coming from OSG: LGPL v2

# Current status of these CMake scripts: alpha (still bugs/tests to perform/missing features)
# High priority tasks:
# - Discuss about the binary output directory:
#		1. source dir ("/", as MSVC projects in "/win32")
#		2. specific dir, with install/packaging moving files where necesarry
# - Do/test cross-building.
#		robn: I use mingw-cross-env.nongnu.org and pass --host=i686-pc-mingw32 to configure. Everything else is "documented" in configure.ac. Mostly avoiding feature detection and just hardcoding paths and switches.
# - XCode project doesn't find GL/glew.h (Dessimat0r's configuration). System paths are:
#		usr/include/GL/glew.h
#		usr/include/GL/GL/glew.h
# - Test everything, especially compared to actual build systems.
#		XCode builds
#		Try the ccache thing. On my (Sukender) Ubuntu Linux, ccache comes with symlinks but I'm not 100% sure it will work everywhere.
#		Try with an old compiler, to ensure it doesn't complain about unsupported compile flags. Build system has to test for the availbility of each warning flag (old compiler support). This SHOULD be done automatically by CMake, but this hasn't been tested.
#		MinGW builds. They need all dependencies to be compiled with MinGW (can't mix MSVC and MinGW DLLs!). However a quick test shows that it compiles and all symbols are found (using MSVC DLLs). I (Sukender) assume it works.
#		Cygwin builds. They've never been tested.
# - Handle extra stuff found in osx/Makefile.am
# - Remove release/debug output subdirs for MSVC.
#
# Low priority tasks:
# - Create a static library (libwin32) from FileSystemWin32.cpp, to stick with configure.ac/Makefile.am.
# - Packaging:
#		Please note the install autodetects dependencies (.so, .dll) and install them alongside the executable if needed (option: Pioneer_INSTALL_DEPENDENCIES).
#		To do:
#			7z packaging for Windows, if possible (May be useless if NSIS is available?)
#			Debian package
#				Previously, there was a "data" target to prepare data. Should not be necessary anymore.
#			DMG (OSX) packaging - Work in progress
#				Possible issue: the packager utility may not locate pioneer-logo.icns and executables
# - Do source packaging. To be defined.
# - Write a PNG+zlib finder that handles debug builds (for MSVC, actually)
# - Pioneer_USE_EXTERNAL_LUA_LIB option (equivalent of --with-external-liblua)
# - Interfacing with CDash, to report builds
#		This should be adressed AFTER CMake scripts are stable.
#		CDash main server is free for open source, limited to 10 daily builds and 3 users. Else you may either pay (!) or install CDash on your own server (open source/free project).
# - Prepare unit testing projects, with CTest so that it links with CDash reporting
# - Create a doxygen target
#
# Date where build scripts have been successfully tested:
#  Linux-gcc    32   2012-03-31 (Brianetta - or was it 64bits???)
#  Linux-gcc    64   never
#  OSX-gcc      32   never
#  OSX-gcc      64   2012-04-02 (Dessimat0r)
#  OSX-XCode    32   never
#  OSX-XCode    64   failed (Dessimat0r)
#  Windows-gcc  32   never, but almost did with CodeBlocks+MinGW. MinGW dependencies were missing but testing with MSVC DLLs shown that symbols are found. (Sukender)
#  Windows-gcc  64   never
#  Windows-msvc 32   2012-03-31 (Sukender)
#  Windows-msvc 64   never


CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5 FATAL_ERROR)

project( Pioneer )
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 22)
SET(${PROJECT_NAME}_PATCH_VERSION 0)
SET(${PROJECT_NAME}_BUILD_VERSION 0)
SET(${PROJECT_NAME}_VERSION_QUALITY "a")		# "dev"=Pre-alpha, "a"=Alpha, "b"=Beta, "RC"=Release Candidate, ""=Stable (No other extension is allowed, since it may be used directly in code)

SET(${PROJECT_NAME}_VERSION_NAME "alpha-${${PROJECT_NAME}_MINOR_VERSION}-dev")	# Name of the version, if any. For now, it handles the versionning as configure.ac does.

#SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION}.${${PROJECT_NAME}_BUILD_VERSION})
#SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION}.${${PROJECT_NAME}_BUILD_VERSION}${${PROJECT_NAME}_VERSION_QUALITY})

# We have some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

include( "${PROJECT_SOURCE_DIR}/CMakeModules/AddDepend.cmake" )
include( "${PROJECT_SOURCE_DIR}/CMakeModules/Warnings.cmake" )

set(HAS_GCC_STYLE_FLAGS OFF)
if(UNIX OR MINGW OR CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(HAS_GCC_STYLE_FLAGS ON)
endif()

#########################################################################
# Find GIT, to enable getting the head SHA (optional for compilation of course)
# The GIT's head SHA is to include in the build (for automated builds)
#   - git log -n1 --pretty=%h HEAD  (Generates short SHA)
#   - git rev-parse HEAD  (Generates long SHA)
set(IS_WORKING_COPY OFF)
set (GIT_SHA )
find_path(TMP_GIT_CONFIG_FILE config PATHS "${PROJECT_SOURCE_DIR}/.git")		# Try finding git config file, to detect if we're in a working copy
if (TMP_GIT_CONFIG_FILE)
	set(IS_WORKING_COPY ON)
	OPTION(${PROJECT_NAME}_BYPASS_REVISION_RETREIVAL "Set to ON to use skip getting the current revision from the GIT repository if you find it too slow, or don't want the code to know about it." OFF)
	mark_as_advanced(${PROJECT_NAME}_BYPASS_REVISION_RETREIVAL)
	find_package(Git)
	if (NOT ${PROJECT_NAME}_BYPASS_REVISION_RETREIVAL)
		if (GIT_FOUND)		# AND NOT GIT_SHA
			message(STATUS "[Info] Project is detected as being a GIT working copy. Getting Git SHA...")
			execute_process(COMMAND "${GIT_EXECUTABLE}" "log" "-n1" "--pretty=%h" "HEAD"
				WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
				OUTPUT_VARIABLE GIT_SHA_TEMP
				RESULT_VARIABLE GIT_SHA_RESULT
				ERROR_VARIABLE GIT_SHA_ERROR
			)
			if (${GIT_SHA_RESULT} EQUAL 0)
				string(REGEX MATCH "[0-9a-fA-F]+" GIT_SHA "${GIT_SHA_TEMP}")		# Remove potential newlines
				message(STATUS "Git SHA: ${GIT_SHA}")
			else()
				set(GIT_SHA )
				message("Git returned error ${GIT_SHA_RESULT}: ${GIT_SHA_ERROR}")
			endif()
		else()
			message(STATUS "[Info] Project is detected as being a GIT working copy, but Git executable hasn't been found.")
		endif()
	else()
		message(STATUS "[Info] Project is detected as being a GIT working copy, but ${PROJECT_NAME}_BYPASS_REVISION_RETREIVAL is enabled.")
	endif()
endif()

if (NOT GIT_SHA)
	#set(GIT_SHA "Unknown revision")
	#message("Couldn't get Git SHA. Value will be defined to '${GIT_SHA}'.")
	message("Couldn't get Git SHA. Value will be an empty string.")
endif()

#########################################################################
# Find dependencies

# Setup for finding shipped dependencies
set(SHIPPED_DEPENDENCIES_PATH )
if (MSVC)		# MinGW/Cygwin dependencies are not available as shipped ones, hence not testing "if(WIN32)"
	set(SHIPPED_DEPENDENCIES_PATH "${PROJECT_SOURCE_DIR}/win32")
endif()

if (SHIPPED_DEPENDENCIES_PATH)
	#SET(CMAKE_PREFIX_PATH "${SHIPPED_DEPENDENCIES_PATH}" ${CMAKE_PREFIX_PATH})
	SET(CMAKE_LIBRARY_PATH "${SHIPPED_DEPENDENCIES_PATH}" "${SHIPPED_DEPENDENCIES_PATH}/lib" ${CMAKE_LIBRARY_PATH})
	SET(CMAKE_INCLUDE_PATH "${SHIPPED_DEPENDENCIES_PATH}" "${SHIPPED_DEPENDENCIES_PATH}/include" ${CMAKE_INCLUDE_PATH})
endif()

# SET(Boost_USE_STATIC_LIBS OFF)
# SET(Boost_USE_MULTITHREADED ON)
# SET(Boost_ADDITIONAL_VERSIONS "1.48" "1.48.0")
# FIND_PACKAGE(Boost 1.48.0 REQUIRED QUIET COMPONENTS filesystem program_options serialization system regex date_time iostreams unit_test_framework locale thread)

FIND_PACKAGE(OpenGL REQUIRED)		# With GLU
FIND_PACKAGE(Glew REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(OGG REQUIRED)
FIND_PACKAGE(Vorbis REQUIRED)
FIND_PACKAGE(VorbisFile REQUIRED)
#FIND_PACKAGE(libtool)				# TODO: To create. Unix-like only. Not REQUIRED. According to jpabx: "it hides some of the differences between platforms, and deals with the .so version system so that builds can cope with multiple installed library versions"
# OPTION(${PROJECT_NAME}_USE_EXTERNAL_LUA_LIB "Set to ON to use external Lua library, instead of shipped one." OFF)
# if(${PROJECT_NAME}_USE_EXTERNAL_LUA_LIB)
	# FIND_PACKAGE(Lua51)
# endif()
FIND_PACKAGE(SigCpp2 REQUIRED)
SET(PNG_NAMES png15_static libpng15_static)		# Additional PNG names to look for
FIND_PACKAGE(PNG REQUIRED)
#FIND_PACKAGE(zlib REQUIRED)		# Already called by findPNG

IF(UNIX)
	# # Not sure what this will do on Cygwin and Msys
	# # Also, remember OS X X11 is a user installed option so it may not exist.
	# FIND_PACKAGE(X11)
	# Some Unicies need explicit linkage to the Math library or the build fails.
	FIND_LIBRARY(MATH_LIBRARY m)
ENDIF()

if(WIN32)
	# Adding shlwapi.lib for FileSystem32.cpp (Sukender: I'd rather use boost.filesystem!)
	if(MSVC)
		set(LIBREGEX "shlwapi\\.lib")
		set(LIBNAME "shlwapi.lib")
	else()
		# gcc-style for MinGW
		set(LIBREGEX "-lshlwapi")
		set(LIBNAME "-lshlwapi")
	endif()
	string(REGEX MATCH "${LIBREGEX}" LIBTEST "${CMAKE_CXX_STANDARD_LIBRARIES}")
	if(NOT LIBTEST)
		SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${LIBNAME}" CACHE STRING "" FORCE)
	endif()
	string(REGEX MATCH "${LIBREGEX}" LIBTEST "${CMAKE_C_STANDARD_LIBRARIES}")
	if(NOT LIBTEST)
		SET(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} ${LIBNAME}" CACHE STRING "" FORCE)
	endif()
endif()

ADD_DEFINITIONS(-D_USE_MATH_DEFINES)		# Sukender: to get "M_PI" I guess?

#########################################################################
# Precompiled headers support
IF(MSVC)
	SET(PRECOMPILED_DEFAULT ON)
ELSE()
	SET(PRECOMPILED_DEFAULT OFF)
ENDIF()
OPTION(${PROJECT_NAME}_USE_PRECOMPILED_HEADERS "Set to ON to use precompiled headers for compilers that support it. This needs to be turned off to check that compilation units includes the appropriate files." ${PRECOMPILED_DEFAULT})
INCLUDE(PrecompiledHeader)
if (WIN32)
	SET(PCH_DEFAULT_H   "${PROJECT_SOURCE_DIR}/win32/src/pch.h")
	SET(PCH_DEFAULT_CPP "${PROJECT_SOURCE_DIR}/win32/src/pch.cpp")
else()
	if(${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
		MESSAGE("Warning: Precompiled headers have not been set for your platform yet.")
	endif()
	SET(PCH_DEFAULT_H   "")
	SET(PCH_DEFAULT_CPP "")
endif()

# Option allowing Win32 apps to have a console or not
IF(WIN32)
	OPTION(${PROJECT_NAME}_FORCE_CONSOLE_APPS "Activate to make all apps to have a console under Windows" OFF)
ENDIF()

#########################################################################
# ccache

#if(NOT WIN32)
if(HAS_GCC_STYLE_FLAGS)
	OPTION(${PROJECT_NAME}_USE_ccache "Set to ON to use ccache to compile." OFF)
	if (${PROJECT_NAME}_USE_ccache)
		find_package(ccache QUIET)
		if(CCACHE_FOUND)
			set(CMAKE_CXX_COMPILER ${CCACHE_GXX})
			set(CMAKE_C_COMPILER ${CCACHE_GCC})
		else()
			message("ccache was requested but was not found. Build will proceed as normal")
		endif()
	endif()
endif()


#########################################################################
# Macros & values for compatibility options with configure.ac

# Adds a C/C++ flag
MACRO(ADD_FLAG FLAG)
	IF(NOT CMAKE_CXX_FLAGS MATCHES "${FLAG}")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
	ENDIF()
	IF(NOT CMAKE_C_FLAGS MATCHES "${FLAG}")
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
	ENDIF()
ENDMACRO()
# Removes a C/C++ flag using a regex
MACRO(REMOVE_FLAG_REGEX FLAG)
	STRING(REGEX REPLACE "${FLAG}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REGEX REPLACE "${FLAG}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
ENDMACRO()

# Adds a C/C++ flag, for a given configuration
MACRO(ADD_FLAG_CONFIG CONFIG FLAG)
	STRING(TOUPPER "${CONFIG}" CONFIG_UPPER)
	IF(NOT CMAKE_CXX_FLAGS_${CONFIG_UPPER} MATCHES "${FLAG}")
		SET(CMAKE_CXX_FLAGS_${CONFIG_UPPER} "${CMAKE_CXX_FLAGS_${CONFIG_UPPER}} ${FLAG}")
	ENDIF()
	IF(NOT CMAKE_C_FLAGS_${CONFIG_UPPER} MATCHES "${FLAG}")
		SET(CMAKE_C_FLAGS_${CONFIG_UPPER} "${CMAKE_C_FLAGS_${CONFIG_UPPER}} ${FLAG}")
	ENDIF()
ENDMACRO()
# Removes a C/C++ flag using a regex, for a given configuration
MACRO(REMOVE_FLAG_REGEX_CONFIG CONFIG FLAG)
	STRING(TOUPPER "${CONFIG}" CONFIG_UPPER)
	STRING(REGEX REPLACE "${FLAG}" "" CMAKE_CXX_FLAGS_${CONFIG_UPPER} "${CMAKE_CXX_FLAGS_${CONFIG_UPPER}}")
	STRING(REGEX REPLACE "${FLAG}" "" CMAKE_C_FLAGS_${CONFIG_UPPER} "${CMAKE_C_FLAGS_${CONFIG_UPPER}}")
ENDMACRO()

# Removes a C/C++ flag using a regex, for all configurations
MACRO(REMOVE_FLAG_REGEX_ALL_CONFIGS FLAG)
	REMOVE_FLAG_REGEX("${FLAG}")
	FOREACH(CURCONFIG ${CMAKE_CONFIGURATION_TYPES})
		REMOVE_FLAG_REGEX_CONFIG("${CURCONFIG}" "${FLAG}")
	ENDFOREACH()
ENDMACRO()


# Notes:
# gcc optimisation flags (-O) are: s, fast, 0, 1, 2, 3
# MSVC optimisation flags (/O) are: 1, 2, d, x. Potentially: g (global optimisations), i (intrinsics), s (small code), t (favor speed), y (frame pointer)
# MSVC inlining flags: /Ob{0|1|2}

# Selecting regex for optimisation flags
set(OptimisationFlagRegex "-O[^ ]+")		# Default configuration is gcc
if(HAS_GCC_STYLE_FLAGS)
	# Nothing to do.
elseif(MSVC)
	set(OptimisationFlagRegex "/O[^b][^ ]*")		# All but inlining mode
else()
	# No warning here
endif()

# Selecting regex for inlining flags
set(InilineFlagRegex "-fno-inline")		# Default configuration is gcc
if(HAS_GCC_STYLE_FLAGS)
	# Nothing to do.
elseif(MSVC)
	set(InilineFlagRegex "/Ob[012]")
else()
	# No warning here
endif()


#########################################################################
# Inlining (-fno-inline)

# Debug must have -fno-inline in the gcc/g++.
# Note: MSVC Debug default conig is to have no inlining enabled
# TODO: make this appear in the cache?
if(HAS_GCC_STYLE_FLAGS)
	ADD_FLAG_CONFIG(Debug "-fno-inline")
endif()


# No inline
OPTION(${PROJECT_NAME}_FORCE_NO_INLINE "Set to ON to force no inlining (-fno-inline option for gcc, /Ob0 for MSVC). Compatibility option for --with-fno-inline." OFF)
if (${PROJECT_NAME}_FORCE_NO_INLINE)
	if(HAS_GCC_STYLE_FLAGS)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("${InilineFlagRegex}")
		ADD_FLAG("-fno-inline")
	elseif(MSVC)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("${InilineFlagRegex}")
		ADD_FLAG("/Ob0")
	else()
		message("Cannot apply ${PROJECT_NAME}_FORCE_NO_INLINE for your compiler (not implemented)")
	endif()
endif()

#########################################################################
# Default opimisations, and "No optimise" option (-O0)

# RelWithDebInfo must use the same -On as Release
# TODO: make this appear in the cache?
STRING(REGEX MATCH "${OptimisationFlagRegex}" ReleaseOptimisationFlag "${CMAKE_CXX_FLAGS_RELEASE}")
if (ReleaseOptimisationFlag)
	REMOVE_FLAG_REGEX_CONFIG(RelWithDebInfo "${OptimisationFlagRegex}")
	ADD_FLAG_CONFIG(RelWithDebInfo "${ReleaseOptimisationFlag}")
else()
	message("Cannot change RelWithDebInfo optimisation flags for your compiler (not implemented)")
endif()


# No optimise
OPTION(${PROJECT_NAME}_FORCE_NO_OPTIMISE "Set to ON to force disabling optimizations, whatever the build type. Compatibility option for --with-no-optimise." OFF)
if(${PROJECT_NAME}_FORCE_NO_OPTIMISE)
	if(HAS_GCC_STYLE_FLAGS)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("${OptimisationFlagRegex}")
		ADD_FLAG("-O0")
	elseif(MSVC)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("${OptimisationFlagRegex}")
		ADD_FLAG("/Od")
	else()
		message("Cannot apply ${PROJECT_NAME}_FORCE_NO_OPTIMISE for your compiler (not implemented)")
	endif()
endif()

# message("DEBUG message - CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
# message("DEBUG message - CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
# message("DEBUG message - CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
# message("DEBUG message - CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")

#########################################################################
# Debug things

# Setup default DEBUG/NDEBUG flags, as they differ from CMake's default:
#   1. Replace NDEBUG with DEBUG for RelWithDebugInfo
#	2. Add DEBUG for Debug (Note: MSVC has "_DEBUG" by default)
if(HAS_GCC_STYLE_FLAGS)
	REMOVE_FLAG_REGEX_CONFIG(RelWithDebInfo "-DNDEBUG")
	ADD_FLAG_CONFIG(RelWithDebInfo "-DDEBUG")
	ADD_FLAG_CONFIG(Debug "-DDEBUG")
elseif(MSVC)
	REMOVE_FLAG_REGEX_CONFIG(RelWithDebInfo "/D NDEBUG")
	ADD_FLAG_CONFIG(RelWithDebInfo "/D DEBUG")
	ADD_FLAG_CONFIG(Debug "/D DEBUG")
else()
	message("Cannot setup DEBUG flags for your compiler (not implemented)")
endif()


# Add FORCE_DEBUG_BUILD option
OPTION(${PROJECT_NAME}_FORCE_DEBUG_BUILD "Set to ON to force 'DEBUG' preprocessor definition. Compatibility option for --enable-debug." OFF)
if(${PROJECT_NAME}_FORCE_DEBUG_BUILD)
	if(HAS_GCC_STYLE_FLAGS)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("-DNDEBUG")
	elseif(MSVC)
		REMOVE_FLAG_REGEX_ALL_CONFIGS("/D NDEBUG")
	else()
		message("Cannot ensure ${PROJECT_NAME}_FORCE_DEBUG_BUILD for your compiler (not implemented)")
	endif()
	ADD_DEFINITIONS("-DDEBUG")		# Adds for all targets, all configuration. Will not be visible in CMAKE_CXX_FLAGS* variables
endif()

# message("DEBUG message - CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
# message("DEBUG message - CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
# message("DEBUG message - CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
# message("DEBUG message - CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")

#########################################################################
# Make the headers visible to everything
IF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
   INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/include")		# For auto-generated config header(s)
ENDIF()

INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/contrib"		# For Lua/OOLua
	"${PROJECT_SOURCE_DIR}/contrib/lua"	# OOLua directly includes "lua.h" instead of "lua/lua.h"
)

if (SHIPPED_DEPENDENCIES_PATH)
	INCLUDE_DIRECTORIES(
		"${SHIPPED_DEPENDENCIES_PATH}/include"
		"${SHIPPED_DEPENDENCIES_PATH}/src"
	)
endif()

#########################################################################
# Dependencies install

set(${PROJECT_NAME}_DEPENDENCIES_DIRS_DEFAULT "")
if (SHIPPED_DEPENDENCIES_PATH)
	set(${PROJECT_NAME}_DEPENDENCIES_DIRS_DEFAULT "${SHIPPED_DEPENDENCIES_PATH}/lib;${SHIPPED_DEPENDENCIES_PATH}/bin")
endif()

OPTION(${PROJECT_NAME}_INSTALL_DEPENDENCIES "Set to ON to search and copy direct dependencies of executables at install/package time." ON)
IF(${PROJECT_NAME}_INSTALL_DEPENDENCIES)
	# Pre-requisites dirs
	SET(${PROJECT_NAME}_DEPENDENCIES_DIRS "${${PROJECT_NAME}_DEPENDENCIES_DIRS_DEFAULT}" CACHE STRING "Paths for dependencies binary dirs. Accepts both Unix/Windows paths and variable set such as 'PATH=C:\\foo;C:\\bar;%PATH%'.")
	IF (NOT ${PROJECT_NAME}_DEPENDENCIES_DIRS)
		MESSAGE("[Warning] ${PROJECT_NAME}_DEPENDENCIES_DIRS is not defined. Install/Package may not work.")
	ENDIF()
	SET(PREREQ_DIRS "${${PROJECT_NAME}_DEPENDENCIES_DIRS}")			# ;${list of binary dirs of project}
	STRING(REPLACE "\\" "/" PREREQ_DIRS "${PREREQ_DIRS}")
	STRING(REGEX REPLACE "^[a-zA-Z0-9_]+=" "" PREREQ_DIRS "${PREREQ_DIRS}")
	STRING(REPLACE ";" "\" \"" PREREQ_DIRS "${PREREQ_DIRS}")
	SET(PREREQ_DIRS "\"${PREREQ_DIRS}\"")
ENDIF()

OPTION(${PROJECT_NAME}_INSTALL_DEPENDENCIES_VERBOSE "Set to ON to make each dependency path be printed when installing." OFF)
SET(DEPENDENCIES_VERBOSE ${${PROJECT_NAME}_INSTALL_DEPENDENCIES_VERBOSE})

#########################################################################
# Configure Testing

IF (BUILD_UNIT_TESTS AND BUILD_TESTING)
	IF (NOT CTEST_ADDITIONNAL_ENVIRONMENT AND ${PROJECT_NAME}_INSTALL_DEPENDENCIES)
		# Delete the cache entry if empty, so that it can take its default value
		UNSET(CTEST_ADDITIONNAL_ENVIRONMENT CACHE)
	ENDIF()
	SET(CTEST_ADDITIONNAL_ENVIRONMENT "${${PROJECT_NAME}_DEPENDENCIES_DIRS}" CACHE STRING "Additionnal environment, such as 'PATH=C:\\foo;C:\\bar;%PATH%'.")
	IF (NOT CTEST_ADDITIONNAL_ENVIRONMENT)
		MESSAGE("[Warning] CTEST_ADDITIONNAL_ENVIRONMENT is not defined. RUN_TESTS project may not run.")
	ENDIF()
	STRING(REPLACE "\\" "\\\\" CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED "${CTEST_ADDITIONNAL_ENVIRONMENT}")
	STRING(REPLACE ";" "\\;" CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED "${CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED}")
	#STRING(REGEX REPLACE "\\%([^\\%]*)\\%" "$ENV{\\1}" CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED "${CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED}")
ENDIF()


OPTION(BUILD_UNIT_TESTS "Build ${PROJECT_NAME} unit tests. This adds some projects to the build tree, containing unit tests." OFF)
MACRO( ADD_UNIT_TEST projectName )
	FOREACH(CURCONFIG ${CMAKE_CONFIGURATION_TYPES})
		GET_TARGET_PROPERTY( testExecutable ${projectName} LOCATION_${CURCONFIG} )
		STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}" testExecutable "${testExecutable}")
		ADD_TEST( NAME "${projectName}-${CURCONFIG}" CONFIGURATIONS ${CURCONFIG} COMMAND ${testExecutable} )
		SET_TESTS_PROPERTIES("${projectName}-${CURCONFIG}" PROPERTIES ENVIRONMENT "${CTEST_ADDITIONNAL_ENVIRONMENT_PROCESSED}\;$ENV{PATH}")
	ENDFOREACH()
ENDMACRO()

IF(BUILD_UNIT_TESTS)
	INCLUDE(CTestConfig.cmake)
	ENABLE_TESTING()
	INCLUDE(CTest)
ENDIF()


################################################################################
# Create bin and lib directories if required

SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
# SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
# MAKE_DIRECTORY(${OUTPUT_LIBDIR})

# SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
# IF(WIN32)
	# SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
# ELSE()
	# SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
# ENDIF()
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")

################################################################################
# Set Config and Version file

macro(SET01 VARNAME VALUE)
	if(${VALUE})
		set(${VARNAME} 1)
	else()
		set(${VARNAME} 0)
	endif()
endmacro()

OPTION(${PROJECT_NAME}_WITH_OBJECTVIEWER "" ON)
SET01(WITH_OBJECTVIEWER ${${PROJECT_NAME}_WITH_OBJECTVIEWER})
OPTION(${PROJECT_NAME}_WITH_DEVKEYS "" ON)
SET01(WITH_DEVKEYS ${${PROJECT_NAME}_WITH_DEVKEYS})

# Data directory
SET(${PROJECT_NAME}_DATA_DIR "" CACHE PATH "Where Pioneer expects its data files to be [data]. Packagers MUST set this!")
if(NOT ${PROJECT_NAME}_DATA_DIR)
	SET(${PROJECT_NAME}_DATA_DIR "data")
	message("Using the default value for ${PROJECT_NAME}_DATA_DIR. Installing is NOT recommended. IF YOU ARE PACKAGING PIONEER, SET THIS VARIABLE!")
endif()


set(Python_ADDITIONAL_VERSIONS 3.3 3.2 3.1 3.0 2.9 2.8)		# Sukender: not sure, but harmless, as far as I know
find_package(PythonInterp)
if (NOT PYTHONINTERP_FOUND)
	message("Python interpreter not found. enum table cannot be automatically generated/updated.")
else()
	OPTION(${PROJECT_NAME}_BUILD_ENUM_AUTO "Enables/disable building 'enum' target automatically, using python. It will be built every time you build the project. This ensures enum_table is always up-to-date, but slows down a bit building." OFF)
	ADD_SUBDIRECTORY(scripts)
endif()

macro(MAKE_ENUM_DEPEND TARGETNAME)
	if (PYTHONINTERP_FOUND AND ${PROJECT_NAME}_BUILD_ENUM_AUTO)
		add_dependencies(${TARGETNAME} enum)
	endif()
endmacro()

SET(${PROJECT_NAME}_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/Config.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/Config.in" "${${PROJECT_NAME}_CONFIG_HEADER}")

SET(${PROJECT_NAME}_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/VersionNumber.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/VersionNumber.in" "${${PROJECT_NAME}_VERSION_HEADER}")

ADD_SUBDIRECTORY(contrib/lua)
ADD_SUBDIRECTORY(contrib/oolua)
ADD_SUBDIRECTORY(src)

# IF(${PROJECT_NAME}_UNIT_TESTS)
	# ADD_SUBDIRECTORY(tests)
# ENDIF()


################################################################################
# Code documentation

# NaturalDocs ("codedoc" target)
find_package(NaturalDocs)
option(${PROJECT_NAME}_BUILD_DOCUMENTATION "Build reference documentation using NaturalDocs" ${NaturalDocs_FOUND})
mark_as_advanced(${PROJECT_NAME}_BUILD_DOCUMENTATION)
if (${PROJECT_NAME}_BUILD_DOCUMENTATION)
	if(NOT NaturalDocs_FOUND)
		message("NaturalDocs not found. Skipping code documentation generation.")
	else()
		add_custom_target(codedoc
			"${NaturalDocs}" "-i" "src/" "-i" "data/libs/" "-xi" "src/data/" "-o" "HTML" "codedoc/" "-p" "nd/" "-do" "-ro" "-s" "Default" "Local"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			COMMENT "Generating code documentation (NaturalDocs)...")
		#INSTALL(DIRECTORY "codedoc" DESTINATION ".")
	endif()
endif()

# Doxygen

# INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
# OPTION(BUILD_DOCUMENTATION "Build reference documentation using doxygen" ON)
# MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)
# # To build the documention, you will have to enable it
# # and then do the equivalent of "make DoxygenDoc".
# IF(BUILD_DOCUMENTATION)

	# SET(${PROJECT_NAME}_DOXYFILE "${PROJECT_BINARY_DIR}/doc/doxyfile")
	# CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile.in" ${${PROJECT_NAME}_DOXYFILE})

    # #INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/OpenThreadsReferenceDocs DESTINATION doc COMPONENT openthreads-doc)

    # # This creates a new target to build documentation.
    # # It runs ${DOXYGEN} which is the full path and executable to
    # # Doxygen on your system, set by the FindDoxygen.cmake module
    # # (called by FindDocumentation.cmake).
    # # It runs the final generated Doxyfile against it.
    # # The DOT_PATH is substituted into the Doxyfile.
    # ADD_CUSTOM_TARGET(Doc ${DOXYGEN}
        # ${${PROJECT_NAME}_DOXYFILE}
    # )
# ENDIF(BUILD_DOCUMENTATION)

################################################################################
# CPack / Install
FUNCTION(PACK_DATA_DIR DIR_TO_PACK RECURSE)
	# INSTALL(DIRECTORY ...) is always recursive...
	SET(DIR_PATH "${PROJECT_SOURCE_DIR}/${DIR_TO_PACK}")
	FILE(GLOB FILES_TO_PACK_RELATIVE RELATIVE "${DIR_PATH}" "${DIR_PATH}/*")
	LIST(REMOVE_ITEM FILES_TO_PACK_RELATIVE ".svn")		#remove '.svn'
	# Make paths absolute
	SET(FILES_TO_PACK )
	FOREACH(CUR_FILE ${FILES_TO_PACK_RELATIVE})
		IF(NOT(IS_DIRECTORY "${DIR_PATH}/${CUR_FILE}"))
			LIST(APPEND FILES_TO_PACK "${DIR_PATH}/${CUR_FILE}")
		ELSE()
			IF(RECURSE)
				PACK_DATA_DIR("${DIR_TO_PACK}/${CUR_FILE}" ${RECURSE})
			ENDIF()
		ENDIF()
	ENDFOREACH()
	INSTALL(FILES ${FILES_TO_PACK} DESTINATION "${DIR_TO_PACK}")
ENDFUNCTION()

# Add files to pack
INSTALL(FILES
	AUTHORS.txt
	#COMPILING.OSX.txt
	Changelog.txt
	COPYING.txt
	fonts.COPYING.txt
	lua.COPYING.txt
	Modelviewer.txt
	music.COPYING.txt
	Quickstart.txt
	README.txt
	#bootstrap
	#generic-exec
	#Makefile.common
	#modelviewer
	#pioneer
	DESTINATION ".")

if(APPLE)
	# Not sure this is useful, even for DMG packaging
	INSTALL(FILES "osx/pioneer-logo.icns" DESTINATION ".")
endif()

# Add directories to pack
PACK_DATA_DIR("${${PROJECT_NAME}_DATA_DIR}" ON)		# You may comment it to speed up install process by skipping data. This helps debugging but you'll need to uncomment it afterwards :)

IF(CMAKE_CPACK_COMMAND)
    OPTION(BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
    IF(BUILD_PACKAGES)
		INCLUDE("CMakeModules/CPackProject.cmake" REQUIRED)
    ENDIF()
ENDIF()

################################################################################
# This needs to be run very last so other parts of the scripts can take advantage of this.
IF(NOT ${PROJECT_NAME}_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(${PROJECT_NAME}_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF()
