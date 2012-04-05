# By Sukender (Benoit NEIL), under the terms of the WTFPL

set(FINDNAME NaturalDocs)

find_program(${FINDNAME} NAMES naturaldocs NaturalDocs)

# handle the QUIETLY and REQUIRED arguments and set XXX_FOUND to TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(${FINDNAME} DEFAULT_MSG ${FINDNAME}_GCC ${FINDNAME}_GXX)
