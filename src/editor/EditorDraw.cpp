// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorDraw.h"

#include "Color.h"
#include "EnumStrings.h"

#include "UndoStepType.h"
#include "editor/EditorIcons.h"
#include "editor/UndoSystem.h"

#include "imgui/imgui.h"

#include "fmt/format.h"

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

bool Draw::BeginHostWindow(const char *label, bool *open, ImGuiWindowFlags flags)
{
	ImGuiViewport *vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(vp->Size, ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	bool shouldSubmit = ImGui::Begin(label, open, flags);

	ImGui::PopStyleVar(3);

	return shouldSubmit;
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

void Draw::BeginHorizontalBar()
{
	ImGui::BeginGroup();
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;
}

void Draw::EndHorizontalBar()
{
	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Vertical;
	ImGui::EndGroup();
}

void Draw::ShowUndoDebugWindow(UndoSystem *undo, bool *p_open)
{
	if (!ImGui::Begin("Undo Stack", p_open, 0)) {
		ImGui::End();
		return;
	}

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Undo Depth: %ld", undo->GetEntryDepth());

	if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && undo->GetEntryDepth()) {
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x * 2.f, 0.f);

		// Get out of jail free card to fix a broken undo state
		if (ImGui::Button("X")) {
			undo->ResetEntry();
			while (undo->GetEntryDepth() > 0)
				undo->EndEntry();
		}
	}

	ImGui::Separator();

	size_t numEntries = undo->GetNumEntries();
	size_t currentIdx = undo->GetCurrentEntry();
	size_t selectedIdx = currentIdx;

	if (ImGui::Selectable("<Initial State>", currentIdx == 0))
		selectedIdx = 0;

	for (size_t idx = 0; idx < numEntries; idx++)
	{
		const UndoEntry *entry = undo->GetEntry(idx);

		bool isSelected = currentIdx == idx + 1;
		std::string label = fmt::format("{}##{}", entry->GetName(), idx);

		if (ImGui::Selectable(label.c_str(), isSelected))
			selectedIdx = idx + 1;
	}

	ImGui::End();

	// If we selected an earlier history entry, undo to that point
	for (; currentIdx > selectedIdx; --currentIdx)
		undo->Undo();

	// If we selected a later history entry, redo to that point
	for (; currentIdx < selectedIdx; ++currentIdx)
		undo->Redo();
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

void Draw::EditEnum(std::string_view label, const char *name, const char *ns, int *val, size_t val_max, UndoSystem *undo)
{
	size_t selected = size_t(*val);
	const char *preview = EnumStrings::GetString(ns, selected);
	if (!preview)
		preview = "<invalid>";

	if (ComboUndoHelper(label, name, preview, undo)) {
		if (ImGui::IsWindowAppearing())
			AddUndoSingleValue(undo, val);

		for (size_t idx = 0; idx <= val_max; ++idx) {
			const char *name = EnumStrings::GetString(ns, idx);
			if (name && ImGui::Selectable(name, selected == idx))
				*val = int(idx);
		}

		ImGui::EndCombo();
	}
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

bool Draw::ColorEdit3(const char *label, Color *color)
{
	Color4f _c = color->ToColor4f();
	bool changed = ImGui::ColorEdit3(label, &_c[0]);
	*color = Color(_c);
	return changed;
}

Draw::DragDropTarget Draw::HierarchyDragDrop(const char *type, ImGuiID targetID, void *data, void *outData, size_t dataSize)
{
	ImGuiContext &g = *ImGui::GetCurrentContext();

	ImU32 col_highlight = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	ImU32 col_trans = ImGui::GetColorU32(ImGuiCol_ButtonHovered, 0.f);

	Draw::DragDropTarget ret = DragDropTarget::DROP_NONE;

	ImGui::PushID(targetID);

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload(type, data, dataSize);
		ImGui::EndDragDropSource();
	}

	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	float halfHeight = ImGui::GetItemRectSize().y * 0.4f;
	float text_offset = g.FontSize + ImGui::GetStyle().FramePadding.x * 2.f;
	float inner_x = ImGui::GetCursorScreenPos().x + text_offset;

	ImGuiID beforeTarget = ImGui::GetID("##drop_before");
	ImGuiID afterTarget = ImGui::GetID("##drop_after");
	ImGuiID innerTarget = ImGui::GetID("##drop-in");

	ImRect beforeRect(min.x, min.y, max.x, min.y + halfHeight);
	ImRect afterRect(min.x, max.y - halfHeight, max.x, max.y);
	ImRect innerRect(inner_x, min.y, max.x, max.y);

	if (ImGui::BeginDragDropTargetCustom(beforeRect, beforeTarget)) {
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		if (payload && payload->Preview) {
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(beforeRect.Min, beforeRect.Max, col_highlight, col_highlight, col_trans, col_trans);
		}

		if (payload && payload->Delivery) {
			assert(size_t(payload->DataSize) == dataSize);
			memcpy(outData, payload->Data, payload->DataSize);

			ret = DragDropTarget::DROP_BEFORE;
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginDragDropTargetCustom(afterRect, afterTarget)) {
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		if (payload && payload->Preview) {
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(afterRect.Min, afterRect.Max, col_trans, col_trans, col_highlight, col_highlight);
		}

		if (payload && payload->Delivery) {
			assert(size_t(payload->DataSize) == dataSize);
			memcpy(outData, payload->Data, payload->DataSize);

			ret = DragDropTarget::DROP_AFTER;
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginDragDropTargetCustom(innerRect, innerTarget)) {
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		if (payload && payload->Preview) {
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(innerRect.Min, innerRect.Max, col_trans, col_highlight, col_highlight, col_trans);
		}

		if (payload && payload->Delivery) {
			assert(size_t(payload->DataSize) == dataSize);
			memcpy(outData, payload->Data, payload->DataSize);

			ret = DragDropTarget::DROP_CHILD;
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::PopID();
	return ret;
}

void Draw::HelpMarker(const char* desc, bool same_line)
{
	if (same_line)
		ImGui::SameLine(ImGui::GetContentRegionAvail().x /*- ImGui::GetFontSize()*/);

    ImGui::TextDisabled(EICON_INFO);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
