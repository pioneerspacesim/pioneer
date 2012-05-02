# Locate TICPP library
# This module defines XXX_FOUND, XXX_INCLUDE_DIRS and XXX_LIBRARIES standard variables

# TODO: Autodetect version and find appropriate lib

SET(TARGET_NAME TICPP)

find_path( ${TARGET_NAME}_CONFIG_DIR NAME ${TARGET_NAME}Config.cmake PATHS ${${TARGET_NAME}_DIR} $ENV{${TARGET_NAME}_DIR} )
find_path( ${TARGET_NAME}_CONFIG_BIS_DIR NAME ${TARGET_NAME}-config.cmake PATHS ${${TARGET_NAME}_DIR} $ENV{${TARGET_NAME}_DIR} )
if( NOT ${${TARGET_NAME}_CONFIG_DIR} MATCHES "${TARGET_NAME}_CONFIG_DIR-NOTFOUND" )
	include( ${${TARGET_NAME}_CONFIG_DIR}/${TARGET_NAME}Config.cmake )
	set( ${TARGET_NAME}_FOUND YES )
	SET(FOUND_BY "config")
elseif( NOT ${${TARGET_NAME}_CONFIG_BIS_DIR} MATCHES "${TARGET_NAME}_CONFIG_BIS_DIR-NOTFOUND" )
	include( ${${TARGET_NAME}_CONFIG_BIS_DIR}/${TARGET_NAME}-config.cmake )
	set( ${TARGET_NAME}_FOUND YES )
	SET(FOUND_BY "config")
else()
	SET(FOUND_BY "module")
	# Try the user's environment request before anything else.

	SET(BASE_PATHS
		~/Library/Frameworks
		/Library/Frameworks
		/usr/local
		/usr
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		)

	FIND_PATH(${TARGET_NAME}_INCLUDE_DIR NAMES ticpp.h
	 	 HINTS
		$ENV{${TARGET_NAME}_DIR}
		$ENV{${TARGET_NAME}_PATH}
		$ENV{TINYXMLPP_DIR}
		$ENV{TINYXMLPP_PATH}
  		PATH_SUFFIXES include
  		 PATHS	 ${${TARGET_NAME}_PATH} ${BASE_PATHS}
	)

	FIND_LIBRARY(	${TARGET_NAME}_LIBRARY
 	 		NAMES 
			TICPP 
			ticpp 
			TICPP-2.6.2
 			 HINTS
			$ENV{${TARGET_NAME}_DIR}
			$ENV{${TARGET_NAME}_PATH}
			$ENV{TINYXMLPP_DIR}
			$ENV{TINYXMLPP_PATH}
  			PATH_SUFFIXES lib64 lib
   			PATHS	 ${${TARGET_NAME}_PATH} ${BASE_PATHS}
	)

	FIND_LIBRARY(	${TARGET_NAME}_LIBRARY_DEBUG 
  			NAMES 
			TICPPd 
			ticppd 
			TICPP-2.6.2d
 			 HINTS
			$ENV{${TARGET_NAME}_DIR}
			$ENV{${TARGET_NAME}_PATH}
			$ENV{TINYXMLPP_DIR}
			$ENV{TINYXMLPP_PATH}
  			PATH_SUFFIXES lib64 lib
   			PATHS	 ${${TARGET_NAME}_PATH} ${BASE_PATHS}
	)

	SET(${TARGET_NAME}_LIBRARIES ${${TARGET_NAME}_LIBRARY})
	SET(${TARGET_NAME}_INCLUDE_DIRS ${${TARGET_NAME}_INCLUDE_DIR})
	SET(${TARGET_NAME}_LIBRARY_DIRS ${${TARGET_NAME}_LIBRARY_DIR})

	# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if 
	# all listed variables are TRUE
	INCLUDE(FindPackageHandleStandardArgs)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(${TARGET_NAME} DEFAULT_MSG ${TARGET_NAME}_LIBRARY ${TARGET_NAME}_INCLUDE_DIR)
	INCLUDE(FindPackageTargetLibraries)
	FIND_PACKAGE_SET_STD_INCLUDE_AND_LIBS(${TARGET_NAME})

endif()

if ( ${${TARGET_NAME}_FOUND} )
	message( STATUS "${TARGET_NAME} version: ${${TARGET_NAME}_VERSION} found by ${FOUND_BY}" )
endif( ${${TARGET_NAME}_FOUND} )
