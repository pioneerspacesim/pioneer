# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([pioneer], [0.alpha.1], [tomm@riseup.net])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign])

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

#AM_PATH_LIB3DS([1.2.0])
#CFLAGS="$CFLAGS $LIB3DS_CFLAGS"
#LIBS="$LIBS $LIB3DS_LIBS"

CFLAGS="-g -Wall"
AC_ARG_WITH(no-optimise, [  --with-no-optimise      Compile without optimisation], CFLAGS="$CFLAGS -O0", CFLAGS="$CFLAGS -O2")
AC_ARG_WITH(fno-inline, [  --with-fno-inline       Compile without inlining. Helps debugging segaults that occur in STL code], CFLAGS="$CFLAGS -fno-inline")
AC_ARG_WITH(gprof, [  --with-gprof            Compile with gprof profiler], CFLAGS="$CFLAGS -p")

PKG_CHECK_MODULES(SIGC, "sigc++-2.0")
CFLAGS="$CFLAGS $SIGC_CFLAGS"
LIBS="$LIBS $SIGC_LIBS"

PKG_CHECK_MODULES(FT, freetype2)
CFLAGS="$CFLAGS $FT_CFLAGS"
LIBS="$LIBS $FT_LIBS"

PKG_CHECK_MODULES(GL, gl)
PKG_CHECK_MODULES(GLU, glu)
CFLAGS="$CFLAGS $GLU_CFLAGS"
LIBS="$LIBS $GLU_LIBS"

PKG_CHECK_MODULES(GLEW, glew)
CFLAGS="$CFLAGS $GLEW_CFLAGS"
LIBS="$LIBS $GLEW_LIBS"

PKG_CHECK_MODULES(SDL, SDL_image)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

CFLAGS="$CFLAGS -DLUA_USE_POSIX"

# Not using vorbis yet
#PKG_CHECK_MODULES(OGG, ogg)
#PKG_CHECK_MODULES(VORBIS, vorbisfile)
#CFLAGS="$CFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS"
#LIBS="$LIBS $SDL_LIBS -lSDL_image $OGG_LIBS $VORBIS_LIBS"

CXXFLAGS="$CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile src/collider/Makefile src/lua/Makefile src/oolua/Makefile])
AC_OUTPUT
