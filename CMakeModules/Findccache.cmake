# This module locates ccache version of gcc
# By Sukender (Benoit NEIL), under the terms of the WTFPL

set(FINDNAME CCACHE)

FIND_PROGRAM(${FINDNAME}_GCC "ccache/gcc" /usr/local/lib64 /usr/lib64 /lib64 /usr/local/lib /usr/lib /lib)
FIND_PROGRAM(${FINDNAME}_GXX "ccache/g++" /usr/local/lib64 /usr/lib64 /lib64 /usr/local/lib /usr/lib /lib)

#FIND_PROGRAM(${FINDNAME} ccache)

# handle the QUIETLY and REQUIRED arguments and set XXX_FOUND to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(${FINDNAME} DEFAULT_MSG ${FINDNAME}_GCC ${FINDNAME}_GXX)

#if (${FINDNAME})
#	set(${FINDNAME}_GCC "${${FINDNAME}}")
#	set(${FINDNAME}_GXX "${${FINDNAME}}")
#endif()
