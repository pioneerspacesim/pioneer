// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "PiGui.h"
#include "WorldView.h"
#include "Pi.h"
#include "Game.h"
#include "graphics/Graphics.h"
#include "Player.h"
#include "EnumStrings.h"
#include "SystemInfoView.h"
#include "Sound.h"

// Windows defines RegisterClass as a macro, but we don't need that here.
// undef it, to avoid including yet another header that undefs it
#undef RegisterClass

template<typename Type>
static Type parse_imgui_flags(lua_State *l, int index, std::map<std::string, Type> table, std::string name) {
	LuaTable flags(l, index);
	Type theflags = Type(0);
	for(LuaTable::VecIter<std::string> iter = flags.Begin<std::string>(); iter != flags.End<std::string>(); ++iter) {
		std::string flag = *iter;
		if(table.find(flag) != table.end())
			theflags = static_cast<Type>(theflags | table.at(flag));
		else
			Error("Unknown %s %s\n", name.c_str(), flag.c_str());
	}
	return theflags;
}

template <typename Type>
static Type parse_imgui_enum(lua_State *l, int index, std::map<std::string, Type> table, std::string name) {
	std::string stylestr = LuaPull<std::string>(l, index);
	if(table.find(stylestr) != table.end())
		return table.at(stylestr);
	else
		Error("Unknown %s %s\n", name.c_str(), stylestr.c_str());
}

void *pi_lua_checklightuserdata(lua_State *l, int index) {
	if(lua_islightuserdata(l, index))
		return lua_touserdata(l, index);
	else
		Error("Expected light user data at index %d, but got %s", index, lua_typename(l, index));
	return nullptr;
}

void pi_lua_generic_pull(lua_State *l, int index, ImVec2 &vector) {
	LuaTable vec(l, index);
	vector.x = vec.Get<double>("x");
	vector.y = vec.Get<double>("y");
}

void pi_lua_generic_pull(lua_State *l, int index, vector3d &vector) {
	LuaTable vec(l, index);
	vector.x = vec.Get<double>("x");
	vector.y = vec.Get<double>("y");
	vector.z = vec.Get<double>("z");
}

void pi_lua_generic_pull(lua_State *l, int index, ImColor &color) {
	LuaTable c(l, index);
	float sc = 1.0f/255.0f;
	color.Value.x = c.Get<int>("r") * sc;
	color.Value.y = c.Get<int>("g") * sc;
	color.Value.z = c.Get<int>("b") * sc;
	color.Value.w = c.Get<int>("a", 255) * sc;
}

int pushOnScreenPositionDirection(lua_State *l, vector3d position)
{
	const int width = Graphics::GetScreenWidth();
	const int height = Graphics::GetScreenHeight();
	vector3d direction = (position - vector3d(width / 2, height / 2, 0)).Normalized();
	if(vector3d(0,0,0) == position || position.x < 0 || position.y < 0 || position.x > width || position.y > height || position.z > 0) {
		LuaPush<bool>(l, false);
		LuaPush<vector3d>(l, vector3d(0, 0, 0));
		LuaPush<vector3d>(l, direction * (position.z > 0 ? -1 : 1)); // reverse direction if behind camera
	} else {
		LuaPush<bool>(l, true);
		LuaPush<vector3d>(l, vector3d(position.x, position.y, 0));
		LuaPush<vector3d>(l, direction);
	}
	return 3;
}

static std::map<std::string, ImGuiSelectableFlags_> imguiSelectableFlagsTable
= {
	{ "DontClosePopups", ImGuiSelectableFlags_DontClosePopups },
	{ "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns },
	{ "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiSelectableFlags_ &theflags) {
	theflags = parse_imgui_flags(l, index, imguiSelectableFlagsTable, "ImGuiSelectableFlags");
}

static std::map<std::string, ImGuiInputTextFlags_> imguiInputTextFlagsTable
= {
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

void pi_lua_generic_pull(lua_State *l, int index, ImGuiInputTextFlags_ &theflags) {
	theflags = parse_imgui_flags(l, index, imguiInputTextFlagsTable, "ImGuiInputTextFlagsTable");
}

static std::map<std::string, ImGuiSetCond_> imguiSetCondTable
= {
	{ "Always", ImGuiSetCond_Always },
	{ "Once", ImGuiSetCond_Once },
	{ "FirstUseEver", ImGuiSetCond_FirstUseEver },
	{ "Appearing", ImGuiSetCond_Appearing }
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiSetCond_ &value) {
	value = parse_imgui_enum(l, index, imguiSetCondTable, "ImGuiSetCond");
}

static std::map<std::string, ImGuiCol_> imguiColTable
= {
	{"Text", ImGuiCol_Text},
	{"TextDisabled", ImGuiCol_TextDisabled},
	{"WindowBg", ImGuiCol_WindowBg},
	{"ChildWindowBg", ImGuiCol_ChildWindowBg},
	{"PopupBg", ImGuiCol_PopupBg},
	{"Border", ImGuiCol_Border},
	{"BorderShadow", ImGuiCol_BorderShadow},
	{"FrameBg", ImGuiCol_FrameBg},
	{"FrameBgHovered", ImGuiCol_FrameBgHovered},
	{"FrameBgActive", ImGuiCol_FrameBgActive},
	{"TitleBg", ImGuiCol_TitleBg},
	{"TitleBgCollapsed", ImGuiCol_TitleBgCollapsed},
	{"TitleBgActive", ImGuiCol_TitleBgActive},
	{"MenuBarBg", ImGuiCol_MenuBarBg},
	{"ScrollbarBg", ImGuiCol_ScrollbarBg},
	{"ScrollbarGrab", ImGuiCol_ScrollbarGrab},
	{"ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered},
	{"ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive},
	{"ComboBg", ImGuiCol_ComboBg},
	{"CheckMark", ImGuiCol_CheckMark},
	{"SliderGrab", ImGuiCol_SliderGrab},
	{"SliderGrabActive", ImGuiCol_SliderGrabActive},
	{"Button", ImGuiCol_Button},
	{"ButtonHovered", ImGuiCol_ButtonHovered},
	{"ButtonActive", ImGuiCol_ButtonActive},
	{"Header", ImGuiCol_Header},
	{"HeaderHovered", ImGuiCol_HeaderHovered},
	{"HeaderActive", ImGuiCol_HeaderActive},
	{"Column", ImGuiCol_Column},
	{"ColumnHovered", ImGuiCol_ColumnHovered},
	{"ColumnActive", ImGuiCol_ColumnActive},
	{"ResizeGrip", ImGuiCol_ResizeGrip},
	{"ResizeGripHovered", ImGuiCol_ResizeGripHovered},
	{"ResizeGripActive", ImGuiCol_ResizeGripActive},
	{"CloseButton", ImGuiCol_CloseButton},
	{"CloseButtonHovered", ImGuiCol_CloseButtonHovered},
	{"CloseButtonActive", ImGuiCol_CloseButtonActive},
	{"PlotLines", ImGuiCol_PlotLines},
	{"PlotLinesHovered", ImGuiCol_PlotLinesHovered},
	{"PlotHistogram", ImGuiCol_PlotHistogram},
	{"PlotHistogramHovered", ImGuiCol_PlotHistogramHovered},
	{"TextSelectedBg", ImGuiCol_TextSelectedBg},
	{"ModalWindowDarkening", ImGuiCol_ModalWindowDarkening}
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiCol_ &value) {
	value = parse_imgui_enum(l, index, imguiColTable, "ImGuiCol");
}

static std::map<std::string, ImGuiStyleVar_> imguiStyleVarTable
= {
    { "Alpha", ImGuiStyleVar_Alpha},
	{ "WindowPadding", ImGuiStyleVar_WindowPadding},
	{ "WindowRounding", ImGuiStyleVar_WindowRounding},
	{ "WindowMinSize", ImGuiStyleVar_WindowMinSize},
	{ "ChildWindowRounding", ImGuiStyleVar_ChildWindowRounding},
	{ "FramePadding", ImGuiStyleVar_FramePadding},
	{ "FrameRounding", ImGuiStyleVar_FrameRounding},
	{ "ItemSpacing", ImGuiStyleVar_ItemSpacing},
	{ "ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing},
	{ "IndentSpacing", ImGuiStyleVar_IndentSpacing},
	{ "GrabMinSize", ImGuiStyleVar_GrabMinSize},
	{ "ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign}
};

void pi_lua_generic_pull(lua_State *l, int index, ImGuiStyleVar_ &value) {
	value = parse_imgui_enum(l, index, imguiStyleVarTable, "ImGuiStyleVar");
}

static std::map<std::string, ImGuiWindowFlags_> imguiWindowFlagsTable
= {
	{ "NoTitleBar", ImGuiWindowFlags_NoTitleBar },
	{ "NoResize", ImGuiWindowFlags_NoResize },
	{ "NoMove", ImGuiWindowFlags_NoMove },
	{ "NoScrollbar", ImGuiWindowFlags_NoScrollbar },
	{ "NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse },
	{ "NoCollapse", ImGuiWindowFlags_NoCollapse },
	{ "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize },
	{ "ShowBorders", ImGuiWindowFlags_ShowBorders },
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

void pi_lua_generic_pull(lua_State *l, int index, ImGuiWindowFlags_ &theflags) {
	theflags = parse_imgui_flags(l, index, imguiWindowFlagsTable, "ImGuiWindowFlags");
}

static void pi_lua_pushVector(lua_State *l, double x, double y, double z) {
	const int n = lua_gettop(l);
	lua_getfield(l, LUA_REGISTRYINDEX, "Imports");
	lua_getfield(l, -1, "libs/Vector.lua"); // is there a better way to get at the lua Vector than this?
	LuaPush<double>(l, x);
	LuaPush<double>(l, y);
	LuaPush<double>(l, z);
	if(lua_pcall(l, 3, 1, 0) != 0) {
		Error("error running Vector\n");
	}
	lua_remove(l, -2);
	assert(lua_gettop(l) == n + 1);
}

static void pi_lua_generic_push(lua_State *l, const ImVec2 &v) {
	pi_lua_pushVector(l, v.x, v.y, 0);
}

void pi_lua_generic_push(lua_State *l, const vector3d &v) {
	pi_lua_pushVector(l, v.x, v.y, v.z);
}

/*
 * Interface: PiGui
 *
 * Various functions for the imgui UI. Do *not* use these directly, use the interface that import('pigui') provides.
 */

/* ****************************** Lua imgui functions ****************************** */
/*
 * Function: Begin
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_pigui_begin(lua_State *l) {
	const std::string name = LuaPull<std::string>(l, 1);
	ImGuiWindowFlags theflags = LuaPull<ImGuiWindowFlags_>(l, 2);
	ImGui::Begin(name.c_str(), nullptr, theflags);
	return 0;
}

static int l_pigui_columns(lua_State *l) {
	int columns = LuaPull<int>(l, 1);
	std::string id = LuaPull<std::string>(l, 2);
	bool border = LuaPull<bool>(l, 3);
	ImGui::Columns(columns, id.c_str(), border);
	return 0;
}

static int l_pigui_progress_bar(lua_State *l) {
	float fraction = LuaPull<double>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	std::string overlay = LuaPull<std::string>(l, 3);
	ImGui::ProgressBar(fraction, size, overlay.c_str());
	return 0;
}

static int l_pigui_next_column(lua_State *l) {
	ImGui::NextColumn();
	return 0;
}

static int l_pigui_end(lua_State *l) {
	ImGui::End();
	return 0;
}

static int l_pigui_pop_clip_rect(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->PopClipRect();
	return 0;
}

static int l_pigui_push_clip_rect(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 min = LuaPull<ImVec2>(l, 1);
	ImVec2 max = LuaPull<ImVec2>(l, 2);
	bool intersect = LuaPull<bool>(l, 3);
	draw_list->PushClipRect(min, max, intersect);
	return 0;
}

static int l_pigui_push_clip_rect_full_screen(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRectFullScreen();
	return 0;
}

static int l_pigui_set_next_window_pos(lua_State *l) {
	ImVec2 pos = LuaPull<ImVec2>(l, 1);
	int cond = LuaPull<ImGuiSetCond_>(l, 2);
	ImGui::SetNextWindowPos(pos, cond);
	return 0;
}

static int l_pigui_set_next_window_focus(lua_State *l) {
	ImGui::SetNextWindowFocus();
	return 0;
}

static int l_pigui_set_window_focus(lua_State *l) {
	std::string name = LuaPull<std::string>(l, 1);
	ImGui::SetWindowFocus(name.c_str());
	return 0;
}

static int l_pigui_set_next_window_size(lua_State *l) {
	ImVec2 size = LuaPull<ImVec2>(l, 1);
	int cond = LuaPull<ImGuiSetCond_>(l, 2);
	ImGui::SetNextWindowSize(size, cond);
	return 0;
}

static int l_pigui_push_style_color(lua_State *l) {
	int style = LuaPull<ImGuiCol_>(l, 1);
	ImColor color = LuaPull<ImColor>(l, 2);
	ImGui::PushStyleColor(style, color);
	return 0;
}

static int l_pigui_pop_style_color(lua_State *l) {
	int num = LuaPull<int>(l, 1);
	ImGui::PopStyleColor(num);
	return 0;
}

static int l_pigui_push_style_var(lua_State *l) {
	int style = LuaPull<ImGuiStyleVar_>(l, 1);
	float val = LuaPull<double>(l, 2);
	ImGui::PushStyleVar(style, val);
	return 0;
}

static int l_pigui_pop_style_var(lua_State *l) {
	int num = LuaPull<int>(l, 1);
	ImGui::PopStyleVar(num);
	return 0;
}

static int l_pigui_add_line(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor col = LuaPull<ImColor>(l, 3);
	double thickness = LuaPull<double>(l, 4);
	draw_list->AddLine(a, b, col, thickness);
	return 0;
}

static int l_pigui_add_circle(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	int radius = LuaPull<int>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	int segments = LuaPull<int>(l, 4);
	double thickness = LuaPull<double>(l, 5);
	draw_list->AddCircle(center, radius, color, segments, thickness);
	return 0;
}

static int l_pigui_add_circle_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	int radius = LuaPull<int>(l, 2);
	ImColor color = LuaPull<ImColor>(l, 3);
	int segments = LuaPull<int>(l, 4);
	draw_list->AddCircleFilled(center, radius, color, segments);
	return 0;
}

static int l_pigui_path_arc_to(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	double radius = LuaPull<double>(l, 2);
	double amin = LuaPull<double>(l, 3);
	double amax = LuaPull<double>(l, 4);
	int segments = LuaPull<int>(l, 5);
	draw_list->PathArcTo(center, radius, amin, amax, segments);
	return 0;
}

static int l_pigui_path_stroke(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImColor color = LuaPull<ImColor>(l, 1);
	bool closed = LuaPull<bool>(l, 2);
	double thickness = LuaPull<double>(l, 3);
	draw_list->PathStroke(color, closed, thickness);
	return 0;
}

static int l_pigui_selectable(lua_State *l) {
	std::string label = LuaPull<std::string>(l, 1);
	bool selected = LuaPull<bool>(l, 2);
	ImGuiSelectableFlags flags = LuaPull<ImGuiSelectableFlags_>(l, 3);
	// TODO: parameter size
	bool res = ImGui::Selectable(label.c_str(), selected, flags);
	LuaPush<bool>(l, res);
	return 1;
}

static int l_pigui_text(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::Text("%s", text.c_str());
	return 0;
}

static int l_pigui_button(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	bool ret = ImGui::Button(text.c_str());
	LuaPush<bool>(l, ret);
	return 1;
}

static int l_pigui_thrust_indicator(lua_State *l) {
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
	PiGui::ThrustIndicator(text.c_str(), size, thrust, velocity, color, frame_padding, vel_fg, vel_bg, thrust_fg, thrust_bg);
	return 0;
}

static int l_pigui_low_thrust_button(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	float level = LuaPull<int>(l, 3);
	ImColor color = LuaPull<ImColor>(l, 4);
	int frame_padding = LuaPull<int>(l, 5);
	ImColor gauge_fg = LuaPull<ImColor>(l, 6);
	ImColor gauge_bg = LuaPull<ImColor>(l, 7);
	bool ret = PiGui::LowThrustButton(text.c_str(), size, level, color, frame_padding, gauge_fg, gauge_bg);
	LuaPush<bool>(l, ret);
	return 1;
}

static int l_pigui_text_wrapped(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::TextWrapped("%s", text.c_str());
	return 0;
}

static int l_pigui_text_colored(lua_State *l) {
	ImColor col = LuaPull<ImColor>(l, 1);
	std::string text = LuaPull<std::string>(l, 2);
	ImGui::TextColored(col, "%s", text.c_str());
	return 0;
}

// static int l_pigui_get_velocity(lua_State *l) {
// 	std::string name = LuaPull<std::string>(l, 1);
// 	WorldView *wv = Pi::game->GetWorldView();
// 	vector3d velocity;
// 	if(!name.compare("nav_prograde")) {
// 		velocity = wv->GetNavProgradeVelocity();
// 	} else if(!name.compare("frame_prograde")) {
// 		velocity = wv->GetFrameProgradeVelocity();
// 	} else
// 		return 0;
// 	LuaTable vel(l);
// 	vel.Set("x", velocity.x);
// 	vel.Set("y", velocity.y);
// 	vel.Set("z", velocity.z);
// 	return 1;
// }

// static int l_pigui_get_hud_marker(lua_State *l) {
// 	std::string name = LuaPull<std::string>(l, 1);
// 	WorldView *wv = Pi::game->GetWorldView();
// 	const Indicator *marker;

// 	if(!name.compare("frame_prograde"))
// 		marker = wv->GetFrameProgradeIndicator();
// 	else if(!name.compare("frame_retrograde"))
// 		marker = wv->GetFrameRetrogradeIndicator();
// 	else if(!name.compare("nav_prograde"))
// 		marker = wv->GetNavProgradeIndicator();
// 	else if(!name.compare("nav_retrograde"))
// 		marker = wv->GetNavRetrogradeIndicator();
// 	else if(!name.compare("frame"))
// 		marker = wv->GetFrameIndicator();
// 	else if(!name.compare("nav"))
// 		marker = wv->GetNavIndicator();
// 	else if(!name.compare("forward"))
// 		marker = wv->GetForwardIndicator();
// 	else if(!name.compare("backward"))
// 		marker = wv->GetBackwardIndicator();
// 	else if(!name.compare("up"))
// 		marker = wv->GetUpIndicator();
// 	else if(!name.compare("down"))
// 		marker = wv->GetDownIndicator();
// 	else if(!name.compare("left"))
// 		marker = wv->GetLeftIndicator();
// 	else if(!name.compare("right"))
// 		marker = wv->GetRightIndicator();
// 	else if(!name.compare("normal"))
// 		marker = wv->GetNormalIndicator();
// 	else if(!name.compare("anti_normal"))
// 		marker = wv->GetAntiNormalIndicator();
// 	else if(!name.compare("radial_in"))
// 		marker = wv->GetRadialOutIndicator();
// 	else if(!name.compare("radial_out"))
// 		marker = wv->GetRadialInIndicator();
// 	else if(!name.compare("away_from_frame"))
// 		marker = wv->GetAwayFromFrameIndicator();
// 	else if(!name.compare("combat_target"))
// 		marker = wv->GetCombatTargetIndicator();
// 	else if(!name.compare("combat_target_lead"))
// 		marker = wv->GetCombatTargetLeadIndicator();
// 	else if(!name.compare("maneuver"))
// 		marker = wv->GetManeuverIndicator();

// 	else
// 		// TODO: error
// 		return 0;
// 	vector2f position = marker->pos - vector2f(Graphics::GetScreenWidth()/2,Graphics::GetScreenHeight()/2);
// 	vector2f direction = position.Normalized();
// 	std::string side = marker->side == INDICATOR_HIDDEN ? "hidden" : (marker->side == INDICATOR_ONSCREEN ? "onscreen" : "other");
// 	lua_pushstring(l, side.c_str());
// 	LuaTable dir(l);
// 	dir.Set("x", direction.x);
// 	dir.Set("y", direction.y);
// 	dir.Set("z", 0);
// 	LuaTable pos(l);
// 	pos.Set("x", marker->pos.x);
// 	pos.Set("y", marker->pos.y);
// 	pos.Set("z", 0);
// 	//	  Output("forward: %f/%f %s\n", marker->pos.x, marker->pos.y, side.c_str());
// 	return 3;
// }

static int l_pigui_add_text(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	ImColor color = LuaPull<ImColor>(l, 2);
	std::string text = LuaPull<std::string>(l, 3);
	draw_list->AddText(center, color, text.c_str());
	return 0;
}

static int l_pigui_add_triangle(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImColor col = LuaPull<ImColor>(l, 4);
	float thickness = LuaPull<double>(l, 5);
	draw_list->AddTriangle(a, b, c, col, thickness);
	return 0;
}

static int l_pigui_add_rect(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor col = LuaPull<ImColor>(l, 3);
	float rounding = LuaPull<double>(l, 4);
	int round_corners = LuaPull<int>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	draw_list->AddRect(a, b, col, rounding, round_corners, thickness);
	return 0;
}

static int l_pigui_add_bezier_curve(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 c0 = LuaPull<ImVec2>(l, 2);
	ImVec2 c1 = LuaPull<ImVec2>(l, 3);
	ImVec2 b = LuaPull<ImVec2>(l, 4);
	ImColor col = LuaPull<ImColor>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	int num_segments = LuaPull<int>(l, 7);
	draw_list->AddBezierCurve(a, c0, c1, b, col, thickness, num_segments);
	return 0;
}

static int l_pigui_add_image(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 a = LuaPull<ImVec2>(l, 2);
	ImVec2 b = LuaPull<ImVec2>(l, 3);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 4);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 5);
	ImColor col = LuaPull<ImColor>(l, 6);
	draw_list->AddImage(id, a, b, uv0, uv1, col);
	return 0;
}

static int l_pigui_add_image_quad(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 a = LuaPull<ImVec2>(l, 2);
	ImVec2 b = LuaPull<ImVec2>(l, 3);
	ImVec2 c = LuaPull<ImVec2>(l, 4);
	ImVec2 d = LuaPull<ImVec2>(l, 5);
	ImVec2 uva = LuaPull<ImVec2>(l, 6);
	ImVec2 uvb = LuaPull<ImVec2>(l, 7);
	ImVec2 uvc = LuaPull<ImVec2>(l, 8);
	ImVec2 uvd = LuaPull<ImVec2>(l, 9);
	ImColor col = LuaPull<ImColor>(l, 10);
	draw_list->AddImageQuad(id, a, b, c, d, uva, uvb, uvc, uvd, col);
	return 0;
}

static int l_pigui_add_rect_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImColor col = LuaPull<ImColor>(l, 3);
	float rounding = LuaPull<double>(l, 4);
	int round_corners = LuaPull<int>(l, 5);
	draw_list->AddRectFilled(a, b, col, rounding, round_corners);
	return 0;
}

static int l_pigui_add_quad(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImVec2 d = LuaPull<ImVec2>(l, 4);
	ImColor col = LuaPull<ImColor>(l, 5);
	float thickness = LuaPull<double>(l, 6);
	draw_list->AddQuad(a, b, c, d, col, thickness);
	return 0;
}

static int l_pigui_add_triangle_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = LuaPull<ImVec2>(l, 1);
	ImVec2 b = LuaPull<ImVec2>(l, 2);
	ImVec2 c = LuaPull<ImVec2>(l, 3);
	ImColor col = LuaPull<ImColor>(l, 4);
	draw_list->AddTriangleFilled(a, b, c, col);
	return 0;
}

static int l_pigui_same_line(lua_State *l) {
	ImGui::SameLine();
	return 0;
}

static int l_pigui_begin_group(lua_State *l) {
	ImGui::BeginGroup();
	return 0;
}

static int l_pigui_end_group(lua_State *l) {
	ImGui::EndGroup();
	return 0;
}

static int l_pigui_separator(lua_State *l) {
	ImGui::Separator();
	return 0;
}

static int l_pigui_spacing(lua_State *l) {
	ImGui::Spacing();
	return 0;
}

static int l_pigui_dummy(lua_State *l) {
	ImVec2 size = LuaPull<ImVec2>(l, 1);
	ImGui::Dummy(size);
	return 0;
}

static int l_pigui_begin_popup(lua_State *l) {
	std::string id = LuaPull<std::string>(l, 1);
	LuaPush<bool>(l, ImGui::BeginPopup(id.c_str()));
	return 1;
}

static int l_pigui_open_popup(lua_State *l) {
	std::string id = LuaPull<std::string>(l, 1);
	ImGui::OpenPopup(id.c_str());
	return 0;
}

static int l_pigui_end_popup(lua_State *l) {
	ImGui::EndPopup();
	return 0;
}

static int l_pigui_begin_child(lua_State *l) {
	std::string id = LuaPull<std::string>(l, 1);
	ImGui::BeginChild(id.c_str());
	return 0;
}

static int l_pigui_end_child(lua_State *l) {
	ImGui::EndChild();
	return 0;
}

static int l_pigui_is_item_hovered(lua_State *l) {
	LuaPush(l, ImGui::IsItemHovered());
	return 1;
}

static int l_pigui_is_item_clicked(lua_State *l) {
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsItemClicked(button));
	return 1;
}

static int l_pigui_is_mouse_released(lua_State *l) {
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseReleased(button));
	return 1;
}

static int l_pigui_is_mouse_down(lua_State *l) {
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseDown(button));
	return 1;
}

static int l_pigui_is_mouse_clicked(lua_State *l) {
	int button = LuaPull<int>(l, 1);
	LuaPush(l, ImGui::IsMouseClicked(button));
	return 1;
}

static int l_pigui_push_font(lua_State *l) {
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	std::string fontname = LuaPull<std::string>(l, 2);
	int size = LuaPull<int>(l, 3);
	ImFont *font = pigui->GetFont(fontname, size);
	if(!font) {
		LuaPush(l, false);
	} else {
		LuaPush(l, true);
		ImGui::PushFont(font);
	}
	return 1;
}

static int l_pigui_pop_font(lua_State *l) {
	ImGui::PopFont();
	return 0;
}

static int l_pigui_calc_text_size(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	ImVec2 size = ImGui::CalcTextSize(text.c_str());
	pi_lua_generic_push(l, size);
	return 1;
}

static int l_pigui_get_mouse_pos(lua_State *l) {
	ImVec2 pos = ImGui::GetMousePos();
	pi_lua_generic_push(l, pos);
	return 1;
}

static int l_pigui_get_mouse_wheel(lua_State *l) {
	float wheel = ImGui::GetIO().MouseWheel;
	LuaPush(l, wheel);
	return 1;
}

static int l_pigui_set_tooltip(lua_State *l) {
	std::string text = LuaPull<std::string>(l, 1);
	ImGui::SetTooltip("%s", text.c_str());
	return 0;
}

static int l_pigui_checkbox(lua_State *l) {
	std::string label = LuaPull<std::string>(l, 1);
	bool checked = LuaPull<bool>(l, 2);
	bool changed = ImGui::Checkbox(label.c_str(), &checked);
	LuaPush<bool>(l, changed);
	LuaPush<bool>(l, checked);
	return 2;
}

static int l_pigui_push_item_width(lua_State *l) {
	double width = LuaPull<double>(l, 1);
	ImGui::PushItemWidth(width);
	return 0;
}

static int l_pigui_pop_item_width(lua_State *l) {
	ImGui::PopItemWidth();
	return 0;
}

static int l_pigui_push_id(lua_State *l) {
	std::string id = LuaPull<std::string>(l, 1);
	ImGui::PushID(id.c_str());
	return 0;
}

static int l_pigui_pop_id(lua_State *l) {
	ImGui::PopID();
	return 0;
}

static int l_pigui_get_window_pos(lua_State *l) {
	ImVec2 pos = ImGui::GetWindowPos();
	pi_lua_generic_push(l, pos);
	return 1;
}

static int l_pigui_image(lua_State *l) {
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 3);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 4);
	ImColor tint_col = LuaPull<ImColor>(l, 5);
	ImGui::Image(id, size, uv0, uv1, tint_col); //,  border_col
	return 0;
}

static int l_pigui_image_button(lua_State *l) {
	ImTextureID id = pi_lua_checklightuserdata(l, 1);
	ImVec2 size = LuaPull<ImVec2>(l, 2);
	ImVec2 uv0 = LuaPull<ImVec2>(l, 3);
	ImVec2 uv1 = LuaPull<ImVec2>(l, 4);
	int frame_padding = LuaPull<int>(l, 5);
	ImColor bg_col = LuaPull<ImColor>(l, 6);
	ImColor tint_col = LuaPull<ImColor>(l, 7);
	bool res = ImGui::ImageButton(id, size, uv0, uv1, frame_padding, bg_col, tint_col);
	LuaPush<bool>(l, res);
	return 1;
}

static int l_pigui_capture_mouse_from_app(lua_State *l) {
	bool b = LuaPull<bool>(l, 1);
	ImGui::CaptureMouseFromApp(b);
	return 0;
}

static int l_pigui_get_mouse_clicked_pos(lua_State *l) {
	int n = LuaPull<int>(l, 1);
	ImVec2 pos = ImGui::GetIO().MouseClickedPos[n];
	pi_lua_generic_push(l, pos);
	return 1;
}

std::tuple<bool, vector3d, vector3d> lua_world_space_to_screen_space(vector3d pos) {
	WorldView *wv = Pi::game->GetWorldView();
	vector3d p = wv->WorldSpaceToScreenSpace(pos);
	const int width = Graphics::GetScreenWidth();
	const int height = Graphics::GetScreenHeight();
	vector3d direction = (p - vector3d(width / 2, height / 2, 0)).Normalized();
	if(vector3d(0,0,0) == p || p.x < 0 || p.y < 0 || p.x > width || p.y > height || p.z > 0) {
		return std::make_tuple(false, vector3d(0, 0, 0), direction * (p.z > 0 ? -1 : 1));
	} else {
		return std::make_tuple(true, vector3d(p.x, p.y, 0), direction);
	}
}

static int l_pigui_get_projected_bodies(lua_State *l) {
	LuaTable result(l);
	for (Body* body : Pi::game->GetSpace()->GetBodies()) {
		if(body == Pi::game->GetPlayer()) continue;
		if (body->GetType() == Object::PROJECTILE) continue;

		LuaTable object(l);

		object.Set("type", EnumStrings::GetString("PhysicsObjectType", body->GetType()));

		std::tuple<bool, vector3d, vector3d> res = lua_world_space_to_screen_space(body->GetInterpPositionRelTo(Pi::game->GetPlayer())); // defined in LuaPiGui.cpp		
		object.Set("onscreen", std::get<0>(res));
		object.Set("screenCoordinates", std::get<1>(res));
		object.Set("direction", std::get<2>(res));
		object.Set("body", body);

		result.Set(body, object);
		lua_pop(l, 1);
	}
	LuaPush(l, result);
	return 1;
}

static int l_pigui_get_targets_nearby(lua_State *l) {
	int range_max = LuaPull<double>(l, 1);
	LuaTable result(l);
	Space::BodyNearList nearby;
	Pi::game->GetSpace()->GetBodiesMaybeNear(Pi::player, range_max, nearby);
	int index = 1;
	for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i) {
		if ((*i) == Pi::player) continue;
		if ((*i)->GetType() == Object::PROJECTILE) continue;
		vector3d position = (*i)->GetPositionRelTo(Pi::player);
		float distance = float(position.Length());
		vector3d shipSpacePosition = position * Pi::player->GetOrient();
		// convert to polar https://en.wikipedia.org/wiki/Spherical_coordinate_system
		vector3d polarPosition(// don't calculate X, it is not used
													 // sqrt(shipSpacePosition.x*shipSpacePosition.x
													 // 			+ shipSpacePosition.y*shipSpacePosition.y
													 // 			+ shipSpacePosition.z*shipSpacePosition.z)
													 0
													 ,
													 atan2(shipSpacePosition.x, shipSpacePosition.y),
													 atan2(-shipSpacePosition.z,
																 sqrt(shipSpacePosition.x*shipSpacePosition.x
																			+ shipSpacePosition.y*shipSpacePosition.y))
													 );
		// convert to AEP https://en.wikipedia.org/wiki/Azimuthal_equidistant_projection
		double rho = M_PI / 2 - polarPosition.z;
		double theta = polarPosition.y;
		vector3d aep(rho * sin(theta) / (2 * M_PI), -rho * cos(theta) / (2 * M_PI), 0);
		LuaTable object(l);
		object.Set("distance", distance);
		object.Set("label", (*i)->GetLabel());

		//		object.Set("type", EnumStrings::GetString("PhysicsObjectType", (*i)->GetType()));
		//		object.Set("position", position);
		//		object.Set("oriented_position", shipSpacePosition);
		//		object.Set("polar_position", polarPosition);

		object.Set("aep", aep);
		object.Set("body", (*i));
		result.Set(std::to_string(index++), object);
		lua_pop(l, 1);
	}
  LuaPush(l, result);
	return 1;
}
// static int l_pigui_disable_mouse_facing(lua_State *l) {
// 	bool b = LuaPull<bool>(l, 1);
// 	auto *p = Pi::player->GetPlayerController();
// 	p->SetDisableMouseFacing(b);
// 	return 0;
// }

// static int l_pigui_set_mouse_button_state(lua_State *l) {
// 	int button = LuaPull<int>(l, 1);
// 	bool state = LuaPull<bool>(l, 2);
// 	Pi::SetMouseButtonState(button, state);
// 	return 0;
// }

static int l_pigui_should_show_labels(lua_State *l)
{
	bool show_labels = Pi::game->GetWorldView()->ShouldShowLabels();
	LuaPush(l, show_labels);
	return 1;
}

static int l_attr_handlers(lua_State *l) {
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	pigui->GetHandlers().PushCopyToStack();
	return 1;
}

static int l_attr_keys(lua_State *l) {
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	pigui->GetKeys().PushCopyToStack();
	return 1;
}

static int l_attr_screen_width(lua_State *l) {
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	LuaPush<int>(l,Graphics::GetScreenWidth());
	return 1;
}

static int l_attr_key_ctrl(lua_State *l) {
	LuaPush<bool>(l, ImGui::GetIO().KeyCtrl);
	return 1;
}

static int l_attr_key_none(lua_State *l) {
	LuaPush<bool>(l, !ImGui::GetIO().KeyCtrl & !ImGui::GetIO().KeyShift & !ImGui::GetIO().KeyAlt);
	return 1;
}

static int l_attr_key_shift(lua_State *l) {
	LuaPush<bool>(l, ImGui::GetIO().KeyShift);
	return 1;
}

static int l_attr_key_alt(lua_State *l) {
	LuaPush<bool>(l, ImGui::GetIO().KeyAlt);
	return 1;
}

static int l_attr_screen_height(lua_State *l) {
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	LuaPush<int>(l,Graphics::GetScreenHeight());
	return 1;
}

static int l_pigui_radial_menu(lua_State *l) {
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	std::string id = LuaPull<std::string>(l, 2);
	std::vector<ImTextureID> tex_ids;
	std::vector<std::pair<ImVec2,ImVec2>> uvs;
	int i = 0;
	while(true) {
		lua_rawgeti(l, 3, ++i);
		if(lua_isnil(l, -1)) {
			lua_pop(l, 1);
			break;
		}
		if(!lua_istable(l, -1)) {
			Output("element of icons not a table %i\n", i);
			break;
		}
		lua_getfield(l, -1, "id");
		ImTextureID tid = pi_lua_checklightuserdata(l, -1);
		lua_pop(l, 1);
		lua_getfield(l, -1, "uv0");
		LuaTable xuv0(l, -1);
		ImVec2 uv0(xuv0.Get<double>("x"), xuv0.Get<double>("y"));
		lua_pop(l, 1);
		lua_getfield(l, -1, "uv1");
		LuaTable xuv1(l, -1);
		ImVec2 uv1(xuv1.Get<double>("x"), xuv1.Get<double>("y"));
		lua_pop(l, 1);
		lua_pop(l, 1);
		tex_ids.push_back(tid);
		uvs.push_back(std::pair<ImVec2,ImVec2>(uv0, uv1));
	}

	std::string fontname = LuaPull<std::string>(l, 4);
	int size = LuaPull<int>(l, 5);
	//	ImFont *font = get_font(fontname, size);
	std::vector<std::string> tooltips;
	LuaTable tts(l, 6);
	for(LuaTable::VecIter<std::string> iter = tts.Begin<std::string>(); iter != tts.End<std::string>(); ++iter) {
		tooltips.push_back(*iter);
	}
	int n = PiGui::RadialPopupSelectMenu(center, id, tex_ids, uvs, size, tooltips);
	LuaPush<int>(l, n);
	return 1;
}

static int l_pigui_should_draw_ui(lua_State *l) {
	LuaPush(l, Pi::DrawGUI);
	return 1;
}

static int l_pigui_is_mouse_hovering_rect(lua_State *l) {
	ImVec2 r_min = LuaPull<ImVec2>(l, 1);
	ImVec2 r_max = LuaPull<ImVec2>(l, 2);
	bool clip = LuaPull<bool>(l, 3);
	LuaPush<bool>(l, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
	return 1;
}

static int l_pigui_data_dir_path(lua_State *l) {
	std::string path = FileSystem::GetDataDir();
	LuaTable pathComponents(l, 1);
	for(LuaTable::VecIter<std::string> iter = pathComponents.Begin<std::string>(); iter != pathComponents.End<std::string>(); ++iter) {
		path = FileSystem::JoinPath(path, *iter);
	}
	LuaPush(l, path);
	return 1;
}

static int l_pigui_is_mouse_hovering_any_window(lua_State *l) {
	LuaPush<bool>(l, ImGui::IsMouseHoveringAnyWindow());
	return 1;
}

static int l_pigui_is_mouse_hovering_window(lua_State *l) {
	LuaPush<bool>(l, ImGui::IsMouseHoveringWindow());
	return 1;
}

static int l_pigui_system_info_view_next_page(lua_State *l) {
	Pi::game->GetSystemInfoView()->NextPage();
	return 0;
}

static int l_pigui_input_text(lua_State *l) {
	std::string label = LuaPull<std::string>(l, 1);
	std::string text = LuaPull<std::string>(l, 2);
	int flags = LuaPull<ImGuiInputTextFlags_>(l, 3);
	// callback
	// user_data
	char buffer[1024];
	memset(buffer, 0, 1024);
	strncpy(buffer, text.c_str(), 1023);
	bool result = ImGui::InputText(label.c_str(), buffer, 1024, flags);
	LuaPush<const char*>(l, buffer);
	LuaPush<bool>(l, result);
	return 2;
}

static int l_pigui_play_sfx(lua_State *l) {
	std::string name = LuaPull<std::string>(l, 1);
	double left = LuaPull<float>(l, 2);
	double right = LuaPull<float>(l, 3);
	Sound::PlaySfx(name.c_str(), left, right, false);
	return 0;
}

static int l_pigui_circular_slider(lua_State *l) {
	ImVec2 center = LuaPull<ImVec2>(l, 1);
	float v = LuaPull<double>(l, 2);
	float v_min = LuaPull<double>(l, 3);
	float v_max = LuaPull<double>(l, 4);
	bool res = PiGui::CircularSlider(center, &v, v_min, v_max);
	if(res)
		LuaPush<double>(l, v);
	else
		lua_pushnil(l);
	return 1;
}

static int l_pigui_is_key_released(lua_State *l) {
	int key = LuaPull<int>(l, 1);
	LuaPush<bool>(l, ImGui::IsKeyReleased(key));
	return 1;
}

static int l_pigui_get_cursor_pos(lua_State *l) {
	ImVec2 v = ImGui::GetCursorPos();
	LuaPush<ImVec2>(l, v);
	return 1;
}

static int l_pigui_set_cursor_pos(lua_State *l) {
	ImVec2 v = LuaPull<ImVec2>(l, 1);
	ImGui::SetCursorPos(v);
	return 0;
}

static int l_pigui_drag_int_4(lua_State *l) {
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

static int l_pigui_add_convex_poly_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	LuaTable pnts(l, 1);
	ImColor col = LuaPull<ImColor>(l, 2);
	bool anti_aliased = LuaPull<bool>(l, 3);
	std::vector<ImVec2> ps;
	int i = 0;
	double x = 0.0, y = 0.0;
	for(LuaTable::VecIter<double> iter = pnts.Begin<double>(); iter != pnts.End<double>(); ++iter) {
		if(i++ % 2) {
			y = *iter;
			ps.push_back(ImVec2(x, y));
		} else
			x = *iter;
	}
	draw_list->AddConvexPolyFilled(ps.data(), ps.size(), col, anti_aliased);
	return 0;
}

static int l_pigui_load_texture_from_svg(lua_State *l) {
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	std::string svg_filename = LuaPull<std::string>(l, 2);
	int width = LuaPull<int>(l, 3);
	int height = LuaPull<int>(l, 4);
	ImTextureID id = pigui->RenderSVG(svg_filename, width, height);
	//	LuaPush(l, id);
	lua_pushlightuserdata(l, id);
	return 1;
}

static int l_pigui_set_scroll_here(lua_State *l) {
	ImGui::SetScrollHere();
	return 0;
}

static int l_pigui_pop_text_wrap_pos(lua_State *l) {
	ImGui::PopTextWrapPos();
	return 0;
}

static int l_pigui_push_text_wrap_pos(lua_State *l) {
	float wrap_pos_x = LuaPull<float>(l, 1);
	ImGui::PushTextWrapPos(wrap_pos_x);
	return 0;
}

template <> const char *LuaObject<PiGui>::s_type = "PiGui";

template <> void LuaObject<PiGui>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "Begin",                  l_pigui_begin },
		{ "End",                    l_pigui_end   },
		{ "PushClipRectFullScreen", l_pigui_push_clip_rect_full_screen },
		{ "PopClipRect",            l_pigui_pop_clip_rect },
		{ "PushClipRect",           l_pigui_push_clip_rect },
		{ "AddCircle",              l_pigui_add_circle },
		{ "AddCircleFilled",        l_pigui_add_circle_filled },
		{ "AddLine",                l_pigui_add_line },
		{ "AddText",                l_pigui_add_text },
		{ "AddTriangle",            l_pigui_add_triangle },
		{ "AddTriangleFilled",      l_pigui_add_triangle_filled },
		{ "AddQuad",                l_pigui_add_quad },
		{ "AddRect",                l_pigui_add_rect },
		{ "AddRectFilled",          l_pigui_add_rect_filled },
		{ "AddImage",               l_pigui_add_image },
		{ "AddImageQuad",           l_pigui_add_image_quad },
		{ "AddBezierCurve",         l_pigui_add_bezier_curve },
		{ "SetNextWindowPos",       l_pigui_set_next_window_pos },
		{ "SetNextWindowSize",      l_pigui_set_next_window_size },
		{ "SetNextWindowFocus",     l_pigui_set_next_window_focus },
		{ "SetWindowFocus",         l_pigui_set_window_focus },
		//		{ "GetHUDMarker",           l_pigui_get_hud_marker },
		//    { "GetVelocity",            l_pigui_get_velocity },
		{ "PushStyleColor",         l_pigui_push_style_color },
		{ "PopStyleColor",          l_pigui_pop_style_color },
		{ "PushStyleVar",           l_pigui_push_style_var },
		{ "PopStyleVar",            l_pigui_pop_style_var },
		{ "Columns",                l_pigui_columns },
		{ "NextColumn",             l_pigui_next_column },
		{ "Text",                   l_pigui_text },
		{ "TextWrapped",            l_pigui_text_wrapped },
		{ "TextColored",            l_pigui_text_colored },
		{ "SetScrollHere",          l_pigui_set_scroll_here },
		{ "Button",                 l_pigui_button },
		{ "Selectable",             l_pigui_selectable },
		{ "BeginGroup",             l_pigui_begin_group },
		{ "SetCursorPos",           l_pigui_set_cursor_pos },
		{ "GetCursorPos",           l_pigui_get_cursor_pos },
		{ "EndGroup",               l_pigui_end_group },
		{ "SameLine",               l_pigui_same_line },
		{ "Separator",              l_pigui_separator },
		{ "IsItemHovered",          l_pigui_is_item_hovered },
		{ "IsItemClicked",          l_pigui_is_item_clicked },
		{ "Spacing",                l_pigui_spacing },
		{ "Dummy",                  l_pigui_dummy },
		{ "BeginChild",             l_pigui_begin_child },
		{ "EndChild",               l_pigui_end_child },
		{ "PushFont",               l_pigui_push_font },
		{ "PopFont",                l_pigui_pop_font },
		{ "CalcTextSize",           l_pigui_calc_text_size },
		{ "SetTooltip",             l_pigui_set_tooltip },
		{ "Checkbox",               l_pigui_checkbox },
		{ "GetMousePos",            l_pigui_get_mouse_pos },
		{ "GetMouseWheel",          l_pigui_get_mouse_wheel },
		{ "PathArcTo",              l_pigui_path_arc_to },
		{ "PathStroke",             l_pigui_path_stroke },
		{ "PushItemWidth",          l_pigui_push_item_width },
		{ "PopItemWidth",           l_pigui_pop_item_width },
		{ "PushTextWrapPos",        l_pigui_push_text_wrap_pos },
		{ "PopTextWrapPos",         l_pigui_pop_text_wrap_pos },
		{ "BeginPopup",             l_pigui_begin_popup },
		{ "EndPopup",               l_pigui_end_popup },
		{ "OpenPopup",              l_pigui_open_popup },
		{ "PushID",                 l_pigui_push_id },
		{ "PopID",                  l_pigui_pop_id },
		{ "IsMouseReleased",        l_pigui_is_mouse_released },
		{ "IsMouseClicked",         l_pigui_is_mouse_clicked },
		{ "IsMouseDown",            l_pigui_is_mouse_down },
		{ "IsMouseHoveringRect",    l_pigui_is_mouse_hovering_rect },
		{ "IsMouseHoveringAnyWindow",    l_pigui_is_mouse_hovering_any_window },
		{ "IsMouseHoveringWindow",  l_pigui_is_mouse_hovering_window },
		{ "Image",                  l_pigui_image },
		{ "ImageButton",            l_pigui_image_button },
		{ "RadialMenu",             l_pigui_radial_menu },
		{ "CircularSlider",         l_pigui_circular_slider },
		{ "GetMouseClickedPos",     l_pigui_get_mouse_clicked_pos },
		{ "AddConvexPolyFilled",    l_pigui_add_convex_poly_filled },
		{ "IsKeyReleased",          l_pigui_is_key_released },
		{ "DragInt4",               l_pigui_drag_int_4 },
		{ "GetWindowPos",           l_pigui_get_window_pos },
		{ "InputText",              l_pigui_input_text },
		{ "CaptureMouseFromApp",    l_pigui_capture_mouse_from_app },
		{ "ProgressBar",            l_pigui_progress_bar },
		{ "LoadTextureFromSVG",     l_pigui_load_texture_from_svg },
		{ "DataDirPath",            l_pigui_data_dir_path },
		{ "ShouldDrawUI",           l_pigui_should_draw_ui },
		{ "GetTargetsNearby",       l_pigui_get_targets_nearby },
		{ "GetProjectedBodies",     l_pigui_get_projected_bodies },
		{ "ShouldShowLabels",       l_pigui_should_show_labels },
		{ "SystemInfoViewNextPage", l_pigui_system_info_view_next_page }, // deprecated
		{ "LowThrustButton",        l_pigui_low_thrust_button },
		{ "ThrustIndicator",        l_pigui_thrust_indicator },
		{ "PlaySfx",                l_pigui_play_sfx },
		// { "DisableMouseFacing",     l_pigui_disable_mouse_facing },
		// { "SetMouseButtonState",    l_pigui_set_mouse_button_state },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "handlers",      l_attr_handlers },
		{ "screen_width",  l_attr_screen_width },
		{ "screen_height", l_attr_screen_height },
		{ "key_ctrl",      l_attr_key_ctrl },
		{ "key_none",      l_attr_key_none },
		{ "key_shift",     l_attr_key_shift },
		{ "key_alt",       l_attr_key_alt },
		{ "keys",          l_attr_keys },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, l_methods, l_attrs, 0);
}
