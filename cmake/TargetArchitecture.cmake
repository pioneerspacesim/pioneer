# This script detects supported target architectures and configures test flags
# accordingly

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(TARGET_ARCH x86)
else()
    set(TARGET_ARCH x64)
endif()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
	set(TARGET_ARCH ARM64)
endif()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	set(TARGET_ARCH ARM64)
endif()

if (TARGET_ARCH STREQUAL "x86" OR TARGET_ARCH STREQUAL "x64")
	set(PIONEER_TARGET_ARM 0)
	set(PIONEER_TARGET_INTEL 1)
endif()

if (TARGET_ARCH STREQUAL "ARM64")
	set(PIONEER_TARGET_ARM 1)
	set(PIONEER_TARGET_INTEL 0)
endif()
