dnl Process this file with autoconf to create configure.

dnl ###########################################################################
dnl Setup
dnl ###########################################################################
AC_PREREQ(2.59)

AC_INIT([pioneer], [alpha-10], [pioneer-dev@pioneerspacesim.net])
dnl AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 dist-xz dist-zip])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

dnl ###########################################################################
dnl Flags and options
dnl ###########################################################################

EXTRA_CFLAGS=
EXTRA_CPPFLAGS=

dnl Add a flag to EXTRAFLAGS, if supported by the compiler
AC_DEFUN([PIONEER_CFLAG],
    [saved_CFLAGS="$CFLAGS"
     CFLAGS="$1 $CFLAGS"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[return 0;]])], [EXTRA_CFLAGS="$EXTRA_CFLAGS $1"], [AC_MSG_WARN([$CC does not support $1])])
     CFLAGS="$saved_CFLAGS"
    ])

dnl Always add -Wall, where supported
PIONEER_CFLAG([-Wall])

dnl Add -O2 or -O0, where supported
AC_ARG_WITH([no-optimise], AS_HELP_STRING([--with-no-optimise], [Compile without optimisation]), [OPTIMISE=-O0], [OPTIMISE=-O2])
PIONEER_CFLAG([$OPTIMISE])

dnl Debug stuff
AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debugging code (stack checks, debug tools features, etc)]), [EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DDEBUG"])
AC_ARG_WITH([fno-inline],  AS_HELP_STRING([--with-fno-inline], [Compile without inlining. Helps debugging segaults that occur in STL code]), [PIONEER_CFLAG([-fno-inline])])
AC_ARG_WITH([gprof],       AS_HELP_STRING([--with-gprof], [Compile with gprof profiler]), [PIONEER_CFLAG([-p])])

dnl Dist versioning
AC_ARG_WITH([extra-version], AS_HELP_STRING([--with-extra-version], [Add text to game version shown on splash screen]), [EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DPIONEER_EXTRAVERSION=\\\"$withval\\\""])

dnl Data files directory
PIONEER_DATA_DIR=${PIONEER_DATA_DIR:-data}
AC_ARG_VAR([PIONEER_DATA_DIR], [where Pioneer expects its data files to be [data]])

dnl ###########################################################################
dnl Library checks
dnl ###########################################################################

AC_DEFUN([PIONEER_CHECK_CONFIG],
    [PKG_CHECK_MODULES([$1], [$2], [], [
	AC_PATH_PROG([$3], [$4], [AC_MSG_ERROR([$4 is required])])
	][$1][_CFLAGS="`$][$3][ --cflags`"
	][$1][_LIBS="`$][$3][ --libs`"
	AC_SUBST([$1][_CFLAGS])
	AC_SUBST([$1][_LIBS])
	])
    ])

dnl FIXME: should really check for headers (if no pkgconfig)
AC_DEFUN([PIONEER_CHECK_LIB],
    [PKG_CHECK_MODULES([$1], [$2], [],
	[AC_CHECK_LIB([$3], [$4], [], [AC_MSG_ERROR([$1 is required])])
	 ][$1][_CFLAGS=
	 ][$1][_LIBS="-l$3"
	 AC_SUBST([$1][_CFLAGS])
	 AC_SUBST([$1][_LIBS])
	])
    ])

dnl Check for sigc++
PKG_CHECK_MODULES([SIGC], [sigc++-2.0])

dnl Check for OpenGL libs
case "$host" in
  i686-pc-mingw32)
	AC_SUBST([GL_CFLAGS], [])
	AC_SUBST([GL_LIBS], [-lopengl32])
	AC_SUBST([GLU_CFLAGS], [])
	AC_SUBST([GLU_LIBS], [-lglu32])
	AC_SUBST([GLEW_CFLAGS], [ -DGLEW_BUILD])
	AC_SUBST([GLEW_LIBS], [-lglew32s])
	;;
  x86_64-pc-mingw64)
	AC_MSG_ERROR([x86-64 on MINGW is not yet supported])
	;;
  *)
	PIONEER_CHECK_LIB([GL],   [gl],   [GL],   [glBegin])
	PIONEER_CHECK_LIB([GLU],  [glu],  [GLU],  [gluSphere])
	PIONEER_CHECK_LIB([GLEW], [glew], [GLEW], [glewInit])
	;;
esac

dnl Check for SDL
PIONEER_CHECK_CONFIG([SDL], [sdl], [SDLCONFIG], [sdl-config])
AC_CHECK_LIB([SDL_image], [IMG_Load])
SDL_LIBS="$SDL_LIBS -lSDL_image"

dnl Check for Freetype
PIONEER_CHECK_CONFIG([FREETYPE], [freetype2], [FTCONFIG], [freetype-config])

dnl Not using vorbis yet
PKG_CHECK_MODULES([VORBIS], [vorbisfile])

dnl Optionally use external liblua
AC_ARG_WITH([external-liblua], AS_HELP_STRING([--with-external-liblua], [Use external liblua in place of our internal copy]), [], [with_external_liblua=no; HAVE_LUA=no])
if test "$with_external_liblua" = yes; then
  PKG_CHECK_MODULES([LUA], [lua5.1], [HAVE_LUA=yes], [AC_MSG_ERROR([External liblua was requested but is not available])])
fi
AM_CONDITIONAL([HAVE_LUA], [test "$HAVE_LUA" = yes])
case "$host" in
  i686-pc-mingw32|x86_64-pc-mingw64)
	;;
  *)
	LUA_CFLAGS="$LUA_CFLAGS -DLUA_USE_POSIX"
esac

dnl ###########################################################################
dnl System-specific stuff not covered above
dnl ###########################################################################

case "$host" in
  i686-pc-mingw32|x86_64-pc-mingw64)
	MINGW_LIBS="-lshlwapi"
	;;
  *)
	MINGW_LIBS=
	;;
esac
AC_SUBST([MINGW_LIBS])

dnl ###########################################################################
dnl Finished :-)
dnl ###########################################################################

AC_SUBST([EXTRA_CFLAGS])
AC_SUBST([EXTRA_CPPFLAGS])

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile src/collider/Makefile src/lua/Makefile src/oolua/Makefile osx/Makefile win32/Makefile])
AC_OUTPUT
