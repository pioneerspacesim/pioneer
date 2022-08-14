// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorDraw.h"
#include "imgui/imgui.h"

using namespace Editor;

ImRect Draw::RectCut(ImRect &orig, float amount, RectSide side)
{
	ImRect out = orig;

	if (side == RectSide_Left) {
		out.Max.x = orig.Min.x = std::min(orig.Min.x + amount, orig.Max.x);
	} else if (side == RectSide_Top) {
		out.Max.y = orig.Min.y = std::min(orig.Min.y + amount, orig.Max.y);
	} else if (side == RectSide_Right) {
		out.Min.x = orig.Max.x = std::max(orig.Max.x - amount, orig.Min.x);
	} else if (side == RectSide_Bottom) {
		out.Min.y = orig.Max.y = std::max(orig.Max.y - amount, orig.Min.y);
	}

	return out;
}

bool Draw::BeginWindow(ImRect rect, const char *label, bool *open, ImGuiWindowFlags flags)
{
	ImGui::SetNextWindowPos(rect.Min, ImGuiCond_Always);
	ImGui::SetNextWindowSize(rect.Max - rect.Min, ImGuiCond_Always);

	flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

	return ImGui::Begin(label, open, flags);
}
