# Pioneer CMake script
#
# By Sukender (Benoit Neil), from PVLE and OSG CMake scripts.
# Licensing terms of all CMake scripts in the project are compatible with GPL v3:
#   - For Sukender's part (new code, and code coming from PVLE): WTFPL v2
#   - For code coming from OSG: LGPL v2

# Current status of these CMake scripts: pre-alpha (Linux and OSX have not been tested, lots of features are missing)
# TODO:
# - Replace PIONEER_VERSION / PIONEER_EXTRAVERSION usage with Version.h.
# 		Version.h/.cpp allow to grab versionning of VersionNumber.h without recompiling the entire project.
# - Handle ccache to speedup builds (Linux)
#		http://stackoverflow.com/questions/1815688/how-to-use-ccache-with-cmake
#		http://stackoverflow.com/questions/2930500/ccache-does-not-hit-cache-without-deleting-cmake-files
#		--with-ccache: enables compiling with ccache
#			"ccache was requested but was not found. Build will proceed as normal"
# - Same options as automake:
#		--enable-debug: -DDEBUG
#		--with-no-optimise: -O0 (else -O3)
#		--with-fno-inline:  CXXFlags =-fno-inline
#		ALREADY IN MINSIZEREL?
				# if(CMAKE_COMPILER_IS_GNUCXX)
					# OPTION(${PROJECT_NAME}_USE_ccache "Set to ON to use ccache to compile." OFF)
					# if (${PROJECT_NAME}_USE_ccache)
					# endif()
				# endif()
				# #		--with-fno-inline:  CXXFlags =-fno-inline
#		------ less important
#		--with-gprof : CXXFlags = -pg
#		has to test for the availbility of each warning flag and use them and not use ones that aren't there for C and C++ (different sets of flags)
# 		AC_INIT([pioneer], [alpha-21-dev], [pioneer@pioneerspacesim.net])
# 		AC_CONFIG_HEADERS([config.h])
# - Write packaging code (to create distributions ZIP or installers).
#		Please note the install autodetects dependencies (.so, .dll) and install them alongside the executable if needed.
#		The install part should be okay, except for some MSVC system DLLs (MSVCRT...) which go in "/bin" instead of "/".
# - Test everything, especially compared to actual build systems (TODO: list all features).
# - Eventually write a PNG+zlib finder that handles debug builds (for MSVC, actually)
# - Add a "configure" script to create "build" subdir, and run cmake. The script should warn about an existing "build" subdir and stop processing.
# - _USE_EXTERNAL_LUA_LIB (--with-external-liblua)
#
# Date where build scripts have been successfully tested:
#  Linux-gcc    32   never (builds under Ubuntu, but running hasn't been tested yet)
#  Linux-gcc    64   never
#  OSX-gcc      32   never
#  OSX-gcc      64   never
#  Windows-gcc  32   never
#  Windows-gcc  64   never
#  Windows-msvc 32   2012-03-30 (Exe can be launched. No more testing.)
#  Windows-msvc 64   never


CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5 FATAL_ERROR)

SET(PROJECT_NAME Pioneer)
project( ${PROJECT_NAME} )
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 20)
SET(${PROJECT_NAME}_PATCH_VERSION 0)
SET(${PROJECT_NAME}_BUILD_VERSION 0)
SET(${PROJECT_NAME}_VERSION_QUALITY "a")		# "dev"=Pre-alpha, "a"=Alpha, "b"=Beta, "RC"=Release Candidate, ""=Stable (No other extension is allowed, since it may be used directly in code)

#SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION}.${${PROJECT_NAME}_BUILD_VERSION})
#SET(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION}.${${PROJECT_NAME}_BUILD_VERSION}${${PROJECT_NAME}_VERSION_QUALITY})

# We have some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

include( "${PROJECT_SOURCE_DIR}/CMakeModules/AddDepend.cmake" )
include( "${PROJECT_SOURCE_DIR}/CMakeModules/Warnings.cmake" )

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
	OPTION(${PROJECT_NAME}_BYPASS_REVISION_RETREIVAL "Set to ON to use skip getting the current revision from the GIT repository if you find it too slow." OFF)
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
	set (GIT_SHA "Unknown revision")
	message("Couldn't get Git SHA. Value will be defined to '${GIT_SHA}'.")
endif()

#########################################################################
# Find dependencies

# Setup for finding shipped dependencies
set(SHIPPED_DEPENDENCIES_PATH )
if (WIN32)
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
	string(REGEX MATCH "shlwapi\\.lib" LIBTEST "${CMAKE_CXX_STANDARD_LIBRARIES}")
	if(NOT LIBTEST)
		SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} shlwapi.lib" CACHE STRING "" FORCE)
	endif()
	string(REGEX MATCH "shlwapi\\.lib" LIBTEST "${CMAKE_C_STANDARD_LIBRARIES}")
	if(NOT LIBTEST)
		SET(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} shlwapi.lib" CACHE STRING "" FORCE)
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

if(NOT WIN32)
	OPTION(${PROJECT_NAME}_USE_ccache "Set to ON to use ccache to compile." OFF)
	if (${PROJECT_NAME}_USE_ccache)
		message("ccache unsupported for now!!!")
		#find_package("ccache")
		#"ccache was requested but was not found. Build will proceed as normal"
	endif()
endif()

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


OPTION(BUILD_UNIT_TESTS "Build ${PROJECT_NAME} tests" ON)
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

SET(${PROJECT_NAME}_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/Config.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/Config.in" "${${PROJECT_NAME}_CONFIG_HEADER}")

SET(${PROJECT_NAME}_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/VersionNumber.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/VersionNumber.in" "${${PROJECT_NAME}_VERSION_HEADER}")

ADD_SUBDIRECTORY(contrib/lua)
ADD_SUBDIRECTORY(contrib/oolua)
ADD_SUBDIRECTORY(src)

# IF(BUILD_UNIT_TESTS)
	# ADD_SUBDIRECTORY(tests)
# ENDIF()

# OPTION(BUILD_APPLICATIONS "Build applications" ON)
# IF(BUILD_APPLICATIONS)
	# ADD_SUBDIRECTORY(Applications)
# ENDIF()



################################################################################
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
# CPack
# present the packaging option only if we have the cpack command defined (effectively >= 2.6.0)
FUNCTION(PACK_DATA_DIR DIR_TO_PACK RECURSE)
	# TODO See if INSTALL(DIRECTORY ...) could be nicer
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

IF(CMAKE_CPACK_COMMAND)
    OPTION(BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" ON)
    IF(BUILD_PACKAGES)
		INCLUDE("CMakeModules/CPackProject.cmake" REQUIRED)

		# Data directory
		SET(${PROJECT_NAME}_DATA_DIR "" CACHE PATH "Where Pioneer expects its data files to be [data]. Packagers MUST set this!")
		if(NOT ${PROJECT_NAME}_DATA_DIR)
			SET(${PROJECT_NAME}_DATA_DIR "data")
			message("Using the default value for ${PROJECT_NAME}_DATA_DIR. Installing is NOT recommended. IF YOU ARE PACKAGING PIONEER, SET THIS VARIABLE!")
		endif()

		# Add files to pack
		INSTALL(FILES
			AUTHORS.txt
			Changelog.txt
			COPYING.txt
			fonts.COPYING.txt
			lua.COPYING.txt
			Modelviewer.txt
			music.COPYING.txt
			Quickstart.txt
			README.txt
			DESTINATION ".")

		# Add directories to pack
		PACK_DATA_DIR("${PROJECT_NAME}_DATA_DIR" ON)		# You may comment it to speed up install process by skipping data. This helps debugging but you'll need to uncomment it afterwards :)

    ENDIF()
ENDIF()

################################################################################
# This needs to be run very last so other parts of the scripts can take advantage of this.
IF(NOT ${PROJECT_NAME}_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(${PROJECT_NAME}_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF()
