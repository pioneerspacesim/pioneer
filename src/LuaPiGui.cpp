// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"
#include "PiGui.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "WorldView.h"
#include "Pi.h"
#include "Game.h"
#include "graphics/Graphics.h"
// #include "FileSystem.h"
// #include "Player.h"
// #include "Pi.h"
// #include "Game.h"
// #include "Lang.h"
// #include "StringF.h"
// #include "WorldView.h"
// #include "DeathView.h"

static ImVec2 luaL_checkImVec2(lua_State *l, int index) {
	ScopedTable vec = LuaTable(l, index);
	return ImVec2(vec.Get<double>("x"), vec.Get<double>("y"));
}

static ImColor luaL_checkImColor(lua_State *l, int index) {
	ScopedTable c = LuaTable(l, index);
	return ImColor(c.Get<int>("r"), c.Get<int>("g"), c.Get<int>("b"), c.Get<int>("a", 255));
}

static ImGuiSelectableFlags luaL_checkImGuiSelectableFlags(lua_State *l, int index) {
	// TODO: make this a dispatch table
	LuaTable flags(l, index);
	int theflags = 0;
	for(LuaTable::VecIter<std::string> iter = flags.Begin<std::string>(); iter != flags.End<std::string>(); ++iter) {
		std::string flag = *iter;
	if(!flag.compare("DontClosePopups"))
		theflags |= ImGuiSelectableFlags_DontClosePopups;
	else if (!flag.compare("SpanAllColumns"))
		theflags |= ImGuiSelectableFlags_SpanAllColumns;
	else if (!flag.compare("AllowDoubleClick"))
		theflags |= ImGuiSelectableFlags_AllowDoubleClick;
	}
	return theflags;
}

static ImGuiSetCond luaL_checkImGuiSetCond(lua_State *l, int index) {
	// TODO: make this a dispatch table
	std::string condstr = luaL_checkstring(l, index);
	if(!condstr.compare("Always"))
		return ImGuiSetCond_Always;
	else if (!condstr.compare("Once"))
		return ImGuiSetCond_Once;
	else if (!condstr.compare("FirstUseEver"))
		return ImGuiSetCond_FirstUseEver;
	else if (!condstr.compare("Appearing"))
		return ImGuiSetCond_Appearing;
	// TODO: else error
	return -1; 
}

static ImGuiCol luaL_checkImGuiCol(lua_State *l, int index) {
	std::string stylestr = luaL_checkstring(l, index);
	if(!stylestr.compare("WindowBg"))
		return ImGuiCol_WindowBg;
	// TODO: else error
	return -1;
}

static bool luaL_checkbool(lua_State *l, int index) {
   if ( lua_isboolean( l, index ) )
		 return lua_toboolean( l, index );
    else
			Output("Error: Cannot convert non 'boolean' value to bool");
	 return false;
}

// /*
//  * Interface: Game
//  *
//  * A global table that exposes a number of essential values relevant to the
//  * current game.
//  *
//  */

// /*
//  * Function: StartGame
//  *
//  * Start a new game.
//  *
//  * > Game.StartGame(system_path, start_time)
//  *
//  * Parameters:
//  *
//  *   system_path - A SystemBody to start at. If this is a starport, the player
//  *                 will begin docked here; otherwise the player will begin in
//  *                 orbit around the specified body.
//  *
//  *   start_time - optional, default 0. Time to start at in seconds from the
//  *                Pioneer epoch (i.e. from 3200-01-01 00:00 UTC).
//  *
//  * Availability:
//  *
//  *   alpha 28
//  *
//  * Status:
//  *
//  *   experimental
//  */
// static int l_game_start_game(lua_State *l)
// {
// 	if (Pi::game) {
// 		luaL_error(l, "can't start a new game while a game is already running");
// 		return 0;
// 	}

// 	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
// 	const double start_time = luaL_optnumber(l, 2, 0.0);
// 	try {
// 		Pi::game = new Game(*path, start_time);
// 	}
// 	catch (InvalidGameStartLocation& e) {
// 		luaL_error(l, "invalid starting location for game: %s", e.error.c_str());
// 	}
// 	return 0;
// }

// /*
//  * Function: LoadGame
//  *
//  * Load a saved game.
//  *
//  * > Game.LoadGame(filename)
//  *
//  * Parameters:
//  *
//  *   filename - Filename to load. It will be loaded from the 'savefiles'
//  *              directory in the user's game directory.
//  *
//  * Availability:
//  *
//  *   alpha 28
//  *
//  * Status:
//  *
//  *   experimental
//  */
// static int l_game_load_game(lua_State *l)
// {
// 	if (Pi::game) {
// 		luaL_error(l, "can't load a game while a game is already running");
// 		return 0;
// 	}

// 	const std::string filename(luaL_checkstring(l, 1));

// 	try {
// 		Pi::game = Game::LoadGame(filename);
// 	}
// 	catch (SavedGameCorruptException) {
// 		luaL_error(l, Lang::GAME_LOAD_CORRUPT);
// 	}
// 	catch (SavedGameWrongVersionException) {
// 		luaL_error(l, Lang::GAME_LOAD_WRONG_VERSION);
// 	}
// 	catch (CouldNotOpenFileException) {
// 		const std::string msg = stringf(Lang::GAME_LOAD_CANNOT_OPEN,
// 			formatarg("filename", filename));
// 		luaL_error(l, msg.c_str());
// 	}

// 	return 0;
// }

// /*
//  * Function: CanLoadGame
//  *
//  * Does file exist for loading.
//  *
//  * > Game.CanLoadGame(filename)
//  *
//  * Parameters:
//  *
//  *   filename - Filename to find.
//  *
//  * Return:
//  *
//  *   bool - can the filename be found to load
//  *
//  * Availability:
//  *
//  *   YYYY - MM - DD
//  *   2016 - 06 - 25
//  *
//  * Status:
//  *
//  *   experimental
//  */
// static int l_game_can_load_game(lua_State *l)
// {
// 	const std::string filename(luaL_checkstring(l, 1));

// 	bool success = Game::CanLoadGame(filename);
// 	lua_pushboolean(l, success);

// 	return 1;
// }

// /*
//  * Function: SaveGame
//  *
//  * Save the current game.
//  *
//  * > path = Game.SaveGame(filename)
//  *
//  * Parameters:
//  *
//  *   filename - Filename to save to. The file will be placed the 'savefiles'
//  *              directory in the user's game directory.
//  *
//  * Return:
//  *
//  *   path - the full path to the saved file (so it can be displayed)
//  *
//  * Availability:
//  *
//  *   June 2013
//  *
//  * Status:
//  *
//  *   experimental
//  */
// static int l_game_save_game(lua_State *l)
// {
// 	if (!Pi::game) {
// 		return luaL_error(l, "can't save when no game is running");
// 	}

// 	const std::string filename(luaL_checkstring(l, 1));
// 	const std::string path = FileSystem::JoinPathBelow(Pi::GetSaveDir(), filename);

// 	try {
// 		Game::SaveGame(filename, Pi::game);
// 		lua_pushlstring(l, path.c_str(), path.size());
// 		return 1;
// 	}
// 	catch (CannotSaveInHyperspace) {
// 		return luaL_error(l, "%s", Lang::CANT_SAVE_IN_HYPERSPACE);
// 	}
// 	catch (CannotSaveDeadPlayer) {
// 		return luaL_error(l, "%s", Lang::CANT_SAVE_DEAD_PLAYER);
// 	}
// 	catch (CouldNotOpenFileException) {
// 		const std::string message = stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path));
// 		lua_pushlstring(l, message.c_str(), message.size());
// 		return lua_error(l);
// 	}
// 	catch (CouldNotWriteToFileException) {
// 		return luaL_error(l, "%s", Lang::GAME_SAVE_CANNOT_WRITE);
// 	}
// }

// /*
//  * Function: EndGame
//  *
//  * End the current game and return to the main menu.
//  *
//  * > Game.EndGame(filename)
//  *
//  * Availability:
//  *
//  *   June 2013
//  *
//  * Status:
//  *
//  *   experimental
//  */
// static int l_game_end_game(lua_State *l)
// {
// 	if (Pi::game) {
// 		// Request to end the game as soon as possible.
// 		// Previously could be called from Lua UI and delete the object doing the calling causing a crash.
// 		Pi::RequestEndGame();
// 	}
// 	return 0;
// }

// /*
//  * Attribute: player
//  *
//  * The <Player> object for the current player.
//  *
//  * Availability:
//  *
//  *  alpha 10
//  *
//  * Status:
//  *
//  *  stable
//  */
// static int l_game_attr_player(lua_State *l)
// {
// 	if (!Pi::game)
// 		lua_pushnil(l);
// 	else
// 		LuaObject<Player>::PushToLua(Pi::player);
// 	return 1;
// }

// /*
//  * Attribute: system
//  *
//  * The <StarSystem> object for the system the player is currently in.
//  *
//  * Availability:
//  *
//  *  alpha 10
//  *
//  * Status:
//  *
//  *  stable
//  */
// static int l_game_attr_system(lua_State *l)
// {
// 	if (!Pi::game)
// 		lua_pushnil(l);
// 	else
// 		LuaObject<StarSystem>::PushToLua(Pi::game->GetSpace()->GetStarSystem().Get());
// 	return 1;
// }

// /*
//  * Attribute: time
//  *
//  * The current game time, in seconds since 12:00 01-01-3200
//  *
//  * Availability:
//  *
//  *  alpha 10
//  *
//  * Status:
//  *
//  *  stable
//  */
// static int l_game_attr_time(lua_State *l)
// {
// 	if (!Pi::game)
// 		lua_pushnil(l);
// 	else
// 		lua_pushnumber(l, Pi::game->GetTime());
// 	return 1;
// }

// /*
//  * Attribute: paused
//  *
//  * True if the game is paused.
//  *
//  * Availability:
//  *
//  *  September 2014
//  *
//  * Status:
//  *
//  *  experimental
//  */
// static int l_game_attr_paused(lua_State *l)
// {
// 	if (!Pi::game)
// 		lua_pushboolean(l, 1);
// 	else
// 		lua_pushboolean(l, Pi::game->IsPaused() ? 1 : 0);
// 	return 1;
// }

// // XXX temporary to support StationView "Launch" button
// // remove once WorldView has been converted to the new UI
// static int l_game_switch_view(lua_State *l)
// {
// 	if (!Pi::game)
// 		return luaL_error(l, "can't switch view when no game is running");
// 	if (Pi::player->IsDead())
// 		Pi::SetView(Pi::game->GetDeathView());
// 	else
// 		Pi::SetView(Pi::game->GetWorldView());
// 	return 0;
// }

static int l_pigui_begin(lua_State *l) {
	const std::string name = luaL_checkstring(l, 1);
	LuaTable flags(l, 2);
	int theflags = 0;
	for(LuaTable::VecIter<std::string> iter = flags.Begin<std::string>(); iter != flags.End<std::string>(); ++iter) {
		std::string flag = *iter;
		if(!flag.compare("NoTitleBar"))
			theflags |= ImGuiWindowFlags_NoTitleBar;
		else if(!flag.compare("NoInputs"))
			theflags |= ImGuiWindowFlags_NoInputs;
		else if(!flag.compare("NoMove"))
			theflags |= ImGuiWindowFlags_NoMove;
		else if(!flag.compare("NoResize"))
			theflags |= ImGuiWindowFlags_NoResize;
		else if(!flag.compare("NoSavedSettings"))
			theflags |= ImGuiWindowFlags_NoSavedSettings;
		else if(!flag.compare("NoFocusOnAppearing"))
			theflags |= ImGuiWindowFlags_NoFocusOnAppearing;
		else if(!flag.compare("NoBringToFrontOnFocus"))
			theflags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	}
	ImGui::Begin(name.c_str(), nullptr, theflags);
	return 0;
}

static int l_pigui_columns(lua_State *l) {
	int columns = luaL_checkinteger(l, 1);
	std::string id = luaL_checkstring(l, 2);
	bool border = luaL_checkbool(l, 3);
	ImGui::Columns(columns, id.c_str(), border);
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

static int l_pigui_push_clip_rect_full_screen(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRectFullScreen();
	return 0;
}

static int l_pigui_set_next_window_pos(lua_State *l) {
	ImVec2 pos = luaL_checkImVec2(l, 1);
	int cond = luaL_checkImGuiSetCond(l, 2);
	ImGui::SetNextWindowPos(pos, cond);
	return 0;
}

static int l_pigui_set_next_window_size(lua_State *l) {
	ImVec2 size = luaL_checkImVec2(l, 1);
	int cond = luaL_checkImGuiSetCond(l, 2);
	ImGui::SetNextWindowSize(size, cond);
	return 0;
}

static int l_pigui_push_style_color(lua_State *l) {
	int style = luaL_checkImGuiCol(l, 1);
	ImColor color = luaL_checkImColor(l, 2);
	ImGui::PushStyleColor(style, color);
	return 0;
}

static int l_pigui_pop_style_color(lua_State *l) {
	int num = luaL_checkinteger(l, 1);
	ImGui::PopStyleColor(num);
	return 0;
}

static int l_pigui_add_line(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImColor col = luaL_checkImColor(l, 3);
	double thickness = luaL_checknumber(l, 4);
	draw_list->AddLine(a, b, col, thickness);
	return 0;
}

static int l_pigui_add_circle(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = luaL_checkImVec2(l, 1);
	int radius = luaL_checkinteger(l, 2);
	ImColor color = luaL_checkImColor(l, 3);
	int segments = luaL_checkinteger(l, 4);
	double thickness = luaL_checknumber(l, 5);
	draw_list->AddCircle(center, radius, color, segments, thickness);
	return 0;
}

static int l_pigui_add_circle_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = luaL_checkImVec2(l, 1);
	int radius = luaL_checkinteger(l, 2);
	ImColor color = luaL_checkImColor(l, 3);
	int segments = luaL_checkinteger(l, 4);
	draw_list->AddCircleFilled(center, radius, color, segments);
	return 0;
}

static int l_pigui_path_arc_to(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = luaL_checkImVec2(l, 1);
	double radius = luaL_checknumber(l, 2);
	double amin = luaL_checknumber(l, 3);
	double amax = luaL_checknumber(l, 4);
	int segments = luaL_checkinteger(l, 5);
	draw_list->PathArcTo(center, radius, amin, amax, segments);
	return 0;
}

static int l_pigui_path_stroke(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImColor color = luaL_checkImColor(l, 1);
	bool closed = luaL_checkbool(l, 2);
	double thickness = luaL_checknumber(l, 3);
	draw_list->PathStroke(color, closed, thickness);
	return 0;
}

static int l_pigui_selectable(lua_State *l) {
	std::string label = luaL_checkstring(l, 1);
	bool selected = luaL_checkbool(l, 2);
	ImGuiSelectableFlags flags = luaL_checkImGuiSelectableFlags(l, 3);
	// TODO: parameter size
	bool res = ImGui::Selectable(label.c_str(), selected, flags);
	lua_pushboolean(l, res);
	return 1;
}

static int l_pigui_text(lua_State *l) {
	std::string text = luaL_checkstring(l, 1);
	ImGui::Text(text.c_str());
	return 0;
}

static int l_pigui_get_velocity(lua_State *l) {
	std::string name = luaL_checkstring(l, 1);
	WorldView *wv = Pi::game->GetWorldView();
	vector3d velocity;
	if(!name.compare("nav_prograde")) {
		velocity = wv->GetNavProgradeVelocity();
	} else if(!name.compare("frame_prograde")) {
		velocity = wv->GetFrameProgradeVelocity();
	} else
		return 0;
	LuaTable vel(l);
	vel.Set("x", velocity.x);
	vel.Set("y", velocity.y);
	vel.Set("z", velocity.z);
	return 1;
}

static int l_pigui_get_hud_marker(lua_State *l) {
	std::string name = luaL_checkstring(l, 1);
	WorldView *wv = Pi::game->GetWorldView();
	const Indicator *marker;
	
	if(!name.compare("frame_prograde"))
		marker = wv->GetFrameProgradeIndicator();
	else if(!name.compare("frame_retrograde"))
		marker = wv->GetFrameRetrogradeIndicator();
	else if(!name.compare("nav_prograde"))
		marker = wv->GetNavProgradeIndicator();
	else if(!name.compare("nav_retrograde"))
		marker = wv->GetNavRetrogradeIndicator();
	else if(!name.compare("frame"))
		marker = wv->GetFrameIndicator();
	else if(!name.compare("nav"))
		marker = wv->GetNavIndicator();
	else
		// TODO: error
		return 0;
	// all ui calculations seem to use 800x600
	vector2f position = marker->pos - vector2f(400,300);
	vector2f direction = position.Normalized();
	std::string side = marker->side == INDICATOR_HIDDEN ? "hidden" : (marker->side == INDICATOR_ONSCREEN ? "onscreen" : "other");
	lua_pushstring(l, side.c_str());
	LuaTable dir(l);
	dir.Set("x", direction.x);
	dir.Set("y", direction.y);
	LuaTable pos(l);
	pos.Set("x", marker->pos.x / 800 * Graphics::GetScreenWidth());
	pos.Set("y", marker->pos.y / 600 * Graphics::GetScreenHeight());
	return 3;
}

static int l_pigui_add_text(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = luaL_checkImVec2(l, 1);
	ImColor color = luaL_checkImColor(l, 2);
	std::string text = luaL_checkstring(l, 3);
	draw_list->AddText(center, color, text.c_str());
	return 0;
}

static int l_pigui_add_triangle(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImVec2 c = luaL_checkImVec2(l, 3);
	ImColor col = luaL_checkImColor(l, 4);
	float thickness = luaL_checknumber(l, 5);
	draw_list->AddTriangle(a, b, c, col, thickness);
	return 0;
}

static int l_pigui_add_quad(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImVec2 c = luaL_checkImVec2(l, 3);
	ImVec2 d = luaL_checkImVec2(l, 4);
	ImColor col = luaL_checkImColor(l, 5);
	float thickness = luaL_checknumber(l, 6);
	draw_list->AddQuad(a, b, c, d, col, thickness);
	return 0;
}

static int l_pigui_add_triangle_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImVec2 c = luaL_checkImVec2(l, 3);
	ImColor col = luaL_checkImColor(l, 4);
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
	ImVec2 size = luaL_checkImVec2(l, 1);
	ImGui::Dummy(size);
	return 0;
}

static int l_pigui_begin_child(lua_State *l) {
	std::string id = luaL_checkstring(l, 1);
	ImGui::BeginChild(id.c_str());
	return 0;
}

static int l_pigui_end_child(lua_State *l) {
	ImGui::EndChild();
	return 0;
}

static int l_pigui_is_item_hovered(lua_State *l) {
	lua_pushboolean(l, ImGui::IsItemHovered());
	return 1;
}

static int l_pigui_is_item_clicked(lua_State *l) {
	int button = luaL_checkinteger(l, 1);
	lua_pushboolean(l, ImGui::IsItemClicked(button));
	return 1;
}

static int l_pigui_push_font(lua_State *l) {
	std::string fontname = luaL_checkstring(l, 1);
	int size = luaL_checkinteger(l, 2);
	ImFont *font;
	if(!fontname.compare("pionillium")) {
		switch(size) {
		case 12: font = PiGui::pionillium12; break;
		case 18: font = PiGui::pionillium18; break;
		case 36: font = PiGui::pionillium36; break;
		default: return 0;
		}
	} else
		return 0;
	ImGui::PushFont(font);
	return 0;
}

static int l_pigui_pop_font(lua_State *l) {
	ImGui::PopFont();
	return 0;
}

static int l_pigui_calc_text_size(lua_State *l) {
	std::string text = luaL_checkstring(l, 1);
	ImVec2 size = ImGui::CalcTextSize(text.c_str());
	LuaTable s(l);
	s.Set("x", size.x);
	s.Set("y", size.y);
	return 1;
}

static int l_pigui_get_mouse_pos(lua_State *l) {
	ImVec2 pos = ImGui::GetMousePos();
	LuaTable p(l);
	p.Set("x", pos.x);
	p.Set("y", pos.y);
	return 1;
}

static int l_pigui_set_tooltip(lua_State *l) {
	std::string text = luaL_checkstring(l, 1);
	ImGui::SetTooltip(text.c_str());
	return 0;
}

static int l_pigui_checkbox(lua_State *l) {
	std::string label = luaL_checkstring(l, 1);
	bool checked = luaL_checkbool(l, 2);
	ImGui::Checkbox(label.c_str(), &checked);
	lua_pushboolean(l, checked);
	return 1;
}

static int l_attr_handlers(lua_State *l) {
	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	pigui->GetHandlers().PushCopyToStack();
	return 1;
}

static int l_attr_screen_width(lua_State *l) {
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	lua_pushinteger(l,Graphics::GetScreenWidth());
	return 1;
}

static int l_attr_screen_height(lua_State *l) {
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	lua_pushinteger(l,Graphics::GetScreenHeight());
	return 1;
}

template <> const char *LuaObject<PiGui>::s_type = "PiGui";

template <> void LuaObject<PiGui>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "Begin",                  l_pigui_begin },
		{ "End",                    l_pigui_end   },
		{ "PushClipRectFullScreen", l_pigui_push_clip_rect_full_screen },
		{ "PushClipRect",           l_pigui_pop_clip_rect },
		{ "AddCircle",              l_pigui_add_circle },
		{ "AddCircleFilled",        l_pigui_add_circle_filled },
		{ "AddLine",                l_pigui_add_line },
		{ "AddText",                l_pigui_add_text },
		{ "AddTriangle",            l_pigui_add_triangle },
		{ "AddTriangleFilled",      l_pigui_add_triangle_filled },
		{ "AddQuad",                l_pigui_add_quad },
		{ "SetNextWindowPos",       l_pigui_set_next_window_pos },
		{ "SetNextWindowSize",      l_pigui_set_next_window_size },
		{ "GetHUDMarker",           l_pigui_get_hud_marker },
		{ "GetVelocity",            l_pigui_get_velocity },
		{ "PushStyleColor",         l_pigui_push_style_color },
		{ "PopStyleColor",          l_pigui_pop_style_color },
		{ "Columns",                l_pigui_columns },
		{ "NextColumn",             l_pigui_next_column },
		{ "Text",                   l_pigui_text },
		{ "Selectable",             l_pigui_selectable },
		{ "BeginGroup",             l_pigui_begin_group },
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
		{ "PathArcTo",              l_pigui_path_arc_to },
		{ "PathStroke",             l_pigui_path_stroke },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "handlers", l_attr_handlers },
		{ "screen_width", l_attr_screen_width },
		{ "screen_height", l_attr_screen_height },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, l_methods, l_attrs, 0);
}
