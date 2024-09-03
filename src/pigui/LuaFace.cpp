// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "LuaPiGui.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"
#include "lua/LuaTable.h"
#include "lua/LuaVector2.h"
#include "profiler/Profiler.h"

namespace PiGui {

	class LuaPiguiFace {
	public:
		static inline FaceParts::FaceDescriptor _unpack_face(lua_State *l)
		{
			PROFILE_SCOPED()
			FaceParts::FaceDescriptor face;

			LUA_DEBUG_START(l);

			if (lua_gettop(l) > 0) {
				LuaTable t = LuaTable(l, 1);

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
			PROFILE_SCOPED()
			FaceParts::FaceDescriptor face = _unpack_face(l);

			Uint32 seed = 0;

			if (lua_gettop(l) > 1 && !lua_isnil(l, 2))
				seed = luaL_checkunsigned(l, 2);

			LuaObject<PiGui::Face>::PushToLua(new Face(face, seed));
			return 1;
		}

		static int l_face_attr_texture_id(lua_State *l)
		{
			PROFILE_SCOPED()
			Face *f = LuaObject<PiGui::Face>::CheckFromLua(1);
			lua_pushlightuserdata(l, f->GetImTextureID());
			return 1;
		}

		static int l_face_attr_texture_size(lua_State *l)
		{
			PROFILE_SCOPED()
			Face *f = LuaObject<PiGui::Face>::CheckFromLua(1);
			vector2f result = f->GetTextureSize();

			LuaVector2::PushToLuaF(l, result);
			return 1;
		}
	};

} // namespace PiGui

template <>
const char *LuaObject<PiGui::Face>::s_type = "PiGui.Modules.Face";

template <>
void LuaObject<PiGui::Face>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", PiGui::LuaPiguiFace::l_new },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "textureId", PiGui::LuaPiguiFace::l_face_attr_texture_id },
		{ "textureSize", PiGui::LuaPiguiFace::l_face_attr_texture_size },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
