
#include <lua.hpp>

#include "PiGui.h"
#include "PiGuiLua.h"

#include "lua/LuaPiGui.h"
#include "lua/LuaUtils.h"

#include "imgui/imgui_internal.h"

#include <array>

struct SavedImguiStackInfo {
	static const char *meta_name;

	uint32_t windowStackSize;
	uint32_t styleColorStack;
	uint32_t styleVarStack;
	uint32_t fontStackSize;
};

const char *SavedImguiStackInfo::meta_name = "PiGui.SavedImguiStackInfo";

static int CleanupWindowStack(SavedImguiStackInfo *stackInfo)
{
	auto &windowStack = ImGui::GetCurrentContext()->CurrentWindowStack;
	int numUnfinishedWindows = windowStack.size() - stackInfo->windowStackSize;

	// While it shouldn't be possible to get a window stack of less than the last time it was updated,
	// we want to check for it anyways to avoid causing issues down the line.
	if (numUnfinishedWindows <= 1 || !stackInfo->windowStackSize)
		return 0;

	for (int n = numUnfinishedWindows; n > 0; n--) {
		auto &wnd = windowStack.back();

		// Finish all calls to BeginGroup() just to be courteous.
		// We don't unwind the ID stack because that's per-window and doesn't affect the geometry output.
		for (size_t gS = wnd->DC.GroupStack.size(); gS > 0; gS--) {
			ImGui::EndGroup();
		}

		// Need to call EndChild() instead of End() here
		if (windowStack.back()->Flags & ImGuiWindowFlags_ChildWindow)
			ImGui::EndChild();

		// Just a regular window, close it
		else
			ImGui::End();
	}

	return numUnfinishedWindows;
}

static int CleanupStyleStack(SavedImguiStackInfo *stackInfo)
{
	auto &colorStack = ImGui::GetCurrentContext()->ColorModifiers;
	int numResetStyles = colorStack.size() - stackInfo->styleColorStack;
	if (colorStack.size() > stackInfo->styleColorStack)
		ImGui::PopStyleColor(colorStack.size() - stackInfo->styleColorStack);

	auto &varStack = ImGui::GetCurrentContext()->StyleModifiers;
	numResetStyles += varStack.size() - stackInfo->styleVarStack;
	if (varStack.size() > stackInfo->styleVarStack)
		ImGui::PopStyleVar(varStack.size() - stackInfo->styleVarStack);

	return numResetStyles;
}

static int CleanupFontStack(SavedImguiStackInfo *stackInfo)
{
	auto &fontStack = ImGui::GetCurrentContext()->FontStack;
	int numResetFonts = fontStack.size() - stackInfo->fontStackSize;

	if (numResetFonts <= 0)
		return 0;

	for (int n = numResetFonts; n > 0; n--)
		ImGui::PopFont();

	return numResetFonts;
}

void UpdateStackInfo(SavedImguiStackInfo *stackInfo)
{
	stackInfo->windowStackSize = ImGui::GetCurrentContext()->CurrentWindowStack.size();
	stackInfo->styleColorStack = ImGui::GetCurrentContext()->ColorModifiers.size();
	stackInfo->styleVarStack = ImGui::GetCurrentContext()->StyleModifiers.size();
	stackInfo->fontStackSize = ImGui::GetCurrentContext()->FontStack.size();
}

static int l_new_stack_info(lua_State *L)
{
	auto *savedStackInfo = static_cast<SavedImguiStackInfo *>(lua_newuserdata(L, sizeof(SavedImguiStackInfo)));

	// placement new to initialize this new userdata
	new (savedStackInfo) SavedImguiStackInfo;
	luaL_setmetatable(L, SavedImguiStackInfo::meta_name);

	UpdateStackInfo(savedStackInfo);
	return 1;
}

static int l_stack_cleanup(lua_State *L)
{
	using std::to_string;
	auto *stackInfo = static_cast<SavedImguiStackInfo *>(luaL_checkudata(L, 1, SavedImguiStackInfo::meta_name));
	std::array<int, 3> resetNum = {
		CleanupWindowStack(stackInfo),
		CleanupStyleStack(stackInfo),
		CleanupFontStack(stackInfo)
	};
	lua_pop(L, 1);

	if (resetNum[0] || resetNum[1] || resetNum[2]) {
		std::string errormsg =
			"Cleaned up " + to_string(resetNum[0]) + " windows, " + to_string(resetNum[1]) + " styles, and " + to_string(resetNum[2]) + " fonts.\n";

		lua_pushstring(L, errormsg.c_str());
	} else {
		lua_pushstring(L, "No imgui stack cleanup necessary.\n");
	}

	// return the new message
	return 1;
}

luaL_Reg l_stack_functions[] = {
	{ "GetImguiStack", l_new_stack_info },
	{ "CleanupImguiStack", l_stack_cleanup },
	{ NULL, NULL }
};

void PiGUI::RegisterSandbox()
{
	lua_State *L = ::Lua::manager->GetLuaState();
	LUA_DEBUG_START(L);

	// Create the new metatable and the
	luaL_newmetatable(L, SavedImguiStackInfo::meta_name);
	lua_pop(L, 1);

	// We use this instead of lua_getglobal because PiGui is in the CoreImports table instead
	pi_lua_split_table_path(L, "PiGui");
	lua_gettable(L, -2);
	luaL_setfuncs(L, l_stack_functions, 0);
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}
