
#include <lua.hpp>

#include "LuaPiGui.h"
#include "PiGui.h"

#include "imgui/imgui.h"
#include "lua/LuaUtils.h"

#include "imgui/imgui_internal.h"

#include <array>

const char *s_meta_name = "PiGui.SavedImguiStackInfo";

// Cleanly recover from all unterminated calls in the current window.
// If the on_begin parameter is passed, recover from all calls since the specified state.
static void ErrorCheckRecover(ImGuiStackSizes *on_begin, ImGuiErrorLogCallback log_callback, void *user_data)
{
	ImGuiContext &g = *ImGui::GetCurrentContext();
	ImGuiWindow *window = g.CurrentWindow;
#ifdef IMGUI_HAS_TABLE
	while (g.CurrentTable && (g.CurrentTable->OuterWindow == window || g.CurrentTable->InnerWindow == window)) {
		if (log_callback) log_callback(user_data, "Recovered from missing EndTable() in '%s'", g.CurrentTable->OuterWindow->Name);
		ImGui::EndTable();
		// if the table uses a child window, the current window has changed
		window = g.CurrentWindow;
	}
#endif
	IM_ASSERT(window != NULL);
	if (on_begin == NULL)
		on_begin = &g.CurrentWindowStack.back().StackSizesOnBegin;

	while (g.CurrentTabBarStack.Size > on_begin->SizeOfTabBarStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing EndTabBar() in '%s'", window->Name);
		ImGui::EndTabBar();
	}
	while (window->DC.TreeDepth > 0) {
		if (log_callback) log_callback(user_data, "Recovered from missing TreePop() in '%s'", window->Name);
		ImGui::TreePop();
	}
	while (g.GroupStack.Size > on_begin->SizeOfGroupStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing EndGroup() in '%s'", window->Name);
		ImGui::EndGroup();
	}
	while (window->IDStack.Size > on_begin->SizeOfIDStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing PopID() in '%s'", window->Name);
		ImGui::PopID();
	}
	while (g.ColorStack.Size > on_begin->SizeOfColorStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing PopStyleColor() in '%s' for ImGuiCol_%s", window->Name, ImGui::GetStyleColorName(g.ColorStack.back().Col));
		ImGui::PopStyleColor();
	}
	while (g.StyleVarStack.Size > on_begin->SizeOfStyleVarStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing PopStyleVar() in '%s'", window->Name);
		ImGui::PopStyleVar();
	}
	while (g.FontStack.Size > on_begin->SizeOfFontStack) {
		if (log_callback) log_callback(user_data, "Recovered from missing PopFont() in '%s'", window->Name);
		ImGui::PopFont();
	}
	while (g.FocusScopeStack.Size > on_begin->SizeOfFocusScopeStack + 1) {
		if (log_callback) log_callback(user_data, "Recovered from missing PopFocusScope() in '%s'", window->Name);
		ImGui::PopFocusScope();
	}
	IM_ASSERT(window == g.CurrentWindow);
}

static void RecoverToState(ImGuiStackSizes *state, ImGuiErrorLogCallback log_callback, void *user_data)
{
	ImGuiContext &g = *GImGui;
	// Pop all windows created after the save point
	while (g.CurrentWindowStack.Size > state->SizeOfWindowStack) {
		ErrorCheckRecover(NULL, log_callback, user_data);
		if (g.CurrentWindowStack.Size == 1) {
			IM_ASSERT(g.CurrentWindow->IsFallbackWindow);
			break;
		}

		ImGuiWindow *window = g.CurrentWindow;
		if (window->Flags & ImGuiWindowFlags_ChildWindow) {
			if (log_callback) log_callback(user_data, "Recovered from missing EndChild() for '%s'", window->Name);
			ImGui::EndChild();
		} else {
			if (log_callback) log_callback(user_data, "Recovered from missing End() for '%s'", window->Name);
			ImGui::End();
		}
	}

	ErrorCheckRecover(state, log_callback, user_data);
}

static int l_new_stack_info(lua_State *L)
{
	auto *savedStackInfo = static_cast<ImGuiStackSizes *>(lua_newuserdata(L, sizeof(ImGuiStackSizes)));

	// placement new to initialize this new userdata
	new (savedStackInfo) ImGuiStackSizes();
	savedStackInfo->SetToContextState(ImGui::GetCurrentContext());

	luaL_setmetatable(L, s_meta_name);
	return 1;
}

void ErrorMsgCallback(void *data, const char *str, ...)
{
	char msg[512]{ '\0' };
	va_list vl;
	va_start(vl, str);
	size_t str_end = std::min(vsnprintf(msg, 511, str, vl), 510);
	msg[str_end] = '\n';
	msg[str_end + 1] = '\0';
	va_end(vl);

	reinterpret_cast<std::string *>(data)->append(msg);
}

static int l_stack_cleanup(lua_State *L)
{
	using std::to_string;
	auto *stackInfo = static_cast<ImGuiStackSizes *>(luaL_checkudata(L, 1, s_meta_name));
	lua_pop(L, 1);

	std::string errormsg;
	RecoverToState(stackInfo, ErrorMsgCallback, &errormsg);
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
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}
