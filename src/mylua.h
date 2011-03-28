#ifndef _MYLUA_H
#define _MYLUA_H

/**
 * Use this header for including lua when you need it.
 * OOLUA is also your friend.
 */

#include "oolua/oolua.h"
#include "oolua/oolua_error.h"

extern int mylua_load_lua(lua_State *L);

#endif /* MYLUA_H */
