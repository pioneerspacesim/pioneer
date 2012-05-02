# Extension of the standard FindBost.cmake
# - Adds options:
#		Boost_USE_AUTO_LINK, to activate/deactivate autolinking via -DBOOST_ALL_NO_LIB
# - Adds variables:
#		Boost_${COMPONENT}_LIBRARIES and Boost_${COMPONENT}_INCLUDE_DIRS
# - Makes the finder be reset when Boost_USE_STATIC_LIBS changes
# - Adds -SBOOST_ALL_DYN_LINK definition if Boost_USE_STATIC_LIBS is false
#
# By Sukender (Benoit NEIL), under the terms of the WTFPL

IF(NOT DEFINED Boost_USE_STATIC_LIBS_DEFAULT)
	IF (DEFINED Boost_USE_STATIC_LIBS)
		SET(Boost_USE_STATIC_LIBS_DEFAULT ${Boost_USE_STATIC_LIBS})
	ELSE()
		SET(Boost_USE_STATIC_LIBS_DEFAULT ON)
	ENDIF()
ENDIF()
OPTION(Boost_USE_STATIC_LIBS "Set boost to use static libraries instead of dynamic" ${Boost_USE_STATIC_LIBS_DEFAULT})

IF(NOT Boost_USE_STATIC_LIBS STREQUAL Boost_USE_STATIC_LIBS_PREVIOUS)
	# Reset all to force the finder to be re-ran
	FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
		STRING(TOUPPER ${COMPONENT} COMPONENT)
		IF(Boost_${COMPONENT}_LIBRARY)
			SET(Boost_${COMPONENT}_LIBRARY "NOTFOUND" CACHE FILEPATH "")
		ENDIF()
		IF(Boost_${COMPONENT}_LIBRARY_RELEASE)
			SET(Boost_${COMPONENT}_LIBRARY_RELEASE "NOTFOUND" CACHE FILEPATH "")
		ENDIF()
		IF(Boost_${COMPONENT}_LIBRARY_DEBUG)
			SET(Boost_${COMPONENT}_LIBRARY_DEBUG "NOTFOUND" CACHE FILEPATH "")
		ENDIF()
	ENDFOREACH()
	IF(Boost_LIBRARIES)
		SET(Boost_LIBRARIES "NOTFOUND" CACHE FILEPATH "")
	ENDIF()
ENDIF()
SET(Boost_USE_STATIC_LIBS_PREVIOUS "${Boost_USE_STATIC_LIBS}" CACHE INTERNAL "Flag used to detect change of Boost_USE_STATIC_LIBS")



INCLUDE("${CMAKE_ROOT}/Modules/FindBoost.cmake")

FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
	STRING(TOUPPER ${COMPONENT} COMPONENT)
	#IF(Boost_${COMPONENT}_FOUND)		# After all, there is no harm setting the variables when the component hasn't been found.
		SET(Boost_${COMPONENT}_LIBRARIES ${Boost_${COMPONENT}_LIBRARY})
		SET(Boost_${COMPONENT}_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
	#ENDIF()
ENDFOREACH()


IF(NOT Boost_USE_STATIC_LIBS)
	ADD_DEFINITIONS( -DBOOST_ALL_DYN_LINK )		# Tell boost to use dynamic libraries
ENDIF()

IF(NOT DEFINED Boost_USE_AUTO_LINK_DEFAULT)
	IF (DEFINED Boost_USE_AUTO_LINK)
		SET(Boost_USE_AUTO_LINK_DEFAULT ${Boost_USE_AUTO_LINK})
	ELSE()
		SET(Boost_USE_AUTO_LINK_DEFAULT OFF)
	ENDIF()
ENDIF()
OPTION(Boost_USE_AUTO_LINK "Allow Boost to use AutoLink feature on supported compiler (MSVC, some gcc...)" ${Boost_USE_AUTO_LINK_DEFAULT})
IF(Boost_USE_AUTO_LINK)
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ELSE()
	ADD_DEFINITIONS( -DBOOST_ALL_NO_LIB )		# No boost auto link
ENDIF()
