# Available generators:
#	NSIS, STGZ (Self extracting Tar GZ), TBZ2, TGZ, TZ, ZIP,
#	Bundle, DragNDrop, PackageMaker, OSXX11, CygwinBinary, DEB, RPM
# Reference: http://www.vtk.org/Wiki/CMake:CPackPackageGenerators

# Select default packaging type
set(DEFAULT_GENERATOR "ZIP")
if(UNIX)
	set(DEFAULT_GENERATOR "TBZ2")
elseif(WIN32)
	set(DEFAULT_GENERATOR "ZIP")		# Should be 7z, but not supported within CMake for now.
elseif(APPLE)
	set(DEFAULT_GENERATOR "OSXX11")		# PackageMaker or other stuff could be used there.
endif()

# Expose CPACK_GENERATOR to the cache
set(CPACK_GENERATOR "${DEFAULT_GENERATOR}" CACHE STRING "CPack package generator type. Most important valid values are: NSIS, TBZ2, TGZ, ZIP, DEB, RPM, Bundle, OSXX11, PackageMaker.")
mark_as_advanced(CPACK_GENERATOR)

#SET(CPACK_SOURCE_GENERATOR "TGZ")

#set(${PROJECT_NAME}_FORCED_CPACK_GENERATOR "${DEFAULT_GENERATOR}" CACHE STRING "")
#if(${PROJECT_NAME}_FORCED_CPACK_GENERATOR)
#	set(DEFAULT_GENERATOR "${${PROJECT_NAME}_FORCED_CPACK_GENERATOR}")
#endif()

# Trivial ZIP-like config
SET(PACKAGE_NAME "${PROJECT_NAME}")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${PACKAGE_NAME};ALL;/")
SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${${PROJECT_NAME}_VERSION}-${CMAKE_SYSTEM_NAME}")

# Installer/bundle
SET(CPACK_DISPLAY_NAME "${PROJECT_NAME}")
SET(CPACK_PACKAGE_VENDOR "${PROJECT_NAME} team")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} is a space adventure game set in the milkyway galaxy at the turn of the 31st century.")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_PATCH_VERSION}")
SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
SET(CPACK_PACKAGE_EXECUTABLES "pioneer;Pioneer;modelviewer;Pioneer model viewer")
SET(CPACK_MONOLITHIC_INSTALL ON)		# Disables the component-based installation mechanism, so that all components are always installed.

SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING.txt")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}")

# Get source path as NSIS style if on Windows
macro(TO_NSIS_STYLE OUTVARNAME INPUT)
	FILE(TO_NATIVE_PATH "${INPUT}" ${OUTVARNAME})
	STRING(REPLACE "\\" "\\\\\\\\" ${OUTVARNAME} "${${OUTVARNAME}}")
endmacro()

if(WIN32)
	# Windows/NSIS specific
	SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_DISPLAY_NAME}")
	TO_NSIS_STYLE(CPACK_PACKAGE_ICON     "${PROJECT_SOURCE_DIR}/data/appIcon/installBranding.bmp")	# Branding image (.bmp for NSIS)
	TO_NSIS_STYLE(CPACK_NSIS_MUI_ICON    "${PROJECT_SOURCE_DIR}/data/appIcon/Pioneer.ico")		# The icon file (.ico) for the generated install program
	TO_NSIS_STYLE(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/data/appIcon/Pioneer.ico")		# The icon file (.ico) for the generated uninstall program. (doesn't work ???)
	SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")		# The arguments that will be passed to the NSIS SetCompressor command.
else()
	set(CPACK_PACKAGE_ICON "data/appIcon/Pioneer256.png")
endif()

# Bundle confirguration
if (APPLE)
	set(EXECUTABLE_NAME "pioneer")
	set(PRODUCT_NAME "${PROJECT_NAME}")
	set(MACOSX_DEPLOYMENT_TARGET "10.5.0")		# TODO: Is this the intended value?
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/osx/pioneer-Info.plist.in" "${PROJECT_BINARY_DIR}/Info.plist")
	set(CPACK_BUNDLE_NAME "${PROJECT_NAME}")
	set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
	set(CPACK_BUNDLE_PLIST "${PROJECT_BINARY_DIR}/Info.plist")
	set(CPACK_BUNDLE_STARTUP_COMMAND "pioneer")
endif()

INCLUDE(CPack)
