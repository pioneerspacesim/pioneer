# Available generators:
#	NSIS, STGZ (Self extracting Tar GZ), TBZ2, TGZ, TZ, ZIP,
#	Bundle, DragNDrop, PackageMaker, OSXX11, CygwinBinary, DEB, RPM
# Reference: http://www.vtk.org/Wiki/CMake:CPackPackageGenerators

# Select default packaging type
set(DEFAULT_GENERATOR "ZIP")
if(UNIX)
	set(DEFAULT_GENERATOR "TBZ2")
elseif(WIN32)
	set(DEFAULT_GENERATOR "ZIP")		# Should be 7z, but not supported within CMake for now
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

# Trivial ZIP-like config:
SET(PACKAGE_NAME "${PROJECT_NAME}")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${PACKAGE_NAME};ALL;/")
SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${${PROJECT_NAME}_VERSION}-${CMAKE_SYSTEM_NAME}")
INCLUDE(CPack)

# Installer/bundle
if(WIN32)
	set(CPACK_PACKAGE_ICON "data/appIcon/Pioneer.ico")
else()
	set(CPACK_PACKAGE_ICON "data/appIcon/Pioneer256.png")
endif()

# Bundle confirguration
set(EXECUTABLE_NAME "pioneer")
set(PRODUCT_NAME "${PROJECT_NAME}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/osx/pioneer-Info.plist.in" "${PROJECT_BINARY_DIR}/Info.plist")
set(CPACK_BUNDLE_NAME "${PROJECT_NAME}")
set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
set(CPACK_BUNDLE_PLIST "${PROJECT_BINARY_DIR}/Info.plist")
set(CPACK_BUNDLE_STARTUP_COMMAND "pioneer")
