// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "fixed.h"
#include "libs.h"

namespace ImGui {
	inline bool InputFixed(const char *str, fixed *val, double step = 0.01, double step_fast = 0.1, const char *format = "%.4f", ImGuiInputTextFlags flags = 0)
	{
		double val_d = val->ToDouble();
		bool changed = ImGui::InputDouble(str, &val_d, step, step_fast, format, flags | ImGuiInputTextFlags_EnterReturnsTrue);
		if (changed)
			*val = fixed::FromDouble(val_d);

		return changed;
	}

	inline bool InputFixedAU(const char *str, fixed *val, double step = 0.01, double step_fast = 0.1)
	{
		return InputFixed(str, val, step, step_fast, "%.4f AU");
	}

	inline bool InputFixedDegrees(const char *str, fixed *val, double step = 1.0, double step_fast = 10.0, const char *format = "%.3f°", ImGuiInputTextFlags flags = 0)
	{
		double val_d = RAD2DEG(val->ToDouble());
		bool changed = ImGui::InputDouble(str, &val_d, step, step_fast, format, flags | ImGuiInputTextFlags_EnterReturnsTrue);
		if (changed)
			*val = fixed::FromDouble(DEG2RAD(val_d));

		return changed;
	}
};
