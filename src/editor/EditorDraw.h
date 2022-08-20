// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "FloatComparison.h"

#include <string_view>

namespace Editor {
	class UndoSystem;
}

namespace Editor::Draw {

	enum RectSide : uint8_t {
		RectSide_Left = 0,
		RectSide_Top = 1,
		RectSide_Right = 2,
		RectSide_Bottom = 3
	};

	// Subtract a section from a side of the original rect and return it as a new rect
	ImRect RectCut(ImRect &orig, float amount, RectSide side);

	// Set the next window size to the given rect and begin it
	bool BeginWindow(ImRect rect, const char *name, bool *p_open = NULL, ImGuiWindowFlags flags = 0);

	// Draw an edit box for an ImVec2 with the given settings
	bool EditFloat2(const char *label, ImVec2 *vec, float step = 0.f, float step_fast = 0.f, const char *format = "%.3f");

	// Begin a horizontal layout for N inputs/items, reserving a given amount of size for each one's label
	bool LayoutHorizontal(const char *label, int nItems, float itemLabelSize);

	// End a horizontal layout block
	void EndLayout();

	// Manage pushing/popping an UndoEntry after an input widget that provides IsItemActivated() and IsItemDeactivated()
	// Note: this helper relies on the widget *not* changing the underlying value the frame IsItemActivated() is true
	bool UndoHelper(std::string_view label, UndoSystem *undo);

	// Open a Combo and manage pushing/popping an UndoEntry. Returns true if the Combo window is open.
	// Use ImGui::IsWindowAppearing() to determine if you should push an UndoStep.
	bool ComboUndoHelper(std::string_view entryName, const char *label, const char *preview, UndoSystem *undo);

	// The above, but defaulting the label to the entryName
	bool ComboUndoHelper(std::string_view label, const char *preview, UndoSystem *undo);

}

inline bool operator==(const ImVec2 &a, const ImVec2 &b)
{
	return is_equal_general(a.x, b.x) && is_equal_general(a.y, b.y);
}
