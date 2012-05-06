#  Process this file with automake to produce Makefile.in

if !HAVE_LUA

include $(top_srcdir)/Makefile.common
# cancel the VERSION define, which conflicts with an internal #define in lundump.c
# (Lua doesn't need to know Pioneer's version number anyway)
AM_CPPFLAGS += $(LUA_CFLAGS) -UVERSION

noinst_LIBRARIES = liblua.a
liblua_a_SOURCES = \
    lapi.c     lcode.c    lctype.c  ldebug.c   ldo.c      ldump.c   lfunc.c   \
    lgc.c      llex.c     lmem.c    lobject.c  lopcodes.c lparser.c lstate.c  \
    lstring.c  ltable.c   ltm.c     lundump.c  lvm.c      lzio.c    lauxlib.c \
    linit.c    lbaselib.c lbitlib.c lcorolib.c ldblib.c   liolib.c  \
    lmathlib.c loadlib.c  loslib.c  lstrlib.c  ltablib.c

noinst_HEADERS = \
    lapi.h    lauxlib.h lcode.h    lctype.h  ldebug.h  \
    ldo.h     lfunc.h   lgc.h      llex.h    llimits.h \
    lmem.h    lobject.h lopcodes.h lparser.h lstate.h  \
    lstring.h ltable.h  ltm.h      luaconf.h lua.h     \
    lua.hpp   lualib.h  lundump.h  lvm.h     lzio.h

endif
