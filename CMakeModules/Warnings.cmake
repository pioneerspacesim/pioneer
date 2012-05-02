################################################################################
# Compiler warnings handling
# Copied from OSG's CMakeLists.txt

IF(WIN32)
    IF(MSVC)
		# Note that code below do not affect CACHE variable, but memory variable.

		# This option is to enable the /MP switch for Visual Studio 2005 and above compilers
        OPTION(WIN32_USE_MP "Set to ON to build with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF()

		# Option to toggle incremental build (Useful to turn it off for VC9 (linker often crashes with incremental build))
		OPTION(${PROJECT_NAME}_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build without incremental linking in debug (release is off by default)" OFF)
		MARK_AS_ADVANCED(${PROJECT_NAME}_MSVC_DEBUG_INCREMENTAL_LINK)
		IF(NOT ${PROJECT_NAME}_MSVC_DEBUG_INCREMENTAL_LINK)
			SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
			SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
			SET(CMAKE_EXE_LINKER_FLAGS_DEBUG    "/debug /INCREMENTAL:NO")
		ENDIF()

		# # turn off various warnings
        # FOREACH(warning 4244 4251 4267 4275 4290 4786 4305 4996)
            # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}")
        # ENDFOREACH()
		# 4290: throw() specifications
		# 4512: Assignment operator could not be generated
		# 4996: secure no deprecate (for some boost 1.44 includes)
        FOREACH(warning 4290 4512 4996)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}")
        ENDFOREACH()

        # # More MSVC specific compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

		# Also replace /EHsc (Exception handling is synchronous and in C++ only) with /EHa (Accept asynchronous exceptions, and in C too)
		# Note that /GX is deprecated in Visual C++ 2005 and replaced with /EH
		OPTION(${PROJECT_NAME}_MSVC_ASYNCHRONOUS_EXCEPTIONS "Set to ON to force use of both C and C++, and both synchronous and asynchronous exceptions (forces CMAKE_CXX_FLAGS to /EHa)." ON)
		MARK_AS_ADVANCED(${PROJECT_NAME}_MSVC_ASYNCHRONOUS_EXCEPTIONS)
		IF(${PROJECT_NAME}_MSVC_ASYNCHRONOUS_EXCEPTIONS)
			string(REGEX REPLACE "/EH[sca]* *" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")		# Remove all /EH options
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
		ENDIF()
	ENDIF()
ENDIF()


# This is for an advanced option to give aggressive warnings
# under different compilers. If yours is not implemented, this option
# will not be made available.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # To be complete, we might also do GNUCC flags,
    # but everything here is C++ code.
    # -Wshadow and -Woverloaded-virtual are also interesting flags, but this
    # returns too many hits.
    # FYI, if we do implement GNUCC, then -Wmissing-prototypes in another
    # interesting C-specific flag.
    # Also, there is a bug in gcc 4.0. Under C++, -pedantic will create
    # errors instead of warnings for certain issues, including superfluous
    # semicolons and commas, and the use of long long. -fpermissive seems
    # to be the workaround.
    #SET(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -fpermissive)

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

	set(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS
		-Wformat -Wformat-security -Wstrict-aliasing=2 -Wmissing-format-attribute -Wmissing-noreturn -Wdisabled-optimization -Wfloat-equal -Wshadow -Wcast-qual -Wcast-align
		-Wstrict-null-sentinel -Wold-style-cast -Wsign-promo
	)
	set(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS_C
		-Wformat -Wformat-security -Wstrict-aliasing=2 -Wmissing-format-attribute -Wmissing-noreturn -Wdisabled-optimization -Wfloat-equal -Wshadow -Wcast-qual -Wcast-align
		-Wno-format-zero-length -Werror-implicit-function-declaration
	)

    # Previous included -Wformat=2 in ${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS but had to remove it due to standard library errors

ELSE()
    IF(MSVC)
        # FIXME: What are good aggressive warning flags for Visual Studio?
        # And do we need to further subcase this for different versions of VS?
        # CMake variables: MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
        SET(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS /W4 /wd4706 /wd4127 /wd4100)


    ELSE()
        # CMake lacks an elseif, so other non-gcc, non-VS compilers need
        # to be listed below. If unhandled, ${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS should
        # remain unset.
    ENDIF()
ENDIF()

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for ${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS.
IF(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS)

    IF (APPLE)
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS OFF)
    ELSE()
        SET(DEFAULT_USE_AGGRESSIVE_WARNINGS ON)
    ENDIF()

    OPTION(${PROJECT_NAME}_USE_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" ${DEFAULT_USE_AGGRESSIVE_WARNINGS})
    MARK_AS_ADVANCED(${PROJECT_NAME}_USE_AGGRESSIVE_WARNINGS)

    IF(${PROJECT_NAME}_USE_AGGRESSIVE_WARNINGS)
        # Add flags defined by ${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS if they aren't already there
        FOREACH(flag ${${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS})
            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
            ENDIF()
        ENDFOREACH()
    ELSE()
        # Remove all flags considered aggresive
        FOREACH(flag ${${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS})
            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ENDFOREACH()
    ENDIF()
ENDIF()

# Handle C flags
IF(${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS_C)
    IF(${PROJECT_NAME}_USE_AGGRESSIVE_WARNINGS)
        # Add flags defined by ${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS_C if they aren't already there
        FOREACH(flag ${${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS_C})
            IF(NOT CMAKE_C_FLAGS MATCHES "${flag}")
                SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
            ENDIF()
        ENDFOREACH()
    ELSE()
        # Remove all flags considered aggresive
        FOREACH(flag ${${PROJECT_NAME}_AGGRESSIVE_WARNING_FLAGS_C})
            STRING(REGEX REPLACE "${flag}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        ENDFOREACH()
    ENDIF()
ENDIF()

