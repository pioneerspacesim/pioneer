#ifndef _MYLUA_H
#define _MYLUA_H

/**
 * Use this header for including lua when you need it.
 * OOLUA is also your friend.
 */

#include "oolua/oolua.h"
#include "oolua/oolua_error.h"

#ifdef DEBUG
# define LUA_DEBUG_START(luaptr) const int __luaStartStackDepth = lua_gettop(luaptr);
# define LUA_DEBUG_END(luaptr, expectedStackDiff) assert(__luaStartStackDepth == (lua_gettop(luaptr)-expectedStackDiff));
#else
# define LUA_DEBUG_START(luaptr)
# define LUA_DEBUG_END(luaptr, expectedStackDiff)
#endif

extern int mylua_panic(lua_State *L);
extern int mylua_load_lua(lua_State *L);

template <typename T>
static inline void push2luaWithGc(lua_State *L, T *o)
{
	// give pointer to lua, which it owns
	OOLUA::push2lua<T>(L, o, OOLUA::Lua);
}

// Copy of:
// LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname)
// with typeerror commented out
static inline void *mylua_checkudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return p;
      } else {
	lua_pop(L, 2);
      }
    }
  }
//  luaL_typerror(L, ud, tname);  /* else error */
  return NULL;  /* to avoid warnings */
}


#endif /* MYLUA_H */
