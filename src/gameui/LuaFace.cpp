// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"

namespace GameUI {

	class LuaFace {
	public:
		static inline FaceParts::FaceDescriptor _unpack_face(lua_State *l)
		{
			FaceParts::FaceDescriptor face;

			LUA_DEBUG_START(l);

			if (lua_gettop(l) > 1) {
				LuaTable t = LuaTable(l, 2);

				face.species = t.Get<int>("FEATURE_SPECIES", -1);
				face.race = t.Get<int>("FEATURE_RACE", -1);
				face.gender = t.Get<int>("FEATURE_GENDER", -1);

				face.head = t.Get<int>("FEATURE_HEAD", -1);
				face.eyes = t.Get<int>("FEATURE_EYES", -1);
				face.nose = t.Get<int>("FEATURE_NOSE", -1);
				face.mouth = t.Get<int>("FEATURE_MOUTH", -1);
				face.hairstyle = t.Get<int>("FEATURE_HAIRSTYLE", -1);
				face.accessories = t.Get<int>("FEATURE_ACCESSORIES", -1);
				face.clothes = t.Get<int>("FEATURE_CLOTHES", -1);
				face.armour = t.Get<int>("FEATURE_ARMOUR", -1);
			}

			LUA_DEBUG_END(l, 0);

			return face;
		}

		static int l_new(lua_State *l)
		{
			UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);

			FaceParts::FaceDescriptor face = _unpack_face(l);

			Uint32 seed = 0;

			if (lua_gettop(l) > 2 && !lua_isnil(l, 3))
				seed = luaL_checkunsigned(l, 3);

			LuaObject<Face>::PushToLua(new Face(c, face, seed));
			return 1;
		}

		static int l_set_height_lines(lua_State *l)
		{
			Face *f = LuaObject<GameUI::Face>::CheckFromLua(1);
			Uint32 lines = luaL_checkinteger(l, 2);
			f->SetHeightLines(lines);
			lua_pushvalue(l, 1);
			return 1;
		}
	};

} // namespace GameUI

using namespace GameUI;

template <>
const char *LuaObject<GameUI::Face>::s_type = "UI.Game.Face";

template <>
void LuaObject<GameUI::Face>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "New", LuaFace::l_new },
		{ "SetHeightLines", LuaFace::l_set_height_lines },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::Face>::DynamicCastPromotionTest);
}
