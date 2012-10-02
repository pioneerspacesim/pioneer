#include "LuaDev.h"
#include "LuaObject.h"
#include "Pi.h"
#include "WorldView.h"

/*
 * Lua commands used in development & debugging
 * Everything here is subject to rapid changes
 */

/*
 * Set current camera offset to vector,
 * it will also make the camera body visible
 * (the offset will reset when switching cameras)
 *
 * Dev.SetCameraOffset(x, y, z)
 */
static int l_dev_set_camera_offset(lua_State *l)
{
	if (!Pi::worldView)
		return luaL_error(l, "Dev.SetCameraOffset only works when there is a game running");
	Camera *cam = Pi::worldView->GetActiveCamera();
	const float x = luaL_checknumber(l, 1);
	const float y = luaL_checknumber(l, 2);
	const float z = luaL_checknumber(l, 3);
	cam->SetPosition(vector3d(x, y, z));
	cam->SetBodyVisible(true);
	return 0;
}

void LuaDev::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg methods[]= {
		{ "SetCameraOffset", l_dev_set_camera_offset },
		{ 0, 0 }
	};

	luaL_newlib(l, methods);
	lua_setglobal(l, "Dev");

	LUA_DEBUG_END(l, 0);
}
