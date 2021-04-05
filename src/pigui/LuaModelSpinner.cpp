
#include "Pi.h"
#include "lua/LuaMetaType.h"
#include "lua/LuaObject.h"
#include "lua/LuaPushPull.h"
#include "lua/LuaVector.h"
#include "lua/LuaVector2.h"
#include "pigui/ModelSpinner.h"

namespace PiGui {
	namespace LuaPiguiModelSpinner {
		static int l_model_new(lua_State *l)
		{
			LuaObject<ModelSpinner>::PushToLua(new ModelSpinner());
			return 1;
		}

		static int l_model_set_model(lua_State *l, ModelSpinner *obj)
		{
			const std::string name(luaL_checkstring(l, 2));
			SceneGraph::ModelSkin *skin = LuaObject<SceneGraph::ModelSkin>::CheckFromLua(3);
			unsigned int pattern = 0;
			if (lua_gettop(l) > 3 && !lua_isnoneornil(l, 4))
				pattern = luaL_checkinteger(l, 4) - 1; // Lua counts from 1
			SceneGraph::Model *model = Pi::FindModel(name);
			obj->SetModel(model, *skin, pattern);

			return 0;
		}

	} // namespace LuaPiguiModelSpinner
} // namespace PiGui

using namespace PiGui::LuaPiguiModelSpinner;

template <>
const char *LuaObject<PiGui::ModelSpinner>::s_type = "PiGui.Modules.ModelSpinner";

template <>
void LuaObject<PiGui::ModelSpinner>::RegisterClass()
{
	using T = PiGui::ModelSpinner;

	static LuaMetaType<T> s_metaType(s_type);
	s_metaType.CreateMetaType(Lua::manager->GetLuaState());

	s_metaType.StartRecording()
		.AddCallCtor(&l_model_new)
		.AddMember("size", &T::GetSize)
		.AddMember("spinning", &T::GetSpinning, &T::SetSpinning)
		.AddFunction("setSize", &T::SetSize)
		.AddFunction("setModel", &l_model_set_model)
		.AddFunction("getTagPos", &T::GetTagPos)
		.AddFunction("draw", [](lua_State *l, T *obj) {
			obj->Render();
			obj->DrawPiGui();
			return 0;
		})
		.StopRecording();

	LuaObjectBase::CreateClass(&s_metaType);

	LUA_DEBUG_END(l, 0);
}
