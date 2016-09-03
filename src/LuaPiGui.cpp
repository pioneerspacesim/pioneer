// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "PiGui.h"
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

#undef RegisterClass

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
	else if(!stylestr.compare("Button"))
		return ImGuiCol_Button;
	// TODO: else error
	return -1;
}

static ImGuiStyleVar luaL_checkImGuiStyleVar(lua_State *l, int index) {
	std::string stylestr = luaL_checkstring(l, index);
	if(!stylestr.compare("WindowRounding"))
		return ImGuiStyleVar_WindowRounding;
	// TODO: else error
	return -1;
}

bool luaL_checkbool(lua_State *l, int index) {
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

static int l_pigui_push_clip_rect(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 min = luaL_checkImVec2(l, 1);
	ImVec2 max = luaL_checkImVec2(l, 2);
	bool intersect = luaL_checkbool(l, 3);
	draw_list->PushClipRect(min, max, intersect);
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

static int l_pigui_set_next_window_focus(lua_State *l) {
	ImGui::SetNextWindowFocus();
	return 0;
}

static int l_pigui_set_window_focus(lua_State *l) {
	std::string name = luaL_checkstring(l, 1);
	ImGui::SetWindowFocus(name.c_str());
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

static int l_pigui_push_style_var(lua_State *l) {
	int style = luaL_checkImGuiStyleVar(l, 1);
	float val = luaL_checknumber(l, 2);
	ImGui::PushStyleVar(style, val);
	return 0;
}

static int l_pigui_pop_style_var(lua_State *l) {
	int num = luaL_checkinteger(l, 1);
	ImGui::PopStyleVar(num);
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

static int l_pigui_button(lua_State *l) {
	std::string text = luaL_checkstring(l, 1);
	bool ret = ImGui::Button(text.c_str());
	lua_pushboolean(l, ret);
	return 1;
}

static int l_pigui_text_wrapped(lua_State *l) {
	std::string text = luaL_checkstring(l, 1);
	ImGui::TextWrapped(text.c_str());
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
	else if(!name.compare("forward"))
		marker = wv->GetForwardIndicator();
	else if(!name.compare("backward"))
		marker = wv->GetBackwardIndicator();
	else if(!name.compare("up"))
		marker = wv->GetUpIndicator();
	else if(!name.compare("down"))
		marker = wv->GetDownIndicator();
	else if(!name.compare("left"))
		marker = wv->GetLeftIndicator();
	else if(!name.compare("right"))
		marker = wv->GetRightIndicator();
	else if(!name.compare("normal"))
		marker = wv->GetNormalIndicator();
	else if(!name.compare("anti_normal"))
		marker = wv->GetAntiNormalIndicator();
	else if(!name.compare("radial_in"))
		marker = wv->GetRadialOutIndicator();
	else if(!name.compare("radial_out"))
		marker = wv->GetRadialInIndicator();
	else if(!name.compare("away_from_frame"))
		marker = wv->GetAwayFromFrameIndicator();
	else if(!name.compare("combat_target"))
		marker = wv->GetCombatTargetIndicator();
	else if(!name.compare("combat_target_lead"))
		marker = wv->GetCombatTargetLeadIndicator();
	else if(!name.compare("maneuver"))
		marker = wv->GetManeuverIndicator();

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
	dir.Set("z", 0);
	LuaTable pos(l);
	pos.Set("x", marker->pos.x / 800 * Graphics::GetScreenWidth());
	pos.Set("y", marker->pos.y / 600 * Graphics::GetScreenHeight());
	pos.Set("z", 0);
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

static int l_pigui_add_rect(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImColor col = luaL_checkImColor(l, 3);
	float rounding = luaL_checknumber(l, 4);
	int round_corners = luaL_checkinteger(l, 5);
	float thickness = luaL_checknumber(l, 6);
	draw_list->AddRect(a, b, col, rounding, round_corners, thickness);
	return 0;
}

static int l_pigui_add_rect_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 a = luaL_checkImVec2(l, 1);
	ImVec2 b = luaL_checkImVec2(l, 2);
	ImColor col = luaL_checkImColor(l, 3);
	float rounding = luaL_checknumber(l, 4);
	int round_corners = luaL_checkinteger(l, 5);
	draw_list->AddRectFilled(a, b, col, rounding, round_corners);
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

static int l_pigui_begin_popup(lua_State *l) {
	std::string id = luaL_checkstring(l, 1);
	lua_pushboolean(l, ImGui::BeginPopup(id.c_str()));
	return 1;
}

static int l_pigui_open_popup(lua_State *l) {
	std::string id = luaL_checkstring(l, 1);
	ImGui::OpenPopup(id.c_str());
	return 0;
}

static int l_pigui_end_popup(lua_State *l) {
	ImGui::EndPopup();
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

static int l_pigui_is_mouse_released(lua_State *l) {
	int button = luaL_checkinteger(l, 1);
	lua_pushboolean(l, ImGui::IsMouseReleased(button));
	return 1;
}

static int l_pigui_is_mouse_clicked(lua_State *l) {
	int button = luaL_checkinteger(l, 1);
	lua_pushboolean(l, ImGui::IsMouseClicked(button));
	return 1;
}

static ImFont *get_font(std::string fontname, int size) {
	ImFont *font;
	if(!fontname.compare("pionillium")) {
		switch(size) {
		case 12: font = PiGui::pionillium12; break;
		case 15: font = PiGui::pionillium15; break;
		case 18: font = PiGui::pionillium18; break;
		case 30: font = PiGui::pionillium30; break;
		case 36: font = PiGui::pionillium36; break;
		default: return 0;
		}
	} else if (!fontname.compare("pionicons")) {
		switch(size) {
		case 12: font = PiGui::pionicons12; break;
			//		case 18: font = PiGui::pionicons18; break;
		case 30: font = PiGui::pionicons30; break;
		default: return 0;
		}
	} else
		return 0;
	return font;
}

static int l_pigui_push_font(lua_State *l) {
	std::string fontname = luaL_checkstring(l, 1);
	int size = luaL_checkinteger(l, 2);
	ImFont *font = get_font(fontname, size);
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
	s.Set("z", 0);
	return 1;
}

static int l_pigui_get_mouse_pos(lua_State *l) {
	ImVec2 pos = ImGui::GetMousePos();
	LuaTable p(l);
	p.Set("x", pos.x);
	p.Set("y", pos.y);
	p.Set("z", 0);
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
	bool changed = ImGui::Checkbox(label.c_str(), &checked);
	lua_pushboolean(l, changed);
	lua_pushboolean(l, checked);
	return 2;
}

static int l_pigui_push_item_width(lua_State *l) {
	double width = luaL_checknumber(l, 1);
	ImGui::PushItemWidth(width);
	return 0;
}

static int l_pigui_pop_item_width(lua_State *l) {
	ImGui::PopItemWidth();
	return 0;
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

static int l_attr_key_ctrl(lua_State *l) {
	lua_pushboolean(l, ImGui::GetIO().KeyCtrl);
	return 1;
}

static int l_attr_screen_height(lua_State *l) {
	//	PiGui *pigui = LuaObject<PiGui>::CheckFromLua(1);
	lua_pushinteger(l,Graphics::GetScreenHeight());
	return 1;
}

static int l_pigui_get_window_pos(lua_State *l) {
	ImVec2 pos = ImGui::GetWindowPos();
	LuaTable v(l);
	v.Set("x", pos.x);
	v.Set("y", pos.y);
	v.Set("z", 0);
	return 1;
}

static int l_pigui_get_mouse_clicked_pos(lua_State *l) {
	int n = luaL_checkinteger(l, 1);
	ImVec2 pos = ImGui::GetIO().MouseClickedPos[n];
	LuaTable v(l);
	v.Set("x", pos.x);
	v.Set("y", pos.y);
	v.Set("z", 0);
	return 1;
}

static int l_pigui_radial_menu(lua_State *l) {
	ImVec2 center = luaL_checkImVec2(l, 1);
	std::string id = luaL_checkstring(l, 2);
	std::vector<std::string> items;
	LuaTable strings(l, 3);
	for(LuaTable::VecIter<std::string> iter = strings.Begin<std::string>(); iter != strings.End<std::string>(); ++iter) {
		items.push_back(*iter);
	}
	std::string fontname = luaL_checkstring(l, 4);
	int size = luaL_checkinteger(l, 5);
	ImFont *font = get_font(fontname, size);
	std::vector<std::string> tooltips;
	LuaTable tts(l, 6);
	for(LuaTable::VecIter<std::string> iter = tts.Begin<std::string>(); iter != tts.End<std::string>(); ++iter) {
		tooltips.push_back(*iter);
	}
	int n = PiGui::RadialPopupSelectMenu(center, id, items, font, tooltips);
	lua_pushinteger(l, n);
	return 1;
}

static int l_pigui_is_mouse_hovering_rect(lua_State *l) {
	ImVec2 r_min = luaL_checkImVec2(l, 1);
	ImVec2 r_max = luaL_checkImVec2(l, 2);
	bool clip = luaL_checkbool(l, 3);
	lua_pushboolean(l, ImGui::IsMouseHoveringRect(r_min, r_max, clip));
	return 1;
}

static int l_pigui_is_mouse_hovering_any_window(lua_State *l) {
	lua_pushboolean(l, ImGui::IsMouseHoveringAnyWindow());
	return 1;
}

static int l_pigui_circular_slider(lua_State *l) {
	ImVec2 center = luaL_checkImVec2(l, 1);
	float v = luaL_checknumber(l, 2);
	float v_min = luaL_checknumber(l, 3);
	float v_max = luaL_checknumber(l, 4);
	bool res = PiGui::CircularSlider(center, &v, v_min, v_max);
	if(res)
		lua_pushnumber(l, v);
	else
		lua_pushnil(l);
	return 1;
}

static int l_pigui_is_key_released(lua_State *l) {
	int key = luaL_checkinteger(l, 1);
	lua_pushboolean(l, ImGui::IsKeyReleased(key));
	return 1;
}

static int l_pigui_drag_int_4(lua_State *l) {
	std::string label = luaL_checkstring(l, 1);
	int v[4];
	v[0] = luaL_checkinteger(l, 2);
	v[1] = luaL_checkinteger(l, 3);
	v[2] = luaL_checkinteger(l, 4);
	v[3] = luaL_checkinteger(l, 5);
	float v_speed = luaL_checknumber(l, 6);
	int v_min = luaL_checknumber(l, 7);
	int v_max = luaL_checknumber(l, 8);
	bool res = ImGui::DragInt4(label.c_str(), v, v_speed, v_min, v_max);
	lua_pushboolean(l, res);
	lua_pushinteger(l, v[0]);
	lua_pushinteger(l, v[1]);
	lua_pushinteger(l, v[2]);
	lua_pushinteger(l, v[3]);
	return 5;
}

static int l_pigui_add_convex_poly_filled(lua_State *l) {
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	LuaTable pnts(l, 1);
	ImColor col = luaL_checkImColor(l, 2);
	bool anti_aliased = luaL_checkbool(l, 3);
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
		{ "SetNextWindowPos",       l_pigui_set_next_window_pos },
		{ "SetNextWindowSize",      l_pigui_set_next_window_size },
		{ "SetNextWindowFocus",     l_pigui_set_next_window_focus },
		{ "SetWindowFocus",         l_pigui_set_window_focus },
		{ "GetHUDMarker",           l_pigui_get_hud_marker },
		{ "GetVelocity",            l_pigui_get_velocity },
		{ "PushStyleColor",         l_pigui_push_style_color },
		{ "PopStyleColor",          l_pigui_pop_style_color },
		{ "PushStyleVar",           l_pigui_push_style_var },
		{ "PopStyleVar",            l_pigui_pop_style_var },
		{ "Columns",                l_pigui_columns },
		{ "NextColumn",             l_pigui_next_column },
		{ "Text",                   l_pigui_text },
		{ "TextWrapped",            l_pigui_text_wrapped },
		{ "Button",                 l_pigui_button },
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
		{ "PushItemWidth",          l_pigui_push_item_width },
		{ "PopItemWidth",           l_pigui_pop_item_width },
		{ "BeginPopup",             l_pigui_begin_popup },
		{ "EndPopup",               l_pigui_end_popup },
		{ "OpenPopup",              l_pigui_open_popup },
		{ "IsMouseReleased",        l_pigui_is_mouse_released },
		{ "IsMouseClicked",         l_pigui_is_mouse_clicked },
		{ "IsMouseHoveringRect",    l_pigui_is_mouse_hovering_rect },
		{ "IsMouseHoveringAnyWindow",    l_pigui_is_mouse_hovering_any_window },
		{ "RadialMenu",             l_pigui_radial_menu },
		{ "CircularSlider",         l_pigui_circular_slider },
		{ "GetMouseClickedPos",     l_pigui_get_mouse_clicked_pos },
		{ "AddConvexPolyFilled",    l_pigui_add_convex_poly_filled },
		{ "IsKeyReleased",          l_pigui_is_key_released },
		{ "DragInt4",               l_pigui_drag_int_4 },
		{ "GetWindowPos",           l_pigui_get_window_pos },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "handlers", l_attr_handlers },
		{ "screen_width", l_attr_screen_width },
		{ "screen_height", l_attr_screen_height },
		{ "key_ctrl",      l_attr_key_ctrl },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, l_methods, l_attrs, 0);
}
