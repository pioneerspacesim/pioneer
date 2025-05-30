
#include <lua.hpp>

#include "LuaPiGui.h"
#include "lua/Lua.h"

#include "imgui/imgui.h"
#include "lua/LuaUtils.h"

#include "imgui/imgui_internal.h"

const char *s_meta_name = "PiGui.SavedImguiStackInfo";

static int l_new_stack_info(lua_State *L)
{
	auto *savedStackInfo = static_cast<ImGuiErrorRecoveryState *>(lua_newuserdata(L, sizeof(ImGuiErrorRecoveryState)));
	ImGui::ErrorRecoveryStoreState(savedStackInfo);

	luaL_setmetatable(L, s_meta_name);
	return 1;
}

void ErrorMsgCallback(ImGuiContext *ctx, void *data, const char *msg)
{
	reinterpret_cast<std::string *>(data)->append(msg);
}

static int l_stack_cleanup(lua_State *L)
{
	using std::to_string;
	auto *state = static_cast<ImGuiErrorRecoveryState *>(luaL_checkudata(L, 1, s_meta_name));

	ImGuiContext &g = *ImGui::GetCurrentContext();
	ImGuiErrorCallback prev_callback = g.ErrorCallback;
	void *prev_udata = g.ErrorCallbackUserData;

	std::string errormsg;
	g.ErrorCallback = &ErrorMsgCallback;
	g.ErrorCallbackUserData = &errormsg;

	ImGui::ErrorRecoveryTryToRecoverState(state);

	g.ErrorCallback = prev_callback;
	g.ErrorCallbackUserData = prev_udata;
	lua_pop(L, 1);

	if (!errormsg.empty()) {
		Log::Debug("ImGui error handler cleaned up:\n\n{}\n", errormsg);
		lua_pushstring(L, errormsg.c_str());
	}

	lua_pushboolean(L, errormsg.empty());
	return 1;
}

luaL_Reg l_stack_functions[] = {
	{ "GetImguiStack", l_new_stack_info },
	{ "CleanupImguiStack", l_stack_cleanup },
	{ NULL, NULL }
};

void PiGui::Lua::RegisterSandbox()
{
	lua_State *L = ::Lua::manager->GetLuaState();
	LUA_DEBUG_START(L);

	// Create the new metatable and the
	luaL_newmetatable(L, s_meta_name);
	lua_pop(L, 1);

	// We use this instead of lua_getglobal because PiGui is in the CoreImports table instead
	pi_lua_split_table_path(L, "PiGui");
	lua_gettable(L, -2);
	luaL_setfuncs(L, l_stack_functions, 0);
	lua_pop(L, 2);

	LUA_DEBUG_END(L, 0);
}
