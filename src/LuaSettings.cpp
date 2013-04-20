// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#include "LuaSettings.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "Settings.h"
#include "graphics/Graphics.h"
#include "Lang.h"
#include "StringF.h"
#include "../contrib/lua/llimits.h"

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
    const std::vector<Graphics::VideoMode> m_videoModes = Graphics::GetAvailableVideoModes();
    lua_createtable(l, m_videoModes.size(), 0);

    int i = 1;
    for (std::vector<Graphics::VideoMode>::const_iterator
            it = m_videoModes.begin(); it != m_videoModes.end(); ++it) {
        std::string tmp = stringf(Lang::X_BY_X, formatarg("x", int(it->width)), formatarg("y", int(it->height)));
        lua_pushlstring(l, tmp.c_str(), tmp.size());
        lua_rawseti(l, -2, i);
        ++i;
    }

    LUA_DEBUG_END(l, 1);
    return 1;
}
static int l_settings_get_game_config(lua_State *l)
{
    LUA_DEBUG_START(l);
    const std::map<std::string, std::map<std::string, std::string > > &map = Pi::config->GetMap();
    for(std::map<std::string, std::map<std::string, std::string > >::const_iterator it = map.begin();
            it != map.end(); ++it) {
        const std::map<std::string, std::string> &ini = it->second;
        lua_newtable(l);
        for(std::map<std::string, std::string>::const_iterator init = ini.begin();
                init != ini.end(); ++init) {
            pi_lua_settable(l,init->first.c_str(),init->second.c_str());
        }
    }
    LUA_DEBUG_END(l, 1);
    return 1;
}
static int l_settings_get_control_headers(lua_State *l)
{
    LUA_DEBUG_START(l);
    const std::vector<std::string> headers = Pi::settings->GetControlHeaders();
    lua_newtable(l);
    int i = 1;
    for (std::vector<std::string>::const_iterator
            it = headers.begin(); it != headers.end(); ++it) {
        pi_lua_settable(l,i++,it->c_str());
    }
    LUA_DEBUG_END(l, 1);
    return 1;
}

static int l_settings_get_control_keys(lua_State *l)
{
    LUA_DEBUG_START(l);
    const std::string header = luaL_checkstring(l, 2);
    const Settings::KeyStrings t = Pi::settings->GetPrettyKeyStrings(header, Pi::settings->GetControlKeys());
    lua_newtable(l);
    for(Settings::KeyStrings::const_iterator it = t.begin(); it != t.end(); ++it) {
        pi_lua_settable(l,it->first.c_str(), it->second.c_str());
    }
    LUA_DEBUG_END(l, 1);
    return 1;
}

void LuaSettings::Register()
{
    static const luaL_Reg l_methods[] = {
        { "GetVideoModes",   l_settings_get_video_modes   },
        { "GetGameConfig",   l_settings_get_game_config   },
        { "GetControlHeaders",   l_settings_get_control_headers   },
        { "GetControlKeys", l_settings_get_control_keys },
        { 0, 0 }
    };

    static const luaL_Reg l_attrs[] = {

        { 0, 0 }
    };

    LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
    lua_setglobal(Lua::manager->GetLuaState(), "Settings");
}
