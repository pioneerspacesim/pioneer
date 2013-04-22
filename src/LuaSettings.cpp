// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#include "LuaSettings.h"
#include "LuaObject.h"
#include "Pi.h"
#include "Settings.h"



#include "LuaTable.h"
// #include "../contrib/lua/llimits.h"

#if 0
void stackdump_g(lua_State* l)
{
    int i;
    int top = lua_gettop(l);

    printf("total in stack %d\n",top);

    for (i = 1; i <= top; i++)
    {   /* repeat for each level */
        int t = lua_type(l, i);
        switch (t) {
        case LUA_TSTRING:  /* strings */
            printf("string: '%s'\n", lua_tostring(l, i));
            break;
        case LUA_TBOOLEAN:  /* booleans */
            printf("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:  /* numbers */
            printf("number: %g\n", lua_tonumber(l, i));
            break;
        default:  /* other values */
            printf("%s\n", lua_typename(l, t));
            break;
        }
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}
#endif

static int l_settings_get_video_modes(lua_State *l)
{
    LUA_DEBUG_START(l);
    
    LuaTable t(l);
    Settings::SVecType result = Pi::settings->GetVideoModes();
    t.LoadVector(result);

    LUA_DEBUG_END(l, 1);
    return 1;
}
static int l_settings_get_game_config(lua_State *l)
{
    LUA_DEBUG_START(l);

    const Settings::MapStrings ini = Pi::settings->GetGameConfig();
    LuaTable t(l);
    t.LoadMap(ini);
    
    LUA_DEBUG_END(l, 1);
    return 1;
}
static int l_settings_save_game_config(lua_State *l)
{
    LuaTable t(l, -1);
    Settings::MapStrings ini = t.GetMap<std::string,std::string>();
    if(!Pi::settings->SaveGameConfig(ini))
        return luaL_error(l, "Could not save Config");
    return 0;
}
static int l_settings_get_headers(lua_State *l)
{
    LUA_DEBUG_START(l);

    const std::vector<std::string> headers = Pi::settings->GetHeaders();
    LuaTable t(l);
    t.LoadVector(headers);
    
    LUA_DEBUG_END(l, 1);
    return 1;
}

static int l_settings_get_keys(lua_State *l)
{
    LUA_DEBUG_START(l);
    
    const std::string header = luaL_checkstring(l, 2);
    const Settings::MapStrings keystrings = Pi::settings->GetPrettyKeyStrings(header, Pi::settings->GetKeys());
    LuaTable t(l);
    t.LoadMap(keystrings);

    LUA_DEBUG_END(l, 1);
    return 1;
}

static int l_settings_get_key_function(lua_State *l)
{
    LUA_DEBUG_START(l);
    const std::string matcher = luaL_checkstring(l, 2);
    const std::string function = Pi::settings->GetFunction(matcher);
    lua_pushlstring(l, function.c_str(), function.size());
    LUA_DEBUG_END(l, 1);
    return 1;
}

void LuaSettings::Register()
{
    static const luaL_Reg l_methods[] = {
        { "GetVideoModes",   l_settings_get_video_modes   },
        { "GetGameConfig",   l_settings_get_game_config   },
	{ "SaveGameConfig",   l_settings_save_game_config   },
        { "GetHeaders",   l_settings_get_headers   },
        { "GetKeys", l_settings_get_keys },
        { "GetKeyFunction", l_settings_get_key_function },
        { 0, 0 }
    };

    static const luaL_Reg l_attrs[] = {

        { 0, 0 }
    };

    LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
    lua_setglobal(Lua::manager->GetLuaState(), "Settings");
}
