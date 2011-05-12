# Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([pioneer], [alpha-10], [pioneer-dev@pioneerspacesim.net])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 dist-xz dist-zip])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

dnl Add a flag to CFLAGS, if supported by the compiler
AC_DEFUN([PIONEER_CFLAG],
    [saved_CFLAGS="$CFLAGS"
     CFLAGS="$EXTRAFLAGS $1 $CFLAGS"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[exit (0);]])], [EXTRAFLAGS="$EXTRAFLAGS $1"], [AC_MSG_WARN([$CC does not support $1])])
     CFLAGS="$saved_CFLAGS"
    ])

EXTRAFLAGS=

dnl Always add -Wall, where supported
PIONEER_CFLAG([-Wall])

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debugging code (stack checks, debug tools features, etc)]), [EXTRAFLAGS="$EXTRAFLAGS -DDEBUG"])

dnl Add -O2 or -O0, where supported
AC_ARG_WITH([no-optimise], AS_HELP_STRING([--with-no-optimise], [Compile without optimisation]), [OPTIMISE=-O0], [OPTIMISE=-O2])
PIONEER_CFLAG([$OPTIMISE])

AC_ARG_WITH(fno-inline,  AS_HELP_STRING([--with-fno-inline], [Compile without inlining. Helps debugging segaults that occur in STL code]), [PIONEER_CFLAG([-fno-inline])])
AC_ARG_WITH(gprof,       AS_HELP_STRING([--with-gprof], [Compile with gprof profiler]), [PIONEER_CFLAG([-p])])

AC_ARG_WITH(extra-version, AS_HELP_STRING([--with-extra-version], [Add text to game version shown on splash screen]), [PIONEER_CFLAG([-DPIONEER_EXTRAVERSION=\\\"$withval\\\"])])

PKG_CHECK_MODULES(SIGC, "sigc++-2.0")
EXTRAFLAGS="$EXTRAFLAGS $SIGC_CFLAGS"
LIBS="$LIBS $SIGC_LIBS"

case "$host" in
  i686-pc-mingw32)
	LIBS="$LIBS -lglew32s -lglu32 -lopengl32"
	;;
  x86_64-pc-mingw64)
	AC_MSG_ERROR([x86-64 on MINGW is not yet supported])
	;;
  *)
	AC_CHECK_LIB(GL, glBegin, [], [AC_MSG_ERROR([GL required])])
	AC_CHECK_LIB(GLU, gluSphere, [], [AC_MSG_ERROR([GLU required])])
	AC_CHECK_LIB(GLEW, glewInit, [], [AC_MSG_ERROR([GLEW required])])
	;;
esac

AC_PATH_PROG([SDLCONFIG], [sdl-config], "no")
if test "$SDLCONFIG" = "no"; then
	AC_MSG_ERROR("sdl-config is required.")
fi
EXTRAFLAGS="$EXTRAFLAGS `$SDLCONFIG --cflags`"
LIBS="$LIBS `$SDLCONFIG --libs`"

AC_CHECK_LIB(SDL_image, IMG_Load)

AC_PATH_PROG([FTCONFIG], [freetype-config], "no")
if test "$FTCONFIG" = "no"; then
	AC_MSG_ERROR("freetype-config is required.")
fi
EXTRAFLAGS="$EXTRAFLAGS `$FTCONFIG --cflags`"
LIBS="$LIBS `$FTCONFIG --libs`"

case "$host" in
  i686-pc-mingw32|x86_64-pc-mingw64)
	;;
  *)
	EXTRAFLAGS="$EXTRAFLAGS -DLUA_USE_POSIX"
esac

# Not using vorbis yet
PKG_CHECK_MODULES(VORBIS, vorbisfile)

AC_ARG_WITH([external-liblua], AS_HELP_STRING([--with-external-liblua], [Use external liblua in place of our internal copy]), [], [with_external_liblua=no; HAVE_LUA=no])
if test "$with_external_liblua" = yes; then
  PKG_CHECK_MODULES([LUA], [lua5.1], [HAVE_LUA=yes], [AC_MSG_ERROR([External liblua was requested but is not available])])
fi
AM_CONDITIONAL([HAVE_LUA], [test "$HAVE_LUA" = yes])

EXTRAFLAGS="$EXTRAFLAGS $LUA_CFLAGS"
LIBS="$LIBS $LUA_LIBS"

case "$host" in
  i686-pc-mingw32|x86_64-pc-mingw64)
	EXTRAFLAGS="$EXTRAFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS -DGLEW_BUILD"
	LIBS="$LIBS $SDL_LIBS -lSDL_image -ltiff -lpng -ljpeg -lz $OGG_LIBS $VORBIS_LIBS -lshlwapi"
	;;
  *)
	EXTRAFLAGS="$EXTRAFLAGS $SDL_CFLAGS $OGG_FLAGS $VORBIS_FLAGS"
	LIBS="$LIBS $SDL_LIBS -lSDL_image $OGG_LIBS $VORBIS_LIBS"
	;;
esac

CFLAGS="$EXTRAFLAGS $CFLAGS"
CXXFLAGS="$EXTRAFLAGS $CXXFLAGS"

PIONEER_DATA_DIR=${PIONEER_DATA_DIR:-data}
AC_ARG_VAR([PIONEER_DATA_DIR], [where Pioneer expects its data files to be [data]])

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile src/collider/Makefile src/lua/Makefile src/oolua/Makefile osx/Makefile win32/Makefile])
AC_OUTPUT
