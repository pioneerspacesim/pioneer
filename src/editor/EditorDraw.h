// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include <cstdint>

namespace Editor::Draw {
	enum RectSide : uint8_t {
		RectSide_Left = 0,
		RectSide_Top = 1,
		RectSide_Right = 2,
		RectSide_Bottom = 3
	};

	enum RectAxis : uint8_t {
		RectAxis_Horizontal = 0,
		RectAxis_Vertical = 1
	};

	enum RectAlign : uint8_t {
		RectAlign_Start = 0,
		RectAlign_End = 2,
		RectAlign_Center = 4
	};

	// Subtract a section from a side of the original rect and return it as a new rect
	ImRect RectCut(ImRect &orig, float amount, RectSide side);

	bool BeginWindow(ImRect rect, const char *name, bool *p_open = NULL, ImGuiWindowFlags flags = 0);

}
