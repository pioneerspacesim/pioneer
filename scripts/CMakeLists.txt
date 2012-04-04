
# Run python scripts/scan_enums.py -o src/enum_table.cpp src/*.h
file(GLOB FILE_LIST RELATIVE "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src/*.h")
add_custom_target(enum
	"${PYTHON_EXECUTABLE}" "scripts/scan_enums.py" "-o" "src/enum_table.cpp" ${FILE_LIST}
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMENT "Regenerating enum table...")
# if (NOT ${PROJECT_NAME}_BUILD_ENUM_AUTO)
	# set_target_properties(enum PROPERTIES EXCLUDE_FROM_ALL ON)
# endif()

# To include it as a pre-build command, it should be:
# add_custom_command(TARGET "${TARGETNAME}" PRE_BUILD
	# COMMAND "${PYTHON_EXECUTABLE}" "scripts/scan_enums.py" "-o" "src/enum_table.cpp" "src/*.h"
	# WORKING_DIRECTORY ".."
	# COMMENT "Regenerating enum table...")
