# Setup script for Pioneer installation paths

if (WIN32)
	# We don't want a 'bin' folder on Windows
	# Setup the variables we use from GNUInstallDirs on unix platforms
	set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX})
	set(CMAKE_INSTALL_DATADIR ${CMAKE_INSTALL_PREFIX})

	# Don't create a <PREFIX>/pioneer subfolder to hold the data
	set(PIONEER_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR} CACHE PATH
		"Path where pioneer's data/ folder will be installed")

	set(PIONEER_DATA_DIR ${PIONEER_INSTALL_DATADIR} CACHE PATH
		"Runtime path to load game data from")
else()
	include(GNUInstallDirs)
endif()

option(PIONEER_INSTALL_INPLACE "Should an in-place install be generated" OFF)

if (NOT PIONEER_INSTALL_DATADIR)
	set(PIONEER_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}/pioneer CACHE PATH
		"Path where pioneer's data/ folder will be installed" FORCE)
endif (NOT PIONEER_INSTALL_DATADIR)

if (NOT PIONEER_INSTALL_BINDIR)
	set(PIONEER_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH
		"Path where Pioneer's executables will be installed" FORCE)
endif (NOT PIONEER_INSTALL_BINDIR)

# If doing an in-place installation, everything is installed in the root of the prefix
if (PIONEER_INSTALL_INPLACE)
	set(PIONEER_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX})
	set(PIONEER_INSTALL_DATADIR ${CMAKE_INSTALL_PREFIX})
	# don't load data from system-wide install
	set(PIONEER_DATA_DIR "data")
endif (PIONEER_INSTALL_INPLACE)

# Expected location of game data
if (NOT PIONEER_DATA_DIR)
	set(PIONEER_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/pioneer/data CACHE PATH
		"Runtime path to load game data from" FORCE)
endif (NOT PIONEER_DATA_DIR)

file(TO_NATIVE_PATH ${PIONEER_DATA_DIR} PIONEER_DATA_RUNTIME_DIR)
