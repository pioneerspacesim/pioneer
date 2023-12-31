// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "EditorIcons.h"
#include "MathUtil.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "fixed.h"

namespace ImGui {
	inline bool InputFixed(const char *str, fixed *val, double step = 0.01, double step_fast = 0.1, const char *format = "%.4f", ImGuiInputTextFlags flags = 0)
	{
		double val_d = val->ToDouble();
		bool changed = ImGui::InputDouble(str, &val_d, step, step_fast, format, flags | ImGuiInputTextFlags_EnterReturnsTrue);
		if (changed)
			*val = fixed::FromDouble(val_d);

		return changed;
	}

	inline bool InputInt(const char* label, int* v, int step, int step_fast, const char *format, ImGuiInputTextFlags flags = 0)
	{
		return InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
	}
}

namespace Editor::Draw {

	// Subtract the currently used space on this line and apply it to the next drawn item
	void SubtractItemWidth();

	inline bool RandomButton()
	{
		ImGuiStyle &style = ImGui::GetStyle();
		bool ret = ImGui::Button(EICON_RANDOM);

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);
		Draw::SubtractItemWidth();

		return ret;
	}

	bool InputFixedSlider(const char *str, fixed *val, double val_min = 0.0, double val_max = 1.0, const char *format = "%.4f", ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);
	bool InputFixedDegrees(const char *str, fixed *val, double val_min = -360.0, double val_max = 360.0, ImGuiInputTextFlags flags = 0);
	bool InputFixedDistance(const char *str, fixed *val, ImGuiInputTextFlags flags = 0);
	bool InputFixedMass(const char *str, fixed *val, bool is_solar, ImGuiInputTextFlags flags = 0);
	bool InputFixedRadius(const char *str, fixed *val, bool is_solar, ImGuiInputTextFlags flags = 0);

};
