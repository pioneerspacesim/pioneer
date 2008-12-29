# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([pioneer], [0.1.svn], [tomm@riseup.net])
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

CFLAGS="-g -O2 -Wall"

CFLAGS="$CFLAGS `pkg-config --cflags sigc++-2.0`"
LIBS="$LIBS `pkg-config --libs sigc++-2.0` -lGL -lGLU"
CFLAGS="$CFLAGS `pkg-config --cflags freetype2`"
LIBS="$LIBS `pkg-config --libs freetype2`"

PKG_CHECK_MODULES(SDL, sdl)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS -lSDL_image"

CXXFLAGS="$CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile src/sbre/Makefile src/collider/Makefile])
AC_OUTPUT
