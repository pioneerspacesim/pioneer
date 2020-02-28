// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"

#include "EnumStrings.h"
#include "Game.h"
#include "LuaColor.h"
#include "LuaConstants.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaVector2.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "SystemInfoView.h"
#include "WorldView.h"
#include "graphics/Graphics.h"
#include "pigui/LuaFlags.h"
#include "pigui/PiGui.h"
#include "ship/PlayerShipController.h"
#include "sound/Sound.h"
#include "ui/Context.h"

#include <iterator>
#include <numeric>

// Windows defines RegisterClass as a macro, but we don't need that here.
// undef it, to avoid including yet another header that undefs it
#undef RegisterClass

template <typename Type>
static Type parse_imgui_flags(lua_State *l, int index, LuaFlags<Type> &lookupTable)
{
	PROFILE_SCOPED()
	Type theFlags = Type(0);
	if (lua_isnumber(l, index)) {
		theFlags = static_cast<Type>(lua_tointeger(l, index));
	} else if (lua_istable(l, index)) {
		theFlags = lookupTable.LookupTable(l, index);
	} else {
		luaL_traceback(l, l, NULL, 1);
		Error("Expected a table or integer, got %s.\n%s\n", luaL_typename(l, index), lua_tostring(l, -1));
	}
	return theFlags;
}

template <typename Type>
static Type parse_imgui_enum(lua_State *l, int index, LuaFlags<Type> lookupTable)
{
	PROFILE_SCOPED()
	if (lua_isstring(l, index))
		return lookupTable.LookupEnum(l, index);
	else if (lua_isnumber(l, index))
		return static_cast<Type>(lua_tointeger(l, index));
	else {
		luaL_traceback(l, l, NULL, 1);
		Error("Expected a table or integer, got %s.\n%s\n", luaL_typename(l, index), lua_tostring(l, -1));
	}
	return static_cast<Type>(0);
}

void *pi_lua_checklightuserdata(lua_State *l, int index)
{
	PROFILE_SCOPED()
	if (lua_islightuserdata(l, index))
		return lua_touserdata(l, index);
	else
		Error("Expected light user data at index %d, but got %s.\n", index, lua_typename(l, index));
	return nullptr;
}

void pi_lua_generic_pull(lua_State *l, int index, ImVec2 &vec)
{
	PROFILE_SCOPED()
	vector2d tr = LuaPull<vector2d>(l, index);
	vec = ImVec2(tr.x, tr.y);
}

int pushOnScreenPositionDirection(lua_State *l, vector3d position)
{
	PROFILE_SCOPED()
	const int width = Graphics::GetScreenWidth();
	const int height = Graphics::GetScreenHeight();
	vector3d direction = (position - vector3d(width / 2, height / 2, 0)).Normalized();
	if (vector3d(0, 0, 0) == position || position.x < 0 || position.y < 0 || position.x > width || position.y > height || position.z > 0) {
		LuaPush<bool>(l, false);
		LuaPush<vector2d>(l, vector2d(0, 0));
		LuaPush<vector3d>(l, direction * (position.z > 0 ? -1 : 1)); // reverse direction if behind camera
	} else {
		LuaPush<bool>(l, true);
		LuaPush<vector2d>(l, vector2d(position.x, position.y));
		LuaPush<vector3d>(l, direction);
	}
	return 3;
}

static LuaFlags<ImGuiSelectableFlags_> imguiSelectableFlagsTable = {
	{ "DontClosePopups", ImGuiSelectableFlags_DontClosePopups },
	{ "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns },
	{ "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick }
};

void pi_lua_generic_pull(lua_State *l, int index, ImColor &color)
{
	PROFILE_SCOPED()
	Color tr = LuaPull<Color>(l, index);
	color = ImColor(tr.r, tr.g, tr.b, tr.a);
}

void pi_lua_generic_pull(lua_State *l, int index, ImGuiSelectableFlags_ &theflags)
{
	PROFILE_SCOPED()
	theflags = parse_imgui_flags(l, index, imguiSelectableFlagsTable);
}

int l_pigui_check_selectable_flags(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TTABLE);
	ImGuiSelectableFlags_ fl = imguiSelectableFlagsTable.LookupTable(l, 1);
	lua_pushinteger(l, fl);
	return 1;
}

static LuaFlags<ImGuiTreeNodeFlags_> imguiTreeNodeFlagsTable = {
	{ "Selected", ImGuiTreeNodeFlags_Selected },
	{ "Framed", ImGuiTreeNodeFlags_Framed },
	{ "AllowOverlapMode", ImGuiTreeNodeFlags_AllowOverlapMode },
	{ "NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen },
	{ "NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog },
	{ "DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen },
	{ "OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick },
	{ "OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow },
	{ "Leaf", ImGuiTreeNodeFlags_Leaf },
	{ "Bullet", ImGuiTreeNodeFlags_Bullet },
	{ "CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader },
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiTreeNodeFlags_ &theflags)
{
	PROFILE_SCOPED()
	theflags = parse_imgui_flags(l, index, imguiTreeNodeFlagsTable);
}

int l_pigui_check_tree_node_flags(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TTABLE);
	ImGuiTreeNodeFlags_ fl = imguiTreeNodeFlagsTable.LookupTable(l, 1);
	lua_pushinteger(l, fl);
	return 1;
}

static LuaFlags<ImGuiInputTextFlags_> imguiInputTextFlagsTable = {
	{ "CharsDecimal", ImGuiInputTextFlags_CharsDecimal },
	{ "CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal },
	{ "CharsUppercase", ImGuiInputTextFlags_CharsUppercase },
	{ "CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank },
	{ "AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll },
	{ "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue },
	{ "CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion },
	{ "CallbackHistory", ImGuiInputTextFlags_CallbackHistory },
	{ "CallbackAlways", ImGuiInputTextFlags_CallbackAlways },
	{ "CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter },
	{ "AllowTabInput", ImGuiInputTextFlags_AllowTabInput },
	{ "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine },
	{ "NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll },
	{ "AlwaysInsertMode", ImGuiInputTextFlags_AlwaysInsertMode },
	{ "ReadOnly", ImGuiInputTextFlags_ReadOnly },
	{ "Password", ImGuiInputTextFlags_Password }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiInputTextFlags_ &theflags)
{
	PROFILE_SCOPED()
	theflags = parse_imgui_flags(l, index, imguiInputTextFlagsTable);
}

int l_pigui_check_input_text_flags(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TTABLE);
	ImGuiInputTextFlags_ fl = imguiInputTextFlagsTable.LookupTable(l, 1);
	lua_pushinteger(l, fl);
	return 1;
}

static LuaFlags<ImGuiCond_> imguiSetCondTable = {
	{ "Always", ImGuiCond_Always },
	{ "Once", ImGuiCond_Once },
	{ "FirstUseEver", ImGuiCond_FirstUseEver },
	{ "Appearing", ImGuiCond_Appearing }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiCond_ &value)
{
	PROFILE_SCOPED()
	value = parse_imgui_enum(l, index, imguiSetCondTable);
}

static LuaFlags<ImGuiCol_> imguiColTable = {
	{ "Text", ImGuiCol_Text },
	{ "TextDisabled", ImGuiCol_TextDisabled },
	{ "WindowBg", ImGuiCol_WindowBg },
	{ "ChildWindowBg", ImGuiCol_ChildWindowBg },
	{ "PopupBg", ImGuiCol_PopupBg },
	{ "Border", ImGuiCol_Border },
	{ "BorderShadow", ImGuiCol_BorderShadow },
	{ "FrameBg", ImGuiCol_FrameBg },
	{ "FrameBgHovered", ImGuiCol_FrameBgHovered },
	{ "FrameBgActive", ImGuiCol_FrameBgActive },
	{ "TitleBg", ImGuiCol_TitleBg },
	{ "TitleBgCollapsed", ImGuiCol_TitleBgCollapsed },
	{ "TitleBgActive", ImGuiCol_TitleBgActive },
	{ "MenuBarBg", ImGuiCol_MenuBarBg },
	{ "ScrollbarBg", ImGuiCol_ScrollbarBg },
	{ "ScrollbarGrab", ImGuiCol_ScrollbarGrab },
	{ "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered },
	{ "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive },
	{ "CheckMark", ImGuiCol_CheckMark },
	{ "SliderGrab", ImGuiCol_SliderGrab },
	{ "SliderGrabActive", ImGuiCol_SliderGrabActive },
	{ "Button", ImGuiCol_Button },
	{ "ButtonHovered", ImGuiCol_ButtonHovered },
	{ "ButtonActive", ImGuiCol_ButtonActive },
	{ "Header", ImGuiCol_Header },
	{ "HeaderHovered", ImGuiCol_HeaderHovered },
	{ "HeaderActive", ImGuiCol_HeaderActive },
	{ "Separator", ImGuiCol_Separator },
	{ "SeparatorHovered", ImGuiCol_SeparatorHovered },
	{ "SeparatorActive", ImGuiCol_SeparatorActive },
	{ "ResizeGrip", ImGuiCol_ResizeGrip },
	{ "ResizeGripHovered", ImGuiCol_ResizeGripHovered },
	{ "ResizeGripActive", ImGuiCol_ResizeGripActive },
	{ "PlotLines", ImGuiCol_PlotLines },
	{ "PlotLinesHovered", ImGuiCol_PlotLinesHovered },
	{ "PlotHistogram", ImGuiCol_PlotHistogram },
	{ "PlotHistogramHovered", ImGuiCol_PlotHistogramHovered },
	{ "TextSelectedBg", ImGuiCol_TextSelectedBg },
	{ "ModalWindowDarkening", ImGuiCol_ModalWindowDarkening }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiCol_ &value)
{
	PROFILE_SCOPED()
	value = parse_imgui_enum(l, index, imguiColTable);
}

static LuaFlags<ImGuiStyleVar_> imguiStyleVarTable = {
	{ "Alpha", ImGuiStyleVar_Alpha },
	{ "WindowPadding", ImGuiStyleVar_WindowPadding },
	{ "WindowRounding", ImGuiStyleVar_WindowRounding },
	{ "WindowBorderSize", ImGuiStyleVar_WindowBorderSize },
	{ "WindowMinSize", ImGuiStyleVar_WindowMinSize },
	{ "ChildRounding", ImGuiStyleVar_ChildRounding },
	{ "ChildBorderSize", ImGuiStyleVar_ChildBorderSize },
	{ "FramePadding", ImGuiStyleVar_FramePadding },
	{ "FrameRounding", ImGuiStyleVar_FrameRounding },
	{ "FrameBorderSize", ImGuiStyleVar_FrameBorderSize },
	{ "ItemSpacing", ImGuiStyleVar_ItemSpacing },
	{ "ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing },
	{ "IndentSpacing", ImGuiStyleVar_IndentSpacing },
	{ "GrabMinSize", ImGuiStyleVar_GrabMinSize },
	{ "ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiStyleVar_ &value)
{
	PROFILE_SCOPED()
	value = parse_imgui_enum(l, index, imguiStyleVarTable);
}

static LuaFlags<ImGuiWindowFlags_> imguiWindowFlagsTable = {
	{ "NoTitleBar", ImGuiWindowFlags_NoTitleBar },
	{ "NoResize", ImGuiWindowFlags_NoResize },
	{ "NoMove", ImGuiWindowFlags_NoMove },
	{ "NoScrollbar", ImGuiWindowFlags_NoScrollbar },
	{ "NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse },
	{ "NoCollapse", ImGuiWindowFlags_NoCollapse },
	{ "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize },
	{ "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings },
	{ "NoInputs", ImGuiWindowFlags_NoInputs },
	{ "MenuBar", ImGuiWindowFlags_MenuBar },
	{ "HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar },
	{ "NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing },
	{ "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus },
	{ "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar },
	{ "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar },
	{ "AlwaysUseWindowPadding", ImGuiWindowFlags_AlwaysUseWindowPadding }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiWindowFlags_ &theflags)
{
	PROFILE_SCOPED()
	theflags = parse_imgui_flags(l, index, imguiWindowFlagsTable);
}

int l_pigui_check_window_flags(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TTABLE);
	ImGuiWindowFlags_ fl = imguiWindowFlagsTable.LookupTable(l, 1);
	lua_pushinteger(l, fl);
	return 1;
}

static LuaFlags<ImGuiHoveredFlags_> imguiHoveredFlagsTable = {
	{ "None", ImGuiHoveredFlags_None },
	{ "ChildWindows", ImGuiHoveredFlags_ChildWindows },
	{ "RootWindow", ImGuiHoveredFlags_RootWindow },
	{ "AnyWindow", ImGuiHoveredFlags_AnyWindow },
	{ "AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup },
	{ "AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem },
	{ "AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped },
	{ "AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled },
	{ "RectOnly", ImGuiHoveredFlags_RectOnly }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiHoveredFlags_ &theflags)
{
	PROFILE_SCOPED()
	theflags = parse_imgui_flags(l, index, imguiHoveredFlagsTable);
}

int l_pigui_check_hovered_flags(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TTABLE);
	ImGuiHoveredFlags_ fl = imguiHoveredFlagsTable.LookupTable(l, 1);
	LuaPush<ImGuiHoveredFlags_>(l, fl);
	return 1;
}

static vector2d s_center(0., 0.);

static vector2d pointOnClock(const double radius, const double hours)
{
	PROFILE_SCOPED()
	double angle = (hours / 6) * 3.14159;
	vector2d res = s_center + vector2d(radius * sin(angle), -radius * cos(angle));
	return res;
}

static vector2d pointOnClock(const vector2d &center, const double radius, const double hours)
{
	PROFILE_SCOPED()
	// Update center:
	s_center = center;
	return pointOnClock(radius, hours);
}

static void lineOnClock(const double hours, const double length, const double radius, const ImColor &color, const double thickness)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	vector2d p1 = pointOnClock(radius, hours);
	vector2d p2 = pointOnClock(radius - length, hours);
	// Type change... TODO: find a better way?
	draw_list->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), color, thickness);
}

static void lineOnClock(const vector2d &center, const double hours, const double length, const double radius, const ImColor &color, const double thickness)
{
	PROFILE_SCOPED()
	// Update center:
	s_center = center;
	lineOnClock(hours, length, radius, color, thickness);
}

static int l_pigui_pointOnClock(lua_State *l)
{
	PROFILE_SCOPED()
	const double radius = LuaPull<double>(l, 2);
	const double hours = LuaPull<double>(l, 3);
	// delay checks on first parameter
	if (lua_type(l, 1) != LUA_TNIL) {
		const vector2d center = LuaPull<vector2d>(l, 1);
		LuaPush<vector2d>(l, pointOnClock(center, radius, hours));
	} else {
		LuaPush<vector2d>(l, pointOnClock(radius, hours));
	};
	return 1;
}

static int l_pigui_lineOnClock(lua_State *l)
{
	PROFILE_SCOPED()
	const double hours = LuaPull<double>(l, 2);
	const double length = LuaPull<double>(l, 3);
	const double radius = LuaPull<double>(l, 4);
	const ImColor color = LuaPull<ImColor>(l, 5);
	const double thickness = LuaPull<double>(l, 6);
	if (lua_type(l, 1) != LUA_TNIL) {
		const vector2d center = LuaPull<vector2d>(l, 1);
		lineOnClock(center, hours, length, radius, color, thickness);
	} else {
		lineOnClock(hours, length, radius, color, thickness);
	};
	return 0;
}

/*
 * Interface: PiGui
 *
 * Various functions for the imgui UI. The below documentation are for the method names as implemented in the 'pigui' module
 *
 * Example:
 *
 * > local ui = import 'pigui/pigui.lua'
 *
 */

/* ****************************** Lua imgui functions ****************************** */
/*
 * Function: begin
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_pigui_begin(lua_State *l)
{
	PROFILE_SCOPED()
	const std::string name = LuaPull<std::string>(l, 1);
	ImGuiWindowFlags theflags = LuaPull<ImGuiWindowFlags_>(l, 2);
	bool ret = ImGui::Begin(name.c_str(), nullptr, theflags);
	LuaPush<bool>(l, ret);
	return 1;
}

/*
 * Function: columns
 *
 * Initiates drawing of a table with n columns
 *
 * > ui.columns(n_columns, label, border)
 *
 * Example:
 *
 * > ui.columns(n_columns, label, has_border)
 * > ui.text("row 1, column 1")
 * > ui.nextColumn()
 * > ui.text("row 1, column 2")
 * > ui.columns(1)
 *
 * Parameters:
 *
 *   n_columns - integer, the number of columns,
 *   label - optional string, the label of the object
 *   border - optional bool, draws a vertical border between columns. Defaults to false.
 *
 */
static int l_pigui_columns(lua_State *l)
{
	PROFILE_SCOPED()
	int columns = LuaPull<int>(l, 1);
	std::string id = LuaPull<std::string>(l, 2, "");
	bool border = LuaPull<bool>(l, 3, false);
	ImGui::Columns(columns, id.c_str(), border);
	return 0;
}

static int l_pigui_get_column_width(lua_State *l)
{
	int column_index = LuaPull<int>(l, 1, -1);
	double width = ImGui::GetColumnWidth(column_index);
	LuaPush<double>(l, width);
	return 1;
}

static int l_pigui_set_column_width(lua_State *l)
{
	int column_index = LuaPull<int>(l, 1);
	double width = LuaPull<double>(l, 2);
	ImGui::SetColumnWidth(column_index, width);
	LuaPush<double>(l, width);
	return 1;
}

static int l_pigui_set_column_offset(lua_State *l)
{
	PROFILE_SCOPED()
	int column_index = LuaPull<int>(l, 1);
	double offset_x = LuaPull<double>(l, 2);
	ImGui::SetColumnOffset(column_index, offset_x);
	return 0;
}

static int l_pigui_get_scroll_y(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush<double>(l, ImGui::GetScrollY());
	return 1;
}

/*
 * Function: progressBar
 *
 * Make a progress bar widget
 *
 * > ui.progressBar(fraction, size, overlay)
 *
 * Example:
 *
 * > ui.progressBar(0.3, Vector2(0.0, 0.0), "%")
 *
 * Parameters:
 *
 *   fraction - float, text on button
 *   size - Vector2, defining size
 *   overlay - string, text to overlay
 *
 */
static int l_pigui_progress_bar(lua_State *l)
{
	PROFILE_SCOPED()
	float fraction = LuaPull<double>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	std::string overlay = LuaPull<std::string>(l, 3);
	ImGui::ProgressBar(fraction, size, overlay.c_str());
	return 0;
}

/*
 * Function: nextColumn
 *
 * > ui.nextColumn()
 *
 * Moves drawing position to next column
 *
 * Example:
 *
 * > ui.columns(n_columns, label, has_border)
 * > ui.text("row 1, column 1")
 * > ui.nextColumn()
 * > ui.text("row 1, column 2")
 * > ui.columns(1)
 *
 */
static int l_pigui_next_column(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::NextColumn();
	return 0;
}

static int l_pigui_end(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::End();
	return 0;
}

static int l_pigui_pop_clip_rect(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	draw_list->PopClipRect();
	return 0;
}

static int l_pigui_push_clip_rect(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 min = LuaPull<ImVec2>(l, 1);
	ImVec2 max = LuaPull<ImVec2>(l, 2);
	bool intersect = LuaPull<bool>(l, 3);
	draw_list->PushClipRect(min, max, intersect);
	return 0;
}

static int l_pigui_push_clip_rect_full_screen(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRectFullScreen();
	return 0;
}

static int l_pigui_set_next_window_pos(lua_State *l)
{
	PROFILE_SCOPED()
	const ImVec2 pos = LuaPull<ImVec2>(l, 1);
	int cond = LuaPull<ImGuiCond_>(l, 2);
	ImVec2 pivot = LuaPull<ImVec2>(l, 3, ImVec2(0, 0));
	ImGui::SetNextWindowPos(pos, cond, pivot);
	return 0;
}

static int l_pigui_set_next_window_focus(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::SetNextWindowFocus();
	return 0;
}

static int l_pigui_set_window_focus(lua_State *l)
{
	PROFILE_SCOPED()
	std::string name = LuaPull<std::string>(l, 1);
	ImGui::SetWindowFocus(name.c_str());
	return 0;
}

static int l_pigui_set_next_window_size(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 size = LuaPull<ImVec2>(l, 1);
	int cond = LuaPull<ImGuiCond_>(l, 2);
	ImGui::SetNextWindowSize(size, cond);
	return 0;
}

static int l_pigui_set_next_window_size_constraints(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 min = LuaPull<ImVec2>(l, 1);
	ImVec2 max = LuaPull<ImVec2>(l, 2);
	ImGui::SetNextWindowSizeConstraints(min, max);
	return 0;
}

static int l_pigui_push_style_color(lua_State *l)
{
	PROFILE_SCOPED()
	int style = LuaPull<ImGuiCol_>(l, 1);
	ImColor color = LuaPull<ImColor>(l, 2);
	ImGui::PushStyleColor(style, static_cast<ImVec4>(color));
	return 0;
}

static int l_pigui_pop_style_color(lua_State *l)
{
	PROFILE_SCOPED()
	int num = LuaPull<int>(l, 1);
	ImGui::PopStyleColor(num);
	return 0;
}

static int l_pigui_push_style_var(lua_State *l)
{
	PROFILE_SCOPED()
	int style = LuaPull<ImGuiStyleVar_>(l, 1);

	if (lua_isnumber(l, 2)) {
		double val = LuaPull<double>(l, 2);
		ImGui::PushStyleVar(style, val);
	} else if (lua_isuserdata(l, 2)) {
		ImVec2 val = LuaPull<ImVec2>(l, 2);
		ImGui::PushStyleVar(style, val);
	}

	return 0;
}

static int l_pigui_pop_style_var(lua_State *l)
{
	PROFILE_SCOPED()
	int num = LuaPull<int>(l, 1);
	ImGui::PopStyleVar(num);
	return 0;
}

static int l_pigui_get_text_line_height(lua_State *l)
{
	LuaPush(l, ImGui::GetTextLineHeight());
	return 1;
}

static int l_pigui_get_text_line_height_with_spacing(lua_State *l)
{
	LuaPush(l, ImGui::GetTextLineHeightWithSpacing());
	return 1;
}

static int l_pigui_get_frame_height(lua_State *l)
{
	LuaPush(l, ImGui::GetFrameHeight());
	return 1;
}

static int l_pigui_get_frame_height_with_spacing(lua_State *l)
{
	LuaPush(l, ImGui::GetFrameHeightWithSpacing());
	return 1;
}

static int l_pigui_add_line(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	double thickness = LuaPull<double>(l, 4);
	draw_list->AddLine(a, b, color, thickness);
	return 0;
}

static int l_pigui_add_circle(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	int radius = LuaPull<int>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	int segments = LuaPull<int>(l, 4);
	double thickness = LuaPull<double>(l, 5);
	draw_list->AddCircle(center, radius, color, segments, thickness);
	return 0;
}

static int l_pigui_add_circle_filled(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	int radius = LuaPull<int>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	int segments = LuaPull<int>(l, 4);
	draw_list->AddCircleFilled(center, radius, color, segments);
	return 0;
}

static int l_pigui_path_arc_to(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	double radius = LuaPull<double>(l, 2);
	double amin = LuaPull<double>(l, 3);
	double amax = LuaPull<double>(l, 4);
	int segments = LuaPull<int>(l, 5);
	draw_list->PathArcTo(center, radius, amin, amax, segments);
	return 0;
}

static int l_pigui_path_stroke(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImColor color = LuaPull<ImColor>(l, 1);
	bool closed = LuaPull<bool>(l, 2);
	double thickness = LuaPull<double>(l, 3);
	draw_list->PathStroke(color, closed, thickness);
	return 0;
}

static int l_pigui_selectable(lua_State *l)
{
	PROFILE_SCOPED()
	std::string label = LuaPull<std::string>(l, 1);
	bool selected = LuaPull<bool>(l, 2);
	ImGuiSelectableFlags flags = LuaPull<ImGuiSelectableFlags_>(l, 3, ImGuiSelectableFlags_None);
	// TODO: parameter size
	bool res = ImGui::Selectable(label.c_str(), selected, flags);
	LuaPush<bool>(l, res);
	return 1;
}

/*
 * Function: text
 *
 * Draw text to screen
 *
 * > ui.text(text)
 *
 * Parameters:
 *
 *   text - string, text to print
 *
 */
static int l_pigui_text(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::Text("%s", text.c_str());
	return 0;
}

/*
 * Function: button
 *
 * Create a button
 *
 * > clicked = ui.button(text, vec_size)
 *
 * Example:
 *
 * > local x = 0
 * > if ui.button("Push me", Vector2(100,0)) then
 * >     x = 42
 * > end
 *
 * Parameters:
 *
 *   text - string, text on button
 *   vec_size - Vector2, size of button
 *
 * Return:
 *
 *   clicked - bool, true if button was clicked, else false
 *
 */
static int l_pigui_button(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	bool ret = ImGui::Button(text.c_str(), size);
	LuaPush<bool>(l, ret);
	return 1;
}

static int l_pigui_thrust_indicator(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	vector3d thr = LuaPull<vector3d>(l, 3);
	vector3d vel = LuaPull<vector3d>(l, 4);
	ImColor color = LuaPull<ImColor>(l, 5);
	int frame_padding = LuaPull<int>(l, 6);
	ImColor vel_fg = LuaPull<ImColor>(l, 7);
	ImColor vel_bg = LuaPull<ImColor>(l, 8);
	ImColor thrust_fg = LuaPull<ImColor>(l, 9);
	ImColor thrust_bg = LuaPull<ImColor>(l, 10);
	ImVec4 thrust(thr.x, thr.y, thr.z, 0);
	ImVec4 velocity(vel.x, vel.y, vel.z, 0);
	PiGui::ThrustIndicator(text.c_str(), size, thrust, velocity, color,
		frame_padding, vel_fg, vel_bg, thrust_fg, thrust_bg);
	return 0;
}

static int l_pigui_low_thrust_button(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	float level = LuaPull<int>(l, 3);
	ImColor color = LuaPull<ImColor>(l, 4);
	int frame_padding = LuaPull<int>(l, 5);
	ImColor gauge_fg = LuaPull<ImColor>(l, 6);
	ImColor gauge_bg = LuaPull<ImColor>(l, 7);
	bool ret = PiGui::LowThrustButton(text.c_str(), size, level,
		color, frame_padding, gauge_fg, gauge_bg);
	LuaPush<bool>(l, ret);
	return 1;
}

static int l_pigui_button_image_sized(lua_State *l)
{
	PROFILE_SCOPED()
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	ImVec2 imgSize = LuaPull<ImVec2>(l, 3);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 4);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 5);
	int frame_padding = LuaPull<int>(l, 6);
	ImColor bg_col = LuaPull<ImColor>(l, 7);
	ImColor tint_col = LuaPull<ImColor>(l, 8);
	bool res = PiGui::ButtonImageSized(id, size, imgSize, uv0, uv1,
		frame_padding, bg_col, tint_col);
	LuaPush<bool>(l, res);
	return 1;
}

static int l_pigui_text_wrapped(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::TextWrapped("%s", text.c_str());
	return 0;
}

/*
 * Function: textColored
 *
 * Print text, with color
 *
 * > ui.textColored(color, text)
 *
 * Example:
 *
 * > ui.textColored(Color(125, 125, 125), "A whiter shade of gray")
 *
 * Parameters:
 *
 *   color - An ImColor each element between 0-255
 *   text - string, text on button
 *
 */
static int l_pigui_text_colored(lua_State *l)
{
	PROFILE_SCOPED()
	ImColor color = LuaPull<ImColor>(l, 1);
	std::string text = LuaPull<std::string>(l, 2);
	ImGui::TextColored(color, "%s", text.c_str());
	return 0;
}

static int l_pigui_get_axisbinding(lua_State *l)
{
	PROFILE_SCOPED()
	std::string binding = "";
	if (!Pi::input.IsJoystickEnabled()) {
		lua_pushnil(l);
		return 1;
	}

	ImGuiIO io = ImGui::GetIO();

	// Escape is used to clear an existing binding
	// io.KeysDown uses scancodes, but we want to use keycodes.
	if (io.KeysDown[SDL_GetScancodeFromKey(SDLK_ESCAPE)]) {
		binding = "disabled";
		LuaPush<std::string>(l, binding);
		return 1;
	}

	// otherwise actually check the joystick

	auto joysticks = Pi::input.GetJoysticksState();

	for (auto js : joysticks) {
		std::vector<float> axes = js.second.axes;
		for (size_t a = 0; a < axes.size(); a++) {
			if (axes[a] > 0.25 || axes[a] < -0.25) {
				binding = "Joy" + Pi::input.JoystickGUIDString(js.first) + "/Axis" + std::to_string(a);
				break;
			}
		}
		if (binding.compare("")) break;
	}

	if (!binding.compare(""))
		lua_pushnil(l);
	else
		LuaPush<std::string>(l, binding);
	return 1;
}

static int l_pigui_get_keybinding(lua_State *l)
{
	PROFILE_SCOPED()
	ImGuiIO io = ImGui::GetIO();
	int key = 0;
	int mod = 0;

	std::string binding;

	// pick the first key that's currently held down
	// should there be a priority?
	for (int i = 0; i < 512; i++) {
		if (io.KeysDown[i]) {
			// io.KeysDown uses scancodes, but we need keycodes.
			key = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(i));
			break;
		}
	}

	// Escape is used to clear an existing binding
	if (key == SDLK_ESCAPE) {
		binding = "disabled";
		LuaPush<std::string>(l, binding);
		return 1;
	}

	// No modifier if the key is a modifier
	// These are all in a continous range
	if (!(key >= SDLK_LCTRL && key <= SDLK_RGUI)) {
		if (io.KeyAlt) mod |= KMOD_ALT;
		if (io.KeyShift) mod |= KMOD_SHIFT;
		if (io.KeyCtrl) mod |= KMOD_CTRL;
		if (io.KeySuper) mod |= KMOD_GUI;
	}

	// Check joysticks if no keys are held down
	if (Pi::input.IsJoystickEnabled() && (key == 0 || (key >= SDLK_LCTRL && key <= SDLK_RGUI))) {
		auto joysticks = Pi::input.GetJoysticksState();

		for (auto js : joysticks) {
			std::vector<bool> buttons = js.second.buttons;
			for (size_t b = 0; b < buttons.size(); b++) {
				if (buttons[b]) {
					binding = "Joy" + Pi::input.JoystickGUIDString(js.first) + "/Button" + std::to_string(b);
					break;
				}
			}
			for (size_t h = 0; h < js.second.hats.size(); h++) {
				if (js.second.hats[h]) {
					int hatDir = js.second.hats[h];
					switch (hatDir) {
					case SDL_HAT_LEFT:
					case SDL_HAT_RIGHT:
					case SDL_HAT_UP:
					case SDL_HAT_DOWN:
						binding = "Joy" + Pi::input.JoystickGUIDString(js.first) + "/Hat" + std::to_string(h) + "Dir" + std::to_string(js.second.hats[h]);
						break;
					default:
						continue;
					}
					break;
				}
			}
			if (binding.compare("")) break;
		}
	} else if (key != 0) {
		// hard coding is bad, but is instantiating a keybinding every frame worse?
		binding = "Key" + std::to_string(key);
		if (mod > 0) binding += "Mod" + std::to_string(mod);
	}
	if (!binding.compare(""))
		lua_pushnil(l);
	else
		LuaPush<std::string>(l, binding);
	return 1;
}

static int l_pigui_add_text(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	ImColor color = LuaPull<ImColor>(l, 2);
	std::string text = LuaPull<std::string>(l, 3);
	draw_list->AddText(center, color, text.c_str());
	return 0;
}

static int l_pigui_add_triangle(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImColor color = LuaPull<ImColor>(l, 4);
	float thickness = LuaPull<double>(l, 5);
	draw_list->AddTriangle(a, b, c, color, thickness);
	return 0;
}

static int l_pigui_add_rect(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	float rounding = LuaPull<double>(l, 4);
	int round_corners = LuaPull<int>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	draw_list->AddRect(a, b, color, rounding, round_corners, thickness);
	return 0;
}

static int l_pigui_add_bezier_curve(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 c0 = LuaPull<ImVec2>(l, 2);
	ImVec2 c1 = LuaPull<ImVec2>(l, 3);
	ImVec2 b = LuaPull<ImVec2>(l, 4);
	ImColor color = LuaPull<ImColor>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	int num_segments = LuaPull<int>(l, 7);
	draw_list->AddBezierCurve(a, c0, c1, b, color, thickness, num_segments);
	return 0;
}

static int l_pigui_add_image(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 a = LuaPull<ImVec2>(l, 2);
	ImVec2 b = LuaPull<ImVec2>(l, 3);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 4);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 5);
	ImColor color = LuaPull<ImColor>(l, 6);
	draw_list->AddImage(id, a, b, uv0, uv1, color);
	return 0;
}

static int l_pigui_add_image_quad(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 a = LuaPull<ImVec2>(l, 2);
	ImVec2 b = LuaPull<ImVec2>(l, 3);
	ImVec2 c = LuaPull<ImVec2>(l, 4);
	ImVec2 d = LuaPull<ImVec2>(l, 5);
	ImVec2 uva = LuaPull<ImVec2>(l, 6);
	ImVec2 uvb = LuaPull<ImVec2>(l, 7);
	ImVec2 uvc = LuaPull<ImVec2>(l, 8);
	ImVec2 uvd = LuaPull<ImVec2>(l, 9);
	ImColor color = LuaPull<ImColor>(l, 10);
	draw_list->AddImageQuad(id, a, b, c, d, uva, uvb, uvc, uvd, color);
	return 0;
}

static int l_pigui_add_rect_filled(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	float rounding = LuaPull<double>(l, 4);
	int round_corners = LuaPull<int>(l, 5);
	draw_list->AddRectFilled(a, b, color, rounding, round_corners);
	return 0;
}

static int l_pigui_add_quad(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImVec2 d = LuaPull<ImVec2>(l, 4);
	ImColor color = LuaPull<ImColor>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	draw_list->AddQuad(a, b, c, d, color, thickness);
	return 0;
}

static int l_pigui_add_triangle_filled(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImColor color = LuaPull<ImColor>(l, 4);
	draw_list->AddTriangleFilled(a, b, c, color);
	return 0;
}

/*
 * Function: sameLine
 *
 * Draw the next command on the same line as previous
 *
 * > ui.sameLine(pos_x, spacing_w)
 *
 * Parameters:
 *
 *   pos_x - optional float, X position for next draw command
 *   spacing_w - optional float, draw with spacing relative to previous
 *
 */
static int l_pigui_same_line(lua_State *l)
{
	PROFILE_SCOPED()
	double pos_x = LuaPull<double>(l, 1);
	double spacing_w = LuaPull<double>(l, 2);
	ImGui::SameLine(pos_x, spacing_w);
	return 0;
}

static int l_pigui_begin_group(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::BeginGroup();
	return 0;
}

static int l_pigui_end_group(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::EndGroup();
	return 0;
}

/*
 * Function: separator
 *
 * Draw a horisontal line
 *
 * > ui.separator()
 *
 */
static int l_pigui_separator(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::Separator();
	return 0;
}

static int l_pigui_spacing(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::Spacing();
	return 0;
}

/*
 * Function: dummy
 *
 * Insert dummy space
 *
 * > ui.dummy(vec)
 *
 * Parameters:
 *
 *   vec - Vector2 for the x and y invisible rectangle to insert
 *
 */
static int l_pigui_dummy(lua_State *l)
{
	PROFILE_SCOPED()
	const vector2d v1 = LuaPull<vector2d>(l, 1);
	ImVec2 size(v1.x, v1.y);
	ImGui::Dummy(size);
	return 0;
}

static int l_pigui_begin_popup(lua_State *l)
{
	PROFILE_SCOPED()
	std::string id = LuaPull<std::string>(l, 1);
	LuaPush<bool>(l, ImGui::BeginPopup(id.c_str()));
	return 1;
}

static int l_pigui_begin_popup_modal(lua_State *l)
{
	PROFILE_SCOPED()
	std::string id = LuaPull<std::string>(l, 1);
	ImGuiWindowFlags flags = LuaPull<ImGuiWindowFlags_>(l, 2, ImGuiWindowFlags_None);

	LuaPush<bool>(l, ImGui::BeginPopupModal(id.c_str(), nullptr, flags));
	return 1;
}

static int l_pigui_open_popup(lua_State *l)
{
	PROFILE_SCOPED()
	std::string id = LuaPull<std::string>(l, 1);
	ImGui::OpenPopup(id.c_str());
	return 0;
}

static int l_pigui_close_current_popup(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::CloseCurrentPopup();
	return 0;
}

static int l_pigui_end_popup(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::EndPopup();
	return 0;
}

static int l_pigui_begin_child(lua_State *l)
{
	PROFILE_SCOPED()
	std::string id = LuaPull<std::string>(l, 1);
	const vector2d v1 = LuaPull<vector2d>(l, 2);

	ImVec2 size(v1.x, v1.y);
	ImGuiWindowFlags theflags = LuaPull<ImGuiWindowFlags_>(l, 3);

	ImGui::BeginChild(id.c_str(), size, false, theflags);
	return 0;
}

static int l_pigui_end_child(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::EndChild();
	return 0;
}

static int l_pigui_is_item_hovered(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush(l, ImGui::IsItemHovered());
	return 1;
}

static int l_pigui_is_item_active(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush(l, ImGui::IsItemActive());
	return 1;
}

static int l_pigui_is_item_clicked(lua_State *l)
{
	PROFILE_SCOPED()
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsItemClicked(button));
	return 1;
}

static int l_pigui_is_mouse_released(lua_State *l)
{
	PROFILE_SCOPED()
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseReleased(button));
	return 1;
}

static int l_pigui_is_mouse_down(lua_State *l)
{
	PROFILE_SCOPED()
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseDown(button));
	return 1;
}

static int l_pigui_is_mouse_clicked(lua_State *l)
{
	PROFILE_SCOPED()
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseClicked(button));
	return 1;
}

static int l_pigui_push_font(lua_State *l)
{
	PROFILE_SCOPED()
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	std::string fontname = LuaPull<std::string>(l, 2);
	int size = LuaPull<int>(l, 3);
	ImFont *font = pigui->GetFont(fontname, size);
	if (!font) {
		LuaPush(l, false);
	} else {
		LuaPush(l, true);
		ImGui::PushFont(font);
	}
	return 1;
}

static int l_pigui_pop_font(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::PopFont();
	return 0;
}

/*
 * Function: calcTextSize
 *
 * Calculate the text size
 *
 * > size = ui.calcTextSize(text)
 *
 * Parameters:
 *
 *   text - The text we want dimensions of
 *
 * Returns:
 *
 *   size - Vector2
 *
 */
static int l_pigui_calc_text_size(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = ImGui::CalcTextSize(text.c_str());
	LuaPush<vector2d>(l, vector2d(size.x, size.y));
	return 1;
}

static int l_pigui_get_mouse_pos(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 pos = ImGui::GetMousePos();
	LuaPush(l, vector2d(pos.x, pos.y));
	return 1;
}

static int l_pigui_get_mouse_wheel(lua_State *l)
{
	PROFILE_SCOPED()
	float wheel = ImGui::GetIO().MouseWheel;
	LuaPush(l, wheel);
	return 1;
}

static int l_pigui_set_tooltip(lua_State *l)
{
	PROFILE_SCOPED()
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::SetTooltip("%s", text.c_str());
	return 0;
}

static int l_pigui_checkbox(lua_State *l)
{
	PROFILE_SCOPED()
	std::string label = LuaPull<std::string>(l, 1);
	bool checked = LuaPull<bool>(l, 2);
	bool changed = ImGui::Checkbox(label.c_str(), &checked);
	LuaPush<bool>(l, changed);
	LuaPush<bool>(l, checked);
	return 2;
}

static int l_pigui_push_item_width(lua_State *l)
{
	PROFILE_SCOPED()
	double width = LuaPull<double>(l, 1);
	ImGui::PushItemWidth(width);
	return 0;
}

static int l_pigui_pop_item_width(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::PopItemWidth();
	return 0;
}

static int l_pigui_push_id(lua_State *l)
{
	PROFILE_SCOPED()
	std::string id = LuaPull<std::string>(l, 1);
	ImGui::PushID(id.c_str());
	return 0;
}

static int l_pigui_pop_id(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::PopID();
	return 0;
}

static int l_pigui_get_window_pos(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 pos = ImGui::GetWindowPos();
	LuaPush<vector2d>(l, vector2d(pos.x, pos.y));
	return 1;
}

static int l_pigui_get_window_size(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 ws = ImGui::GetWindowSize();
	LuaPush<vector2d>(l, vector2d(ws.x, ws.y));
	return 1;
}

static int l_pigui_get_content_region(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 cra = ImGui::GetContentRegionAvail();
	LuaPush<vector2d>(l, vector2d(cra.x, cra.y));
	return 1;
}

static int l_pigui_image(lua_State *l)
{
	PROFILE_SCOPED()
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 3);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 4);
	ImColor tint_col = LuaPull<ImColor>(l, 5);
	ImGui::Image(id, size, uv0, uv1, tint_col); //,  border_col
	return 0;
}

static int l_pigui_image_button(lua_State *l)
{
	PROFILE_SCOPED()
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 3);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 4);
	int frame_padding = LuaPull<int>(l, 5);
	ImColor bg_col = LuaPull<ImColor>(l, 6);
	ImColor tint_col = LuaPull<ImColor>(l, 7);
	bool res = ImGui::ImageButton(id, size, uv0, uv1,
		frame_padding, bg_col, tint_col);
	LuaPush<bool>(l, res);
	return 1;
}

static int l_pigui_capture_mouse_from_app(lua_State *l)
{
	PROFILE_SCOPED()
	bool b = LuaPull<bool>(l, 1);
	ImGui::CaptureMouseFromApp(b);
	return 0;
}

static int l_pigui_get_mouse_clicked_pos(lua_State *l)
{
	PROFILE_SCOPED()
	int n = LuaPull<int>(l, 1);
	vector2d pos(ImGui::GetIO().MouseClickedPos[n].x, ImGui::GetIO().MouseClickedPos[n].y);
	LuaPush<vector2d>(l, pos);
	return 1;
}

TScreenSpace lua_world_space_to_screen_space(const vector3d &pos)
{
	PROFILE_SCOPED()
	const WorldView *wv = Pi::game->GetWorldView();
	const vector3d p = wv->WorldSpaceToScreenSpace(pos);
	const int width = Graphics::GetScreenWidth();
	const int height = Graphics::GetScreenHeight();
	const vector3d direction = (p - vector3d(width / 2, height / 2, 0)).Normalized();
	if (vector3d(0, 0, 0) == p || p.x < 0 || p.y < 0 || p.x > width || p.y > height || p.z > 0) {
		return TScreenSpace(false, vector2d(0, 0), direction * (p.z > 0 ? -1 : 1));
	} else {
		return TScreenSpace(true, vector2d(p.x, p.y), direction);
	}
}

TScreenSpace lua_world_space_to_screen_space(const Body *body)
{
	PROFILE_SCOPED()
	const WorldView *wv = Pi::game->GetWorldView();
	const vector3d p = wv->WorldSpaceToScreenSpace(body);
	const int width = Graphics::GetScreenWidth();
	const int height = Graphics::GetScreenHeight();
	const vector3d direction = (p - vector3d(width / 2, height / 2, 0)).Normalized();
	if (vector3d(0, 0, 0) == p || p.x < 0 || p.y < 0 || p.x > width || p.y > height || p.z > 0) {
		return TScreenSpace(false, vector2d(0, 0), direction * (p.z > 0 ? -1 : 1));
	} else {
		return TScreenSpace(true, vector2d(p.x, p.y), direction);
	}
}

bool first_body_is_more_important_than(Body *body, Body *other)
{

	Object::Type a = body->GetType();
	const SystemBody *sb_a = body->GetSystemBody();
	bool a_gas_giant = sb_a && sb_a->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT;
	bool a_planet = sb_a && sb_a->IsPlanet();
	bool a_moon = sb_a && sb_a->IsMoon();

	Object::Type b = other->GetType();
	const SystemBody *sb_b = other->GetSystemBody();
	bool b_gas_giant = sb_b && sb_b->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT;
	bool b_planet = sb_b && sb_b->IsPlanet();
	bool b_moon = sb_b && sb_b->IsMoon();

	bool result = false;

	// if type is the same, just sort alphabetically
	// planets are different, because moons are
	// less important (but don't have their own type)
	if (a == b && a != Object::Type::PLANET) result = body->GetLabel() < other->GetLabel();
	// a star is larger than any other object
	else if (a == Object::Type::STAR)
		result = true;
	// any (non-star) object is smaller than a star
	else if (b == Object::Type::STAR)
		result = false;
	// a gas giant is larger than anything but a star,
	// but remember to keep total order in mind: if both are
	// gas giants, order alphabetically
	else if (a_gas_giant)
		result = !b_gas_giant || body->GetLabel() < other->GetLabel();
	// any (non-star, non-gas giant) object is smaller than a gas giant
	else if (b_gas_giant)
		result = false;
	// between two planets or moons, alphabetic
	else if (a_planet && b_planet)
		result = body->GetLabel() < other->GetLabel();
	else if (a_moon && b_moon)
		result = body->GetLabel() < other->GetLabel();
	// a planet is larger than any non-planet
	else if (a_planet)
		result = true;
	// a non-planet is smaller than any planet
	else if (b_planet)
		result = false;
	// a moon is larger than any non-moon
	else if (a_moon)
		result = true;
	// a non-moon is smaller than any moon
	else if (b_moon)
		result = false;
	// spacestation > city > ship > hyperspace cloud > cargo body > missile > projectile
	else if (a == Object::Type::SPACESTATION)
		result = true;
	else if (b == Object::Type::SPACESTATION)
		result = false;
	else if (a == Object::Type::CITYONPLANET)
		result = true;
	else if (b == Object::Type::CITYONPLANET)
		result = false;
	else if (a == Object::Type::SHIP)
		result = true;
	else if (b == Object::Type::SHIP)
		result = false;
	else if (a == Object::Type::HYPERSPACECLOUD)
		result = true;
	else if (b == Object::Type::HYPERSPACECLOUD)
		result = false;
	else if (a == Object::Type::CARGOBODY)
		result = true;
	else if (b == Object::Type::CARGOBODY)
		result = false;
	else if (a == Object::Type::MISSILE)
		result = true;
	else if (b == Object::Type::MISSILE)
		result = false;
	else if (a == Object::Type::PROJECTILE)
		result = true;
	else if (b == Object::Type::PROJECTILE)
		result = false;
	else
		Error("don't know how to compare %i and %i\n", a, b);

	return result;
}

/*
 * Function: GetProjectedBodiesGrouped
 *
 * Returns all bodies visible on screen, grouped into clusters of bodies
 * which are close together on screen. The current combat target is always
 * kept in its own seperate group.
 *
 * > groups = Engine.pigui.GetProjectedBodiesGrouped(collapse, ship_max_distance)
 *
 * Parameters:
 *
 *   collapse - Vector2 defining the screen size of the clusters
 *
 *   ship_max_distance - ships farther away than this are not included in the result
 *
 * Returns:
 *
 *   groups - array of info records describing each group
 *
 * Fields in info record:
 *
 *   screenCoordinates - coordinates of main <Body>
 *   mainBody - the main <Body> of the group; if present in the group,
 *              the navigation target is considered the main body
 *   hasNavTarget - true if group contains the player's current navigation target
 *   multiple - true if group consists of more than one body
 *   bodies - array of all <Body> objects in the group, sorted by importance
 *
 * Availability:
 *
 *   2019-12
 *
 * Status:
 *
 *   stable
 */
static int l_pigui_get_projected_bodies_grouped(lua_State *l)
{
	PROFILE_SCOPED()
	const vector2d gap = LuaPull<vector2d>(l, 1);
	const double ship_max_distance = LuaPull<double>(l, 2);

	TSS_vector filtered;
	filtered.reserve(Pi::game->GetSpace()->GetNumBodies());

	for (Body *body : Pi::game->GetSpace()->GetBodies()) {
		if (body == Pi::game->GetPlayer()) continue;
		if (body->GetType() == Object::PROJECTILE) continue;
		if (body->GetType() == Object::SHIP &&
			body->GetPositionRelTo(Pi::player).Length() > ship_max_distance) continue;
		const TScreenSpace res = lua_world_space_to_screen_space(body); // defined in LuaPiGui.cpp
		if (!res._onScreen) continue;
		filtered.emplace_back(res);
		filtered.back()._body = body;
	}

	struct GroupInfo {
		Body *m_mainBody;
		vector2d m_screenCoords; // screen coords of group
		std::vector<Body *> m_bodies;
		bool m_hasNavTarget;

		GroupInfo(Body *b, const vector2d &coords, bool isNavTarget) :
			m_mainBody(b),
			m_screenCoords(coords),
			m_hasNavTarget(isNavTarget)
		{
			m_bodies.push_back(b);
		}
	};
	std::vector<GroupInfo> groups;
	groups.reserve(filtered.size());
	const Body *nav_target = Pi::game->GetPlayer()->GetNavTarget();
	const Body *combat_target = Pi::game->GetPlayer()->GetCombatTarget();

	for (TScreenSpace &obj : filtered) {
		bool inserted = false;

		// never collapse combat target
		if (obj._body != combat_target) {
			for (GroupInfo &group : groups) {
				if ((std::abs(group.m_screenCoords.x - obj._screenPosition.x) <= gap.x) &&
					(std::abs(group.m_screenCoords.y - obj._screenPosition.y) <= gap.y)) {
					// body inside group boundaries: insert into group
					group.m_bodies.push_back(obj._body);
					if (obj._body == nav_target) {
						group.m_hasNavTarget = true;
						// nav target becomes main body
						group.m_mainBody = obj._body;
						group.m_screenCoords = lua_world_space_to_screen_space(obj._body)._screenPosition;
					}
					inserted = true;
					break;
				}
			}
		}
		if (!inserted) {
			// create new group
			GroupInfo newgroup(obj._body, obj._screenPosition,
				obj._body == nav_target ? true : false);
			groups.push_back(std::move(newgroup));
		}
	}

	// Sort each groups member according to a given function
	for (GroupInfo &group : groups) {
		std::sort(begin(group.m_bodies), end(group.m_bodies),
			[](Body *a, Body *b) {
				return first_body_is_more_important_than(a, b);
			});
		if (!group.m_hasNavTarget) {
			// make most important body the main body
			group.m_mainBody = group.m_bodies.front();
			group.m_screenCoords = lua_world_space_to_screen_space(group.m_mainBody)._screenPosition;
		}
	}

	LuaTable result(l, groups.size(), 0);
	int index = 1;

	for (GroupInfo &group : groups) {
		LuaTable info_table(l, 0, 5);
		LuaTable bodies_table(l, group.m_bodies.size(), 0);

		info_table.Set("screenCoordinates", group.m_screenCoords);
		info_table.Set("mainBody", group.m_mainBody);
		bodies_table.LoadVector(group.m_bodies.begin(), group.m_bodies.end());
		info_table.Set("bodies", bodies_table);
		lua_pop(l, 1);
		info_table.Set("multiple", group.m_bodies.size() > 1 ? true : false);
		info_table.Set("hasNavTarget", group.m_hasNavTarget);
		result.Set(index++, info_table);
		lua_pop(l, 1);
	}
	LuaPush(l, result);
	return 1;
}

static int l_pigui_get_projected_bodies(lua_State *l)
{
	PROFILE_SCOPED()
	TSS_vector filtered;
	filtered.reserve(Pi::game->GetSpace()->GetNumBodies());
	for (Body *body : Pi::game->GetSpace()->GetBodies()) {
		if (body == Pi::game->GetPlayer()) continue;
		if (body->GetType() == Object::PROJECTILE) continue;
		const TScreenSpace res = lua_world_space_to_screen_space(body); // defined in LuaPiGui.cpp
		if (!res._onScreen) continue;
		filtered.emplace_back(res);
		filtered.back()._body = body;
	}

	LuaTable result(l, 0, filtered.size());
	for (TScreenSpace &res : filtered) {
		LuaTable object(l, 0, 3);

		object.Set("onscreen", res._onScreen);
		object.Set("screenCoordinates", res._screenPosition);
		object.Set("body", res._body);

		result.Set(res._body, object);
		lua_pop(l, 1);
	}
	LuaPush(l, result);
	return 1;
}

static int l_pigui_get_targets_nearby(lua_State *l)
{
	PROFILE_SCOPED()
	int range_max = LuaPull<double>(l, 1);
	Space::BodyNearList nearby = Pi::game->GetSpace()->GetBodiesMaybeNear(Pi::player, range_max);

	std::vector<Body *> filtered;
	filtered.reserve(nearby.size());
	for (Body *body : nearby) {
		if (body == Pi::player) continue;
		if (body->GetType() == Object::PROJECTILE) continue;
		filtered.push_back(body);
	};

	LuaTable result(l, 0, filtered.size());
	int index = 1;
	for (Body *body : filtered) {
		vector3d position = body->GetPositionRelTo(Pi::player);
		float distance = float(position.Length());
		vector3d shipSpacePosition = position * Pi::player->GetOrient();
		// convert to polar https://en.wikipedia.org/wiki/Spherical_coordinate_system
		vector3d polarPosition( // don't calculate X, it is not used
			// sqrt(shipSpacePosition.x*shipSpacePosition.x
			// 		+ shipSpacePosition.y*shipSpacePosition.y
			// 		+ shipSpacePosition.z*shipSpacePosition.z)
			0,
			atan2(shipSpacePosition.x, shipSpacePosition.y),
			atan2(-shipSpacePosition.z, sqrt(shipSpacePosition.x * shipSpacePosition.x + shipSpacePosition.y * shipSpacePosition.y)));
		// convert to AEP https://en.wikipedia.org/wiki/Azimuthal_equidistant_projection
		double rho = M_PI / 2 - polarPosition.z;
		double theta = polarPosition.y;

		vector2d aep(rho * sin(theta) / (2 * M_PI), -rho * cos(theta) / (2 * M_PI));

		LuaTable object(l, 0, 4);

		object.Set("distance", distance);
		object.Set("label", body->GetLabel());

		//		object.Set("type", EnumStrings::GetString("PhysicsObjectType", (*i)->GetType()));
		//		object.Set("position", position);
		//		object.Set("oriented_position", shipSpacePosition);
		//		object.Set("polar_position", polarPosition);

		object.Set("aep", aep);
		object.Set("body", body);
		result.Set(std::to_string(index++), object);
		lua_pop(l, 1);
	}
	LuaPush(l, result);
	return 1;
}

static int l_pigui_disable_mouse_facing(lua_State *l)
{
	PROFILE_SCOPED()
	bool b = LuaPull<bool>(l, 1);
	auto *p = Pi::player->GetPlayerController();
	p->SetDisableMouseFacing(b);
	return 0;
}

static int l_pigui_set_mouse_button_state(lua_State *l)
{
	PROFILE_SCOPED()
	int button = LuaPull<int>(l, 1);
	bool state = LuaPull<bool>(l, 2);
	Pi::input.SetMouseButtonState(button, state);
	if (state == false) {
		// new UI caches which widget should receive the mouse up event
		// after a mouse down. This function exists exactly because the mouse-up event
		// never gets delivered after imgui uses it. So reset that context as well.
		// This can go away when everything is moved to imgui.
		Pi::ui->ResetMouseActiveReceiver();
	}
	return 0;
}

static int l_pigui_should_show_labels(lua_State *l)
{
	PROFILE_SCOPED()
	bool show_labels = Pi::game->GetWorldView()->ShouldShowLabels();
	LuaPush(l, show_labels);
	return 1;
}

static int l_attr_handlers(lua_State *l)
{
	PROFILE_SCOPED()
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	pigui->GetHandlers().PushCopyToStack();
	return 1;
}

static int l_attr_keys(lua_State *l)
{
	PROFILE_SCOPED()
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	pigui->GetKeys().PushCopyToStack();
	return 1;
}

static int l_attr_screen_width(lua_State *l)
{
	PROFILE_SCOPED()
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	LuaPush<int>(l, Graphics::GetScreenWidth());
	return 1;
}

static int l_attr_key_ctrl(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush<bool>(l, ImGui::GetIO().KeyCtrl);
	return 1;
}

static int l_attr_key_none(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush<bool>(l, !ImGui::GetIO().KeyCtrl & !ImGui::GetIO().KeyShift & !ImGui::GetIO().KeyAlt);
	return 1;
}

static int l_attr_key_shift(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush<bool>(l, ImGui::GetIO().KeyShift);
	return 1;
}

static int l_attr_key_alt(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush<bool>(l, ImGui::GetIO().KeyAlt);
	return 1;
}

static int l_attr_screen_height(lua_State *l)
{
	PROFILE_SCOPED()
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	LuaPush<int>(l, Graphics::GetScreenHeight());
	return 1;
}

// TODO: the Combo API was upgraded in IMGUI v1.53.
// The Lua API currently uses the old API, and needs to be upgraded.
static int l_pigui_combo(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	int selected = LuaPull<int>(l, 2);

	LuaTable t(l, 3);
	std::vector<const char *> items;
	for (auto it = t.Begin<const char *>(); it != t.End<const char *>(); it++) {
		items.push_back(*it);
	}

	bool changed = ImGui::Combo(lbl.c_str(), &selected, &items[0], static_cast<int>(items.size()));

	LuaPush<bool>(l, changed);
	LuaPush<int>(l, selected);

	return 2;
}

static int l_pigui_listbox(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	int selected = LuaPull<int>(l, 2);

	LuaTable t(l, 3);
	std::vector<const char *> items;
	for (auto it = t.Begin<const char *>(); it != t.End<const char *>(); it++) {
		items.push_back(*it);
	}

	bool changed = ImGui::ListBox(lbl.c_str(), &selected, &items[0], static_cast<int>(items.size()));

	LuaPush<bool>(l, changed);
	LuaPush<int>(l, selected);

	return 2;
}

static int l_pigui_radial_menu(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	std::string id = LuaPull<std::string>(l, 2);
	int mouse_button = LuaPull<int>(l, 3);
	std::vector<ImTextureID> tex_ids;
	std::vector<std::pair<ImVec2, ImVec2>> uvs;
	int i = 0;
	while (true) {
		lua_rawgeti(l, 4, ++i);
		if (lua_isnil(l, -1)) {
			lua_pop(l, 1);
			break;
		}
		if (!lua_istable(l, -1)) {
			Output("element of icons not a table %i\n", i);
			break;
		}
		lua_getfield(l, -1, "id");
		ImTextureID tid = pi_lua_checklightuserdata(l, -1);
		lua_pop(l, 1);

		lua_getfield(l, -1, "uv0");
		ImVec2 uv0 = LuaPull<ImVec2>(l, -1);
		lua_pop(l, 1);

		lua_getfield(l, -1, "uv1");
		ImVec2 uv1 = LuaPull<ImVec2>(l, -1);
		lua_pop(l, 1);

		lua_pop(l, 1);
		tex_ids.push_back(tid);
		uvs.push_back(std::pair<ImVec2, ImVec2>(uv0, uv1));
	}

	int size = LuaPull<int>(l, 5);
	std::vector<std::string> tooltips;
	LuaTable tts(l, 6);
	for (LuaTable::VecIter<std::string> iter = tts.Begin<std::string>(); iter != tts.End<std::string>(); ++iter) {
		tooltips.push_back(*iter);
	}
	int n = PiGui::RadialPopupSelectMenu(center, id, mouse_button, tex_ids, uvs, size, tooltips);
	LuaPush<int>(l, n);
	return 1;
}

static int l_pigui_should_draw_ui(lua_State *l)
{
	PROFILE_SCOPED()
	LuaPush(l, Pi::DrawGUI);
	return 1;
}

static int l_pigui_is_mouse_hovering_rect(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 r_min = LuaPull<ImVec2>(l, 1);
	ImVec2 r_max = LuaPull<ImVec2>(l, 2);
	bool clip = LuaPull<bool>(l, 3);
	LuaPush<bool>(l, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
	return 1;
}

static int l_pigui_data_dir_path(lua_State *l)
{
	PROFILE_SCOPED()
	std::string path = FileSystem::GetDataDir();
	LuaTable pathComponents(l, 1);
	for (LuaTable::VecIter<std::string> iter = pathComponents.Begin<std::string>(); iter != pathComponents.End<std::string>(); ++iter) {
		path = FileSystem::JoinPath(path, *iter);
	}
	LuaPush(l, path);
	return 1;
}

static int l_pigui_is_window_hovered(lua_State *l)
{
	PROFILE_SCOPED()
	int flags = LuaPull<ImGuiHoveredFlags_>(l, 1, ImGuiHoveredFlags_None);
	LuaPush<bool>(l, ImGui::IsWindowHovered(flags));
	return 1;
}

static int l_pigui_system_info_view_next_page(lua_State *l)
{
	PROFILE_SCOPED()
	Pi::game->GetSystemInfoView()->NextPage();
	return 0;
}

static int l_pigui_input_text(lua_State *l)
{
	PROFILE_SCOPED()
	std::string label = LuaPull<std::string>(l, 1);
	std::string text = LuaPull<std::string>(l, 2);
	int flags = LuaPull<ImGuiInputTextFlags_>(l, 3, ImGuiInputTextFlags_None);
	// callback
	// user_data
	char buffer[1024];
	memset(buffer, 0, 1024);
	strncpy(buffer, text.c_str(), 1023);
	bool result = ImGui::InputText(label.c_str(), buffer, 1024, flags);
	LuaPush<const char *>(l, buffer);
	LuaPush<bool>(l, result);
	return 2;
}

static int l_pigui_play_sfx(lua_State *l)
{
	PROFILE_SCOPED()
	std::string name = LuaPull<std::string>(l, 1);
	double left = LuaPull<float>(l, 2);
	double right = LuaPull<float>(l, 3);
	Sound::PlaySfx(name.c_str(), left, right, false);
	return 0;
}

static int l_pigui_circular_slider(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	float v = LuaPull<double>(l, 2);
	float v_min = LuaPull<double>(l, 3);
	float v_max = LuaPull<double>(l, 4);
	bool res = PiGui::CircularSlider(center, &v, v_min, v_max);
	if (res)
		LuaPush<double>(l, v);
	else
		lua_pushnil(l);
	return 1;
}

/*
 * Function: sliderFloat
 *
 * A slider for setting floating values
 *
 * > value = ui.sliderFloat(label, value, min, max, format)
 *
 * Parameters:
 *
 *   label - string, text
 *   value - float, set slider to this value
 *   min - float, lower bound
 *   max - float, upper bound
 *   format - optional string, format according to snprintf
 *
 * Returns:
 *
 *   value - the value that the slider was set to
 *
 */
static int l_pigui_slider_float(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	float value = LuaPull<int>(l, 2);
	float val_min = LuaPull<int>(l, 3);
	float val_max = LuaPull<int>(l, 4);
	std::string format = LuaPull<std::string>(l, 5, "%.3f");

	ImGui::SliderFloat(lbl.c_str(), &value, val_min, val_max, format.c_str());

	LuaPush<float>(l, value);
	return 1;
}

/*
 * Function: sliderInt
 *
 * A slider for setting integer values
 *
 * > value = ui.sliderInt(label, value, min, max, format)
 *
 * Parameters:
 *
 *   label - string, text
 *   value - int, set slider to this value
 *   min - int, lower bound
 *   max - int, upper bound
 *   format - optional string, format according to snprintf
 *
 * Returns:
 *
 *   value - the value that the slider was set to
 *
 */
static int l_pigui_slider_int(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	int value = LuaPull<int>(l, 2);
	int val_min = LuaPull<int>(l, 3);
	int val_max = LuaPull<int>(l, 4);
	std::string format = LuaPull<std::string>(l, 5, "%d");

	ImGui::SliderInt(lbl.c_str(), &value, val_min, val_max, format.c_str());

	LuaPush<int>(l, value);
	return 1;
}

static int l_pigui_vsliderint(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);

	int value = LuaPull<int>(l, 3);
	int val_min = LuaPull<int>(l, 4);
	int val_max = LuaPull<int>(l, 5);

	ImGui::VSliderInt(lbl.c_str(), size, &value, val_min, val_max);

	LuaPush<int>(l, value);

	return 1;
}

static int l_pigui_vsliderfloat(lua_State *l)
{
	PROFILE_SCOPED()
	std::string lbl = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);

	float value = LuaPull<float>(l, 3);
	float val_min = LuaPull<float>(l, 4);
	float val_max = LuaPull<float>(l, 5);

	ImGui::VSliderFloat(lbl.c_str(), size, &value, val_min, val_max);

	LuaPush<float>(l, value);

	return 1;
}

static int l_pigui_is_key_released(lua_State *l)
{
	PROFILE_SCOPED()
	SDL_Keycode key = LuaPull<int>(l, 1);
	LuaPush<bool>(l, ImGui::IsKeyReleased(SDL_GetScancodeFromKey(key)));
	return 1;
}

static int l_pigui_get_cursor_pos(lua_State *l)
{
	PROFILE_SCOPED()
	vector2d v(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y);
	LuaPush<vector2d>(l, v);
	return 1;
}

static int l_pigui_get_cursor_screen_pos(lua_State *l)
{
	PROFILE_SCOPED()
	vector2d v(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
	LuaPush<vector2d>(l, v);
	return 1;
}

static int l_pigui_set_cursor_pos(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 v = LuaPull<ImVec2>(l, 1);
	ImGui::SetCursorPos(v);
	return 0;
}

static int l_pigui_set_cursor_screen_pos(lua_State *l)
{
	PROFILE_SCOPED()
	ImVec2 v = LuaPull<ImVec2>(l, 1);
	ImGui::SetCursorScreenPos(v);
	return 0;
}

static int l_pigui_drag_int_4(lua_State *l)
{
	PROFILE_SCOPED()
	std::string label = LuaPull<std::string>(l, 1);
	int v[4];
	v[0] = LuaPull<int>(l, 2);
	v[1] = LuaPull<int>(l, 3);
	v[2] = LuaPull<int>(l, 4);
	v[3] = LuaPull<int>(l, 5);
	double v_speed = LuaPull<double>(l, 6);
	double v_min = LuaPull<double>(l, 7);
	double v_max = LuaPull<double>(l, 8);
	bool res = ImGui::DragInt4(label.c_str(), v, v_speed, v_min, v_max);
	LuaPush<bool>(l, res);
	LuaPush<int>(l, v[0]);
	LuaPush<int>(l, v[1]);
	LuaPush<int>(l, v[2]);
	LuaPush<int>(l, v[3]);
	return 5;
}

static int l_pigui_add_convex_poly_filled(lua_State *l)
{
	PROFILE_SCOPED()
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	LuaTable pnts(l, 1);
	ImColor color = LuaPull<ImColor>(l, 2);
	bool anti_aliased = LuaPull<bool>(l, 3);
	std::vector<ImVec2> ps;
	int i = 0;
	double x = 0.0, y = 0.0;
	for (LuaTable::VecIter<double> iter = pnts.Begin<double>(); iter != pnts.End<double>(); ++iter) {
		if (i++ % 2) {
			y = *iter;
			ps.push_back(ImVec2(x, y));
		} else
			x = *iter;
	}
	ImDrawListFlags flags = draw_list->Flags;
	if (!anti_aliased) flags = 0; // Disable antialiasing
	draw_list->AddConvexPolyFilled(ps.data(), ps.size(), color);
	draw_list->Flags = flags; // Restore the flags.
	return 0;
}

static int l_pigui_load_texture_from_svg(lua_State *l)
{
	PROFILE_SCOPED()
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	std::string svg_filename = LuaPull<std::string>(l, 2);
	int width = LuaPull<int>(l, 3);
	int height = LuaPull<int>(l, 4);
	ImTextureID id = pigui->RenderSVG(svg_filename, width, height);
	//	LuaPush(l, id);
	lua_pushlightuserdata(l, id);
	return 1;
}

static int l_pigui_set_scroll_here(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::SetScrollHere();
	return 0;
}

static int l_pigui_pop_text_wrap_pos(lua_State *l)
{
	PROFILE_SCOPED()
	ImGui::PopTextWrapPos();
	return 0;
}

// ui.anchor = { left = 1, right = 2, center = 3, top = 4, bottom = 5, baseline = 6 }

static int l_pigui_calc_text_alignment(lua_State *l)
{
	PROFILE_SCOPED()
	vector2d pos = LuaPull<vector2d>(l, 1);
	vector2d size = LuaPull<vector2d>(l, 2);
	int anchor_h, anchor_v;

	if (lua_type(l, 3) != LUA_TNIL) {
		anchor_h = LuaPull<int>(l, 3);
	} else
		anchor_h = -1;

	if (lua_type(l, 4) != LUA_TNIL) {
		anchor_v = LuaPull<int>(l, 4);
	} else
		anchor_v = -1;

	if (anchor_h == 1 || anchor_h == -1) {
	} else if (anchor_h == 2) {
		pos.x -= size.x;
	} else if (anchor_h == 3) {
		pos.x -= size.x / 2;
	} else
		luaL_error(l, "CalcTextAlignment: incorrect horizontal anchor %d", anchor_h);

	if (anchor_v == 4 || anchor_v == -1) {
	} else if (anchor_v == 5) {
		pos.y -= size.y;
	} else if (anchor_v == 3) {
		pos.y -= size.y / 2;
	} else
		luaL_error(l, "CalcTextAlignment: incorrect vertical anchor %d", anchor_v);
	LuaPush<vector2d>(l, pos);
	return 1;
}

/*
 * Function: collapsingHeader
 *
 * A foldable header, to toggle show/hide
 *
 * > clicked = ui.collapsingHeader(text, flags)
 *
 * Example:
 *
 * > if ui.collapsingHeader("My header", {"DefaultOpen"}) then
 * >     ui.text("Now you see me")
 * > end
 *
 * Parameters:
 *
 *   text - string, headline
 *   flags - optional dict of flags
 *
 * Returns:
 *
 *   clicked - boolean, true if clicked, else false
 *
 */
static int l_pigui_collapsing_header(lua_State *l)
{
	PROFILE_SCOPED()
	std::string label = LuaPull<std::string>(l, 1);
	ImGuiTreeNodeFlags flags = LuaPull<ImGuiTreeNodeFlags_>(l, 2, ImGuiTreeNodeFlags_None);
	LuaPush(l, ImGui::CollapsingHeader(label.c_str(), flags));
	return 1;
}

static int l_pigui_push_text_wrap_pos(lua_State *l)
{
	PROFILE_SCOPED()
	float wrap_pos_x = LuaPull<float>(l, 1);
	ImGui::PushTextWrapPos(wrap_pos_x);
	return 0;
}

template <>
const char *LuaObject<PiGui>::s_type = "PiGui";

template <>
void LuaObject<PiGui>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "Begin", l_pigui_begin },
		{ "End", l_pigui_end },
		{ "PushClipRectFullScreen", l_pigui_push_clip_rect_full_screen },
		{ "PopClipRect", l_pigui_pop_clip_rect },
		{ "PushClipRect", l_pigui_push_clip_rect },
		{ "AddCircle", l_pigui_add_circle },
		{ "AddCircleFilled", l_pigui_add_circle_filled },
		{ "AddLine", l_pigui_add_line },
		{ "AddText", l_pigui_add_text },
		{ "AddTriangle", l_pigui_add_triangle },
		{ "AddTriangleFilled", l_pigui_add_triangle_filled },
		{ "AddQuad", l_pigui_add_quad },
		{ "AddRect", l_pigui_add_rect },
		{ "AddRectFilled", l_pigui_add_rect_filled },
		{ "AddImage", l_pigui_add_image },
		{ "AddImageQuad", l_pigui_add_image_quad },
		{ "AddBezierCurve", l_pigui_add_bezier_curve },
		{ "SetNextWindowPos", l_pigui_set_next_window_pos },
		{ "SetNextWindowSize", l_pigui_set_next_window_size },
		{ "SetNextWindowSizeConstraints", l_pigui_set_next_window_size_constraints },
		{ "SetNextWindowFocus", l_pigui_set_next_window_focus },
		{ "SetWindowFocus", l_pigui_set_window_focus },
		{ "GetKeyBinding", l_pigui_get_keybinding },
		{ "GetAxisBinding", l_pigui_get_axisbinding },
		{ "PushStyleColor", l_pigui_push_style_color },
		{ "PopStyleColor", l_pigui_pop_style_color },
		{ "PushStyleVar", l_pigui_push_style_var },
		{ "PopStyleVar", l_pigui_pop_style_var },
		{ "Columns", l_pigui_columns },
		{ "NextColumn", l_pigui_next_column },
		{ "GetColumnWidth", l_pigui_get_column_width },
		{ "SetColumnWidth", l_pigui_set_column_width },
		{ "SetColumnOffset", l_pigui_set_column_offset },
		{ "GetScrollY", l_pigui_get_scroll_y },
		{ "Text", l_pigui_text },
		{ "TextWrapped", l_pigui_text_wrapped },
		{ "TextColored", l_pigui_text_colored },
		{ "SetScrollHere", l_pigui_set_scroll_here },
		{ "Button", l_pigui_button },
		{ "Selectable", l_pigui_selectable },
		{ "BeginGroup", l_pigui_begin_group },
		{ "SetCursorPos", l_pigui_set_cursor_pos },
		{ "GetCursorPos", l_pigui_get_cursor_pos },
		{ "SetCursorScreenPos", l_pigui_set_cursor_screen_pos },
		{ "GetCursorScreenPos", l_pigui_get_cursor_screen_pos },
		{ "EndGroup", l_pigui_end_group },
		{ "SameLine", l_pigui_same_line },
		{ "Separator", l_pigui_separator },
		{ "IsItemHovered", l_pigui_is_item_hovered },
		{ "IsItemActive", l_pigui_is_item_active },
		{ "IsItemClicked", l_pigui_is_item_clicked },
		{ "Spacing", l_pigui_spacing },
		{ "Dummy", l_pigui_dummy },
		{ "BeginChild", l_pigui_begin_child },
		{ "EndChild", l_pigui_end_child },
		{ "PushFont", l_pigui_push_font },
		{ "PopFont", l_pigui_pop_font },
		{ "CalcTextSize", l_pigui_calc_text_size },
		{ "SetTooltip", l_pigui_set_tooltip },
		{ "Checkbox", l_pigui_checkbox },
		{ "GetMousePos", l_pigui_get_mouse_pos },
		{ "GetMouseWheel", l_pigui_get_mouse_wheel },
		{ "PathArcTo", l_pigui_path_arc_to },
		{ "PathStroke", l_pigui_path_stroke },
		{ "PushItemWidth", l_pigui_push_item_width },
		{ "PopItemWidth", l_pigui_pop_item_width },
		{ "PushTextWrapPos", l_pigui_push_text_wrap_pos },
		{ "PopTextWrapPos", l_pigui_pop_text_wrap_pos },
		{ "BeginPopup", l_pigui_begin_popup },
		{ "BeginPopupModal", l_pigui_begin_popup_modal },
		{ "EndPopup", l_pigui_end_popup },
		{ "OpenPopup", l_pigui_open_popup },
		{ "CloseCurrentPopup", l_pigui_close_current_popup },
		{ "PushID", l_pigui_push_id },
		{ "PopID", l_pigui_pop_id },
		{ "IsMouseReleased", l_pigui_is_mouse_released },
		{ "IsMouseClicked", l_pigui_is_mouse_clicked },
		{ "IsMouseDown", l_pigui_is_mouse_down },
		{ "IsMouseHoveringRect", l_pigui_is_mouse_hovering_rect },
		{ "IsWindowHovered", l_pigui_is_window_hovered },
		{ "Image", l_pigui_image },
		{ "pointOnClock", l_pigui_pointOnClock },
		{ "lineOnClock", l_pigui_lineOnClock },
		{ "ImageButton", l_pigui_image_button },
		{ "ButtonImageSized", l_pigui_button_image_sized },
		{ "RadialMenu", l_pigui_radial_menu },
		{ "CircularSlider", l_pigui_circular_slider },
		{ "SliderInt", l_pigui_slider_int },
		{ "SliderFloat", l_pigui_slider_float },
		{ "VSliderFloat", l_pigui_vsliderfloat },
		{ "VSliderInt", l_pigui_vsliderint },
		{ "GetMouseClickedPos", l_pigui_get_mouse_clicked_pos },
		{ "AddConvexPolyFilled", l_pigui_add_convex_poly_filled },
		{ "IsKeyReleased", l_pigui_is_key_released },
		{ "DragInt4", l_pigui_drag_int_4 },
		{ "GetWindowPos", l_pigui_get_window_pos },
		{ "GetWindowSize", l_pigui_get_window_size },
		{ "GetContentRegion", l_pigui_get_content_region },
		{ "GetTextLineHeight", l_pigui_get_text_line_height },
		{ "GetTextLineHeightWithSpacing", l_pigui_get_text_line_height_with_spacing },
		{ "GetFrameHeight", l_pigui_get_frame_height },
		{ "GetFrameHeightWithSpacing", l_pigui_get_frame_height_with_spacing },
		{ "InputText", l_pigui_input_text },
		{ "Combo", l_pigui_combo },
		{ "ListBox", l_pigui_listbox },
		{ "CollapsingHeader", l_pigui_collapsing_header },
		{ "CaptureMouseFromApp", l_pigui_capture_mouse_from_app },
		{ "ProgressBar", l_pigui_progress_bar },
		{ "LoadTextureFromSVG", l_pigui_load_texture_from_svg },
		{ "DataDirPath", l_pigui_data_dir_path },
		{ "ShouldDrawUI", l_pigui_should_draw_ui },
		{ "GetTargetsNearby", l_pigui_get_targets_nearby },
		{ "GetProjectedBodies", l_pigui_get_projected_bodies },
		{ "GetProjectedBodiesGrouped", l_pigui_get_projected_bodies_grouped },
		{ "CalcTextAlignment", l_pigui_calc_text_alignment },
		{ "ShouldShowLabels", l_pigui_should_show_labels },
		{ "SystemInfoViewNextPage", l_pigui_system_info_view_next_page }, // deprecated
		{ "LowThrustButton", l_pigui_low_thrust_button },
		{ "ThrustIndicator", l_pigui_thrust_indicator },
		{ "PlaySfx", l_pigui_play_sfx },
		{ "DisableMouseFacing", l_pigui_disable_mouse_facing },
		{ "SetMouseButtonState", l_pigui_set_mouse_button_state },
		{ "SelectableFlags", l_pigui_check_selectable_flags },
		{ "TreeNodeFlags", l_pigui_check_tree_node_flags },
		{ "InputTextFlags", l_pigui_check_input_text_flags },
		{ "WindowFlags", l_pigui_check_window_flags },
		{ "HoveredFlags", l_pigui_check_hovered_flags },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "handlers", l_attr_handlers },
		{ "screen_width", l_attr_screen_width },
		{ "screen_height", l_attr_screen_height },
		{ "key_ctrl", l_attr_key_ctrl },
		{ "key_none", l_attr_key_none },
		{ "key_shift", l_attr_key_shift },
		{ "key_alt", l_attr_key_alt },
		{ "keys", l_attr_keys },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, l_methods, l_attrs, 0);

	lua_State *l = Lua::manager->GetLuaState();

	imguiSelectableFlagsTable.Register(l, "ImGuiSelectableFlags");
	imguiTreeNodeFlagsTable.Register(l, "ImGuiTreeNodeFlags");
	imguiInputTextFlagsTable.Register(l, "ImGuiInputTextFlags");
	imguiSetCondTable.Register(l, "ImGuiCond");
	imguiColTable.Register(l, "ImGuiCol");
	imguiStyleVarTable.Register(l, "ImGuiStyleVar");
	imguiWindowFlagsTable.Register(l, "ImGuiWindowFlags");
	imguiHoveredFlagsTable.Register(l, "ImGuiHoveredFlags");
}
