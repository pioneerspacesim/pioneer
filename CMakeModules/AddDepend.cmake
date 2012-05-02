# Functions used to easily add a list of dependencies

# Adds include directories for a list of libraries found with FIND_PACKAGE
FUNCTION(INCLUDE_WITH_VARIABLES)
	FOREACH(CUR_DEPENDENCY ${ARGN})
		INCLUDE_DIRECTORIES(${${CUR_DEPENDENCY}_INCLUDE_DIRS})
		if ( VERBOSE )
			MESSAGE("Include-${CUR_DEPENDENCY}: ${${CUR_DEPENDENCY}_INCLUDE_DIRS}")
		endif()
	ENDFOREACH()
ENDFUNCTION()

# Adds include and link directories for a list of libraries found with FIND_PACKAGE
# This is required BEFORE "add_executable()" or "add_library()" commands.
FUNCTION(LINK_DIRS_WITH_VARIABLES)
	INCLUDE_WITH_VARIABLES(${ARGN})
	FOREACH(CUR_DEPENDENCY ${ARGN})
		LINK_DIRECTORIES( ${${CUR_DEPENDENCY}_LIBRARY_DIRS} )
		if ( VERBOSE )
			MESSAGE("LinkDir-${CUR_DEPENDENCY}: ${${CUR_DEPENDENCY}_LIBRARY_DIRS}")
		endif()
	ENDFOREACH()
ENDFUNCTION()

# Adds includes and link properties for a list of libraries found with FIND_PACKAGE
# This is required AFTER "add_executable()" or "add_library()" commands.
FUNCTION(LINK_WITH_VARIABLES TRGTNAME)
	#INCLUDE_WITH_VARIABLES(${ARGN})
	LINK_DIRS_WITH_VARIABLES(${ARGN})
	FOREACH(CUR_DEPENDENCY ${ARGN})
		#LINK_DIRECTORIES( ${${CUR_DEPENDENCY}_LIBRARY_DIRS} )
		TARGET_LINK_LIBRARIES(${TRGTNAME} ${${CUR_DEPENDENCY}_LIBRARIES} )
		if ( VERBOSE )
			MESSAGE("Link-${TRGTNAME}->${CUR_DEPENDENCY}: ${${CUR_DEPENDENCY}_LIBRARIES}")
		endif()
	ENDFOREACH()
ENDFUNCTION()
