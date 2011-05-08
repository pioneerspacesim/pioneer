# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([pioneer], [0.alpha.9], [pioneer-dev@pioneerspacesim.net])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

CFLAGS="-g -Wall"

AC_ARG_ENABLE(debug, [  --enable-debug          Enable debugging code (stack checks, debug tools features, etc)], CFLAGS="$CFLAGS -DDEBUG")

AC_ARG_WITH(no-optimise, [  --with-no-optimise      Compile without optimisation], CFLAGS="$CFLAGS -O0", CFLAGS="$CFLAGS -O2")
AC_ARG_WITH(fno-inline,  [  --with-fno-inline       Compile without inlining. Helps debugging segaults that occur in STL code], CFLAGS="$CFLAGS -fno-inline")
AC_ARG_WITH(gprof,       [  --with-gprof            Compile with gprof profiler], CFLAGS="$CFLAGS -p")

PKG_CHECK_MODULES(SIGC, "sigc++-2.0")
CFLAGS="$CFLAGS $SIGC_CFLAGS"
LIBS="$LIBS $SIGC_LIBS"

if test "x-$host" = "x-i686-pc-mingw32" ; then
	LIBS="$LIBS -lglew32s -lglu32 -lopengl32"
else
	AC_CHECK_LIB(GL, glBegin, [], [AC_MSG_ERROR([GL required])])
	AC_CHECK_LIB(GLU, gluSphere, [], [AC_MSG_ERROR([GLU required])])
	AC_CHECK_LIB(GLEW, glewInit, [], [AC_MSG_ERROR([GLEW required])])
fi

AC_PATH_PROG([SDLCONFIG], [sdl-config], "no")
if test "$SDLCONFIG" = "no"; then
	AC_MSG_ERROR("sdl-config is required.")
fi
CFLAGS="$CFLAGS `$SDLCONFIG --cflags`"
LIBS="$LIBS `$SDLCONFIG --libs`"

AC_CHECK_LIB(SDL_image, IMG_Load)

AC_PATH_PROG([FTCONFIG], [freetype-config], "no")
if test "$FTCONFIG" = "no"; then
	AC_MSG_ERROR("freetype-config is required.")
fi
CFLAGS="$CFLAGS `$FTCONFIG --cflags`"
LIBS="$LIBS `$FTCONFIG --libs`"

if test "x-$host" != "x-i686-pc-mingw32" ; then
	CFLAGS="$CFLAGS -DLUA_USE_POSIX"
fi

# Not using vorbis yet
PKG_CHECK_MODULES(VORBIS, vorbisfile)

if test "x-$host" = "x-i686-pc-mingw32" ; then
	CFLAGS="$CFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS -DGLEW_BUILD"
	LIBS="$LIBS $SDL_LIBS -lSDL_image -ltiff -lpng -ljpeg -lz $OGG_LIBS $VORBIS_LIBS -lshlwapi"
else
	CFLAGS="$CFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS"
	LIBS="$LIBS $SDL_LIBS -lSDL_image $OGG_LIBS $VORBIS_LIBS"
fi

CXXFLAGS="$CFLAGS"

PIONEER_DATA_DIR="${PIONEER_DATA_DIR:-\\\"data\\\"}"
AC_ARG_VAR([PIONEER_DATA_DIR], [where Pioneer expects its data files to be ["./data"]])

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile src/collider/Makefile src/lua/Makefile src/oolua/Makefile])
AC_OUTPUT
