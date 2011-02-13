# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([pioneer], [0.alpha.2], [tomm@riseup.net])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

CFLAGS="-g -Wall"
AC_ARG_WITH(no-optimise, [  --with-no-optimise      Compile without optimisation], CFLAGS="$CFLAGS -O0", CFLAGS="$CFLAGS -O2")
AC_ARG_WITH(fno-inline, [  --with-fno-inline       Compile without inlining. Helps debugging segaults that occur in STL code], CFLAGS="$CFLAGS -fno-inline")
AC_ARG_WITH(gprof, [  --with-gprof            Compile with gprof profiler], CFLAGS="$CFLAGS -p")

PKG_CHECK_MODULES(SIGC, "sigc++-2.0")
CFLAGS="$CFLAGS $SIGC_CFLAGS"
LIBS="$LIBS $SIGC_LIBS"

#AC_CHECK_LIB(GL, glBegin, [], [AC_MSG_ERROR([GL required])])
#AC_CHECK_LIB(GLU, gluSphere, [], [AC_MSG_ERROR([GLU required])])
#AC_CHECK_LIB(GLEW, glewInit, [], [AC_MSG_ERROR([GLEW required])])
LIBS="$LIBS -lglew32s -lglu32 -lopengl32"

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

CFLAGS="$CFLAGS" # -DLUA_USE_POSIX"

# Not using vorbis yet
PKG_CHECK_MODULES(VORBIS, vorbisfile)
CFLAGS="$CFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS -DGLEW_BUILD"
LIBS="$LIBS $SDL_LIBS -lSDL_image -ltiff -lpng -ljpeg -lz $OGG_LIBS $VORBIS_LIBS"

CXXFLAGS="$CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile src/collider/Makefile src/lua/Makefile src/oolua/Makefile])
AC_OUTPUT
