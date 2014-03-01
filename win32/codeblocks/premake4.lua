--organizes files in virtual folders
local VPATHS = { ["Headers"] = "**.h",  ["Sources"] = {"**.cpp", "**.c"} }
local BASEDIR = "../../"
local SRCDIR = BASEDIR .. "src/"

newoption {
   trigger     = "thirdparty",
   description = "Location of third-party dependencies package (optional)",
}

newoption {
	trigger     = "noconsole",
	description = "Don't open a console window when running the game"
}

newoption {
	trigger     = "release-symbols",
	description = "Include debug symbols in release build. Makes debugging possible, results in a larger executable"
}

solution "Pioneer"
	location "build"
	configurations { "Debug", "Release" }
	language "C++"
	platforms "x32"

	local TP_DIR = _OPTIONS["thirdparty"]
	if TP_DIR ~= nil then
		includedirs {
			path.join(TP_DIR, "include"),
			path.join(TP_DIR, "include/SDL2"),
			path.join(TP_DIR, "include/freetype2") }
		libdirs { path.join(TP_DIR, "lib") }
	end
	includedirs { ".", SRCDIR, BASEDIR .. "contrib" }
	defines { "GLEW_STATIC" }
	vpaths (VPATHS)
	--std=c++11 causes errors with miniz
	buildoptions  { "-std=gnu++11" }

	configuration "Debug"
		targetdir "build/bin/Debug"
		flags { "Symbols" }

	configuration "Release"
		targetdir "build/bin/Release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		if _OPTIONS["release-symbols"] ~= nil then
			flags { "Symbols" }
		end

	--first party libs
	project "collider"
		LIBDIR = SRCDIR .. "collider/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "galaxy"
		LIBDIR = SRCDIR .. "galaxy/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "gameui"
		LIBDIR = SRCDIR .. "gameui/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "graphics"
		kind "StaticLib"
		LIBDIR = SRCDIR .. "graphics/"
		files { LIBDIR .. "**.h", LIBDIR .. "**.cpp" }
		includedirs { LIBDIR }

	project "gui" --old gui
		LIBDIR = SRCDIR .. "gui/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "scenegraph"
		LIBDIR = SRCDIR .. "scenegraph/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "text"
		LIBDIR = SRCDIR .. "text/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "terrain"
		LIBDIR = SRCDIR .. "terrain/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "ui"
		LIBDIR = SRCDIR .. "ui/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	project "win32"
		LIBDIR = SRCDIR .. "win32/"
		kind "StaticLib"
		files { LIBDIR .. "FileSystemWin32.cpp", LIBDIR .. "OSWin32.cpp" }

	--contrib libs
	project "glew"
		kind "StaticLib"
		files { BASEDIR .. "contrib/glew/*.h", BASEDIR .. "contrib/glew/*.c" }

	project "jenkins"
		kind "StaticLib"
		files { BASEDIR .. "contrib/jenkins/lookup3.*" }

	project "json"
		kind "StaticLib"
		files { BASEDIR .. "contrib/json/json*" }

	project "lua"
		LIBDIR = BASEDIR .. "contrib/lua/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.c" }
		--exclude command line compiler
		excludes { LIBDIR .. "lua.c", LIBDIR .. "luac.c" }

	project "picodds"
		LIBDIR = BASEDIR .. "contrib/PicoDDS/"
		kind "StaticLib"
		files { LIBDIR .. "*.h", LIBDIR .. "*.cpp" }

	--main application
	project "pioneer"
		kind "ConsoleApp"
		files {
			SRCDIR .. "*.cpp",
			SRCDIR .. "*.h"
		}
		excludes {
			SRCDIR .. "test*",
			SRCDIR .. "uitest.cpp",
			SRCDIR .. "textstress.cpp"
		}
		if _OPTIONS["noconsole"] ~= nil then
			linkoptions "-mwindows"
		end
		links { "galaxy", "terrain", "gameui", "collider", "gui",
			"scenegraph", "text", "ui", "graphics", "win32", "glew",
			"jenkins", "json", "lua", "picodds"
		}
		links { "mingw32", "SDL2main", "SDL2", "SDL2_Image", "png",
			"sigc-2.0.dll", "freetype", "assimp", "vorbisfile",
			"vorbis", "ogg", "opengl32", "shlwapi"
		}
		targetdir (BASEDIR)
		configuration "Debug"
			targetsuffix "_debug"

