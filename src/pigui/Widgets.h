// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <imgui/imgui.h>
#include "vector3.h"

#include <vector>

namespace PiGui::Draw {

	int RadialPopupSelectMenu(const ImVec2 center, const char *popup_id, int mouse_button, const std::vector<ImTextureID> &tex_ids, const std::vector<std::pair<ImVec2, ImVec2>> &uvs, const std::vector<ImU32> &colors, const std::vector<const char *> &tooltips, unsigned int size, unsigned int padding);
	bool CircularSlider(const ImVec2 &center, float *v, float v_min, float v_max);

	bool LowThrustButton(const char *label, const ImVec2 &size_arg, int thrust_level, const ImVec4 &bg_col, int frame_padding, ImColor gauge_fg, ImColor gauge_bg);
	bool ButtonImageSized(ImTextureID user_texture_id, const ImVec2 &size, const ImVec2 &imgSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col);

	void ThrustIndicator(ImGuiID id, float diameter, const vector3d &thrust);

	// Draw a generic counter-clockwise circular indicator with a value dial going from 0..1 and a limit dial going from 1..0.
	// The phase parameter controls where on the circle value and value_inv start at.
	void CircleIndicator(ImGuiID id, float diameter, const char *label, const char *unit, float value, float value_inv, float phase);

	bool GlyphButton(const char *id_str, const char *glyph, const ImVec2 &size, ImGuiButtonFlags flags);

	// we need to know if the change was made by direct input or the change was
	// made by mouse movement, to successfully serve values such as YYYY-MM-DD
	// when changing with the mouse, we get some internal delta value and add
	// it to previous internal value
	// when we receive direct input from the keyboard, we get a completely
	// different (face value), an integer of the form YYYY-MM-DD so we just
	// convert it to internal value.
	// Also see: data/pigui/modules/new-game-window/location.lua
	enum class DragChangeMode : unsigned { NOT_CHANGED, CHANGED, CHANGED_BY_TYPING };
	DragChangeMode IncrementDrag(const char *label, double &v, float v_speed, const double v_min, const double v_max, const char *format, bool draw_progress_bar);

	// Begin a horizontal layout block. Internally, this creates a new group.
	void BeginHorizontalGroup();
	// End a horizontal layout block. Internally, this closes the group.
	void EndHorizontalGroup();

} // namespace PiGui::Draw
