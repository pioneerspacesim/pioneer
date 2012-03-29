# Extension of the standard FindOpenGL.cmake
INCLUDE("${CMAKE_ROOT}/Modules/FindOpenGL.cmake")

# Setup OPENGL_INCLUDE_DIRS (not present in CMake 2.6.3)
IF(OPENGL_FOUND)
	SET(OPENGL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
ELSE()
	SET(OPENGL_INCLUDE_DIRS)
ENDIF()
