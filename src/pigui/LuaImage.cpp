// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"
#include "lua/LuaPiGui.h"
#include "lua/LuaTable.h"
#include "lua/LuaVector2.h"

namespace PiGUI {

	class LuaPiguiImage {
	public:
		static int l_new(lua_State *l)
		{
			std::string filename = LuaPull<std::string>(l, 1);

			LuaObject<PiGUI::Image>::PushToLua(new PiGUI::Image(filename));
			return 1;
		}

		static int l_image_attr_texture_id(lua_State *l)
		{
			Image *i = LuaObject<PiGUI::Image>::CheckFromLua(1);
			lua_pushlightuserdata(l, reinterpret_cast<void *>(i->GetId()));

			return 1;
		}

		static int l_image_attr_texture_size(lua_State *l)
		{
			Image *i = LuaObject<PiGUI::Image>::CheckFromLua(1);
			LuaVector2::PushToLuaF(l, i->GetSize());

			return 1;
		}

		static int l_image_attr_texture_uv(lua_State *l)
		{
			Image *i = LuaObject<PiGUI::Image>::CheckFromLua(1);
			LuaVector2::PushToLuaF(l, i->GetUv());

			return 1;
		}
	};

} // namespace PiGUI

template <>
const char *LuaObject<PiGUI::Image>::s_type = "PiGui.Modules.Image";

template <>
void LuaObject<PiGUI::Image>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", PiGUI::LuaPiguiImage::l_new },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "id", PiGUI::LuaPiguiImage::l_image_attr_texture_id },
		{ "size", PiGUI::LuaPiguiImage::l_image_attr_texture_size },
		{ "uv", PiGUI::LuaPiguiImage::l_image_attr_texture_uv },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
