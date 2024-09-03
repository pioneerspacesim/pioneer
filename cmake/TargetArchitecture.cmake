# This script detects supported target architectures and configures test flags
# accordingly

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES x86|x64)
	set(PIONEER_TARGET_INTEL ON)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES aarch64|ARM64)
	set(PIONEER_TARGET_ARM64 ON)
endif()
