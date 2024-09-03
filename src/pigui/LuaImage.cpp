// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "LuaPiGui.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"
#include "lua/LuaTable.h"
#include "lua/LuaVector2.h"

namespace PiGui {

	class LuaPiguiImage {
	public:
		static int l_new(lua_State *l)
		{
			std::string filename = LuaPull<std::string>(l, 1);

			LuaObject<PiGui::Image>::PushToLua(new PiGui::Image(filename));
			return 1;
		}

		static int l_image_attr_texture_id(lua_State *l)
		{
			Image *i = LuaObject<PiGui::Image>::CheckFromLua(1);
			lua_pushlightuserdata(l, i->GetImTextureID());

			return 1;
		}

		static int l_image_attr_texture_size(lua_State *l)
		{
			Image *i = LuaObject<PiGui::Image>::CheckFromLua(1);
			LuaVector2::PushToLuaF(l, i->GetSize());

			return 1;
		}

		static int l_image_attr_texture_uv(lua_State *l)
		{
			Image *i = LuaObject<PiGui::Image>::CheckFromLua(1);
			LuaVector2::PushToLuaF(l, i->GetUv());

			return 1;
		}
	};

} // namespace PiGui

template <>
const char *LuaObject<PiGui::Image>::s_type = "PiGui.Modules.Image";

template <>
void LuaObject<PiGui::Image>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", PiGui::LuaPiguiImage::l_new },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "id", PiGui::LuaPiguiImage::l_image_attr_texture_id },
		{ "size", PiGui::LuaPiguiImage::l_image_attr_texture_size },
		{ "uv", PiGui::LuaPiguiImage::l_image_attr_texture_uv },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
