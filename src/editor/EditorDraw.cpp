// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorDraw.h"
#include "UndoSystem.h"

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

bool Draw::EditFloat2(const char *label, ImVec2 *vec, float step, float step_fast, const char *format)
{
	bool changed = false;
	if (Draw::LayoutHorizontal(label, 2, ImGui::GetFontSize())) {
		changed |= ImGui::InputFloat("X", &vec->x, step, step_fast, format);
		changed |= ImGui::InputFloat("Y", &vec->y, step, step_fast, format);

		Draw::EndLayout();
	}

	return changed;
}

bool Draw::LayoutHorizontal(const char *label, int numItems, float itemLabelSize)
{
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	float width = ImGui::GetContentRegionAvail().x;

	ImGui::TextUnformatted(label);
	ImGui::BeginGroup();

	ImGui::PushID(label);
	ImGui::PushItemWidth((width / numItems) - itemLabelSize - spacing);
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;

	return true;
}

void Draw::EndLayout()
{
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Vertical;
	ImGui::PopItemWidth();
	ImGui::PopID();

	ImGui::EndGroup();
	ImGui::Spacing();
}

bool Draw::UndoHelper(std::string_view label, UndoSystem *undo)
{
	if (ImGui::IsItemDeactivated()) {
		undo->EndEntry();
		// Log::Info("Ending entry {}\n", label);
	}

	if (ImGui::IsItemActivated()) {
		undo->BeginEntry(label);
		// Log::Info("Beginning entry {}\n", label);
		return true;
	}

	return false;
}

bool Draw::ComboUndoHelper(std::string_view entryName, const char *label, const char *preview, UndoSystem *undo)
{
	ImGuiID id = ImGui::GetID(entryName.data());
	bool *opened = ImGui::GetStateStorage()->GetBoolRef(id);
	bool append = ImGui::BeginCombo(label, preview);

	if (ImGui::IsWindowAppearing()) {
		// Log::Info("Beginning entry {}\n", entryName);
		undo->BeginEntry(entryName);
		*opened = true;
	}

	if (*opened && !append)
	{
		*opened = false;
		undo->EndEntry();
		// Log::Info("Ending entry {}\n", entryName);
	}

	return append;
}

bool Draw::ComboUndoHelper(std::string_view label, const char *preview, UndoSystem *undo)
{
	return ComboUndoHelper(label, label.data(), preview, undo);
}

bool Draw::MenuButton(const char *label)
{
	ImVec2 screenPos = ImGui::GetCursorScreenPos();

	if (ImGui::Button(label))
		ImGui::OpenPopup(label);

	if (ImGui::IsPopupOpen(label)) {
		ImGuiPopupFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus;
		ImGui::SetNextWindowPos(screenPos + ImVec2(0.f, ImGui::GetFrameHeightWithSpacing()));

		return ImGui::BeginPopup(label, flags);
	}

	return false;
}

bool Draw::ToggleButton(const char *label, bool *value, ImVec4 activeColor)
{
	if (*value)
		ImGui::PushStyleColor(ImGuiCol_Button, activeColor);

	bool changed = ImGui::Button(label);

	if (*value)
		ImGui::PopStyleColor(1);

	if (changed)
		*value = !*value;

	return changed;
}
