// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditorViewport.h"

#include "EditorIcons.h"
#include "SystemEditor.h"

#include "Background.h"
#include "SystemView.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"

#include "editor/EditorApp.h"
#include "editor/ViewportWindow.h"
#include "pigui/PiGui.h"
#include "system/SystemEditor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

SystemEditorViewport::SystemEditorViewport(EditorApp *app, SystemEditor *editor) :
	ViewportWindow(app),
	m_app(app),
	m_editor(editor)
{
	m_map.reset(new SystemMapViewport(m_app));

	m_map->svColor[SystemMapViewport::GRID] = Color(0x24242BFF);
	m_map->svColor[SystemMapViewport::GRID_LEG] = Color(0x787878FF);
	m_map->svColor[SystemMapViewport::SYSTEMBODY] = Color(0xB5BCE3FF).Shade(0.5);
	m_map->svColor[SystemMapViewport::SYSTEMBODY_ORBIT] = Color(0x5ACC0AFF);
}

SystemEditorViewport::~SystemEditorViewport()
{
}

void SystemEditorViewport::SetSystem(RefCountedPtr<StarSystem> system)
{
	m_map->SetReferenceTime(0.0); // Jan 1 3200
	m_map->SetCurrentSystem(system);

	m_background.reset(new Background::Container(m_app->GetRenderer(), m_editor->GetRng()));
	m_background->SetDrawFlags(Background::Container::DRAW_SKYBOX);
	m_map->SetBackground(m_background.get());
}

bool SystemEditorViewport::OnCloseRequested()
{
	return false;
}

void SystemEditorViewport::OnUpdate(float deltaTime)
{
	m_map->Update(deltaTime);
}

void SystemEditorViewport::OnRender(Graphics::Renderer *r)
{
	m_map->Draw3D();
}

void SystemEditorViewport::OnHandleInput(bool clicked, bool released, ImVec2 mousePos)
{
	m_map->HandleInput(m_app->DeltaTime());
}

void SystemEditorViewport::OnDraw()
{
	ImDrawListSplitter split {};

	split.Split(ImGui::GetWindowDrawList(), 2);
	split.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

	split.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);

	ImVec2 windowTL = ImGui::GetWindowPos();
	ImVec2 windowBR = ImVec2(windowTL.x + ImGui::GetWindowSize().x, windowTL.y + ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y * 2.f);
	ImGui::GetWindowDrawList()->PushClipRect(windowTL, windowBR);
	ImGui::GetWindowDrawList()->AddQuadFilled(
		windowTL, ImVec2(windowBR.x, windowTL.y),
		windowBR, ImVec2(windowTL.x, windowBR.y),
		ImColor(1.f, 1.f, 1.f, 0.25f));
	ImGui::GetWindowDrawList()->PopClipRect();

	if (ImGui::Button("This is a test!")) {}

	// Render all GUI elements over top of viewport overlays

	split.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

	const auto &projected = m_map->GetProjected();
	std::vector<Projectable::GroupInfo> groups = m_map->GroupProjectables({ 10.f, 10.f }, {});

	// Depth sort groups (further groups drawn first / under closer groups)
	std::sort(groups.begin(), groups.end(), [](const auto &a, const auto &b){ return a.screenpos.z > b.screenpos.z; });

	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	// Then draw "under" the GUI elements so we can use ImGui::IsItemHovered et al.
	for (auto &group : groups) {
		ImVec2 itempos = { group.screenpos.x, group.screenpos.y };
		ImVec2 iconSize = { ImGui::GetFontSize(), ImGui::GetFontSize() };

		const Projectable &item = projected[group.tracks[0]];
		if (group.type == Projectable::OBJECT && item.base == Projectable::SYSTEMBODY) {
			ImColor icon_col(0xFFC8C8C8);
			const char *bodyName = item.ref.sbody->GetName().c_str();

			ImVec2 drawPos = windowTL + itempos - iconSize * 0.5f;
			ImGui::GetWindowDrawList()->AddText(drawPos, icon_col, GetBodyIcon(item.ref.sbody));

			ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 12));
			ImVec2 textPos = windowTL + itempos + ImVec2(iconSize.x, -ImGui::GetFontSize() * 0.5f);
			ImVec2 textSize = ImGui::CalcTextSize(bodyName);
			// label shadow
			ImGui::GetWindowDrawList()->AddText(textPos + ImVec2(1.f, 1.f), IM_COL32_BLACK, bodyName);
			// body label
			ImGui::GetWindowDrawList()->AddText(textPos, icon_col, bodyName);
			ImGui::PopFont();

			ImRect hoverRect(drawPos, drawPos + iconSize);
			hoverRect.Add(textPos + textSize);

			if (ImGui::ItemHoverable(hoverRect, 0, 0)) {
				ImGui::SetTooltip("%s", bodyName);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					m_map->SetSelectedObject(item);
					m_editor->SetSelectedBody(const_cast<SystemBody *>(item.ref.sbody));
				}

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					m_map->ViewSelectedObject();
				}
			}
		}
	}

	ImGui::PopFont();

	split.Merge(ImGui::GetWindowDrawList());
}
