# Setup script for Pioneer installation paths

# ==============================================================================
# Install Path Setup
# ==============================================================================

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

# ==============================================================================
# Misc. Installation Steps
# ==============================================================================

list(APPEND install_txt
	"AUTHORS.txt"
	"Changelog.txt"
	"editor.txt"
	"Quickstart.txt"
	"README.md"
	"NEWS.md"
	)

install(FILES ${install_txt} DESTINATION ${PIONEER_INSTALL_DATADIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/licenses
	DESTINATION ${PIONEER_INSTALL_DATADIR})

if(APPIMAGE_BUILD)
    set(PIONEER_EXECUTABLE init_pioneer.sh)
    set(PIONEER_SH_FILE ${CMAKE_BINARY_DIR}/init_pioneer.sh)
    configure_file(init_pioneer.sh.cmakein ${PIONEER_SH_FILE} @ONLY)
    install(FILES ${PIONEER_SH_FILE}
        DESTINATION ${PIONEER_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
    )
else(APPIMAGE_BUILD)
    set(PIONEER_EXECUTABLE ${CMAKE_INSTALL_FULL_BINDIR}/pioneer)
endif(APPIMAGE_BUILD)

if (WIN32)
	configure_file(pioneer.iss.cmakein pioneer.iss @ONLY)
	if(NOT ISCC)
		set(ISCC "C:/Program Files (x86)/Inno Setup 6/ISCC.exe")
	endif(NOT ISCC)
	add_custom_target(win-installer COMMAND ${ISCC} /Q pioneer.iss)
endif (WIN32)

if (UNIX AND NOT PIONEER_INSTALL_INPLACE)
	set(PIONEER_DESKTOP_FILE ${CMAKE_BINARY_DIR}/metadata/net.pioneerspacesim.Pioneer.desktop)
	configure_file(metadata/net.pioneerspacesim.Pioneer.desktop.cmakein ${PIONEER_DESKTOP_FILE} @ONLY)
	install(FILES ${PIONEER_DESKTOP_FILE}
		DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
	)
	install(FILES metadata/net.pioneerspacesim.Pioneer.metainfo.xml
		DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
	)

	foreach(_i IN ITEMS 16 22 24 32 40 48 64 128 256)
		install(FILES ${CMAKE_BINARY_DIR}/pioneer-${_i}x${_i}.png
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${_i}x${_i}/apps
			RENAME net.pioneerspacesim.Pioneer.png
		)
	endforeach()
endif (UNIX AND NOT PIONEER_INSTALL_INPLACE)

macro(build_png SIZE BADGE)
    set(SVG ${CMAKE_SOURCE_DIR}/application-icon/badge-${BADGE}.svg)
    set(PNG pioneer-${SIZE}x${SIZE}.png)
    list(APPEND PIONEER_PNGS ${PNG})
    add_custom_command(OUTPUT ${PNG}
        DEPENDS ${SVG}
        COMMAND inkscape ${SVG}
            --export-filename=${PNG}
            --export-height=${SIZE}
            --export-width=${SIZE}
            --export-type=png
            --export-area-page
            --export-background=black
            --export-background-opacity=0.0
        COMMAND optipng -clobber ${PNG}
        VERBATIM
    )
endmacro()
build_png(256 enlarged-text)
build_png(128 enlarged-text)
build_png(64 notext-extrastars)
build_png(48 notext-extrastars)
build_png(40 notext-extrastars)
build_png(32 notext-extrastars)
build_png(24 square)
build_png(22 square)
build_png(16 square)

set(PIONEER_ICO pioneer.ico)
set(PIONEER_ICO_PNGS
    pioneer-16x16.png
    pioneer-24x24.png
    pioneer-32x32.png
    pioneer-48x48.png
    pioneer-64x64.png
    pioneer-256x256.png
)
add_custom_command(OUTPUT ${PIONEER_ICO}
    DEPENDS ${PIONEER_ICO_PNGS}
    COMMAND icotool --create --icon --output=${PIONEER_ICO} ${PIONEER_ICO_PNGS}
    VERBATIM
)

add_custom_target(generate_icons
    ALL
    DEPENDS ${PIONEER_PNGS} ${PIONEER_ICO}
)
