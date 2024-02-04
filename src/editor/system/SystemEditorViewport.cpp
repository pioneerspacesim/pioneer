// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

	m_map->SetShowGravpoints(true);

	m_map->svColor[SystemMapViewport::GRID] = Color(0x24242BFF);
	m_map->svColor[SystemMapViewport::GRID_LEG] = Color(0x787878FF);
	m_map->svColor[SystemMapViewport::SYSTEMBODY] = Color(0xB5BCE3FF).Shade(0.5);
	m_map->svColor[SystemMapViewport::SYSTEMBODY_ORBIT] = Color(0x5ACC0AFF);

	m_background.reset(new Background::Container(m_app->GetRenderer(), m_editor->GetRng()));
	m_background->SetDrawFlags(Background::Container::DRAW_SKYBOX);
	m_map->SetBackground(m_background.get());
}

SystemEditorViewport::~SystemEditorViewport()
{
}

void SystemEditorViewport::SetSystem(RefCountedPtr<StarSystem> system)
{
	m_map->SetReferenceTime(0.0); // Jan 1 3200
	m_map->SetCurrentSystem(system);
}

bool SystemEditorViewport::OnCloseRequested()
{
	return false;
}

void SystemEditorViewport::OnUpdate(float deltaTime)
{
	m_map->Update(deltaTime);

	m_map->AddObjectTrack({ Projectable::types(Projectable::_MAX + 1), Projectable::BODY, static_cast<Body *>(nullptr), vector3d(1e12, 0.0, 0.0) });
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

	// Draw background
	ImVec2 toolbar_bg_size = ImVec2(ImGui::GetWindowSize().x, ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y * 2.f);
	ImColor toolbar_bg_color = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];

	ImGui::GetWindowDrawList()->PushClipRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetWindowPos(), ImGui::GetWindowPos() + toolbar_bg_size,
		toolbar_bg_color);
	ImGui::GetWindowDrawList()->PopClipRect();

	DrawTimelineControls();

	// Render all GUI elements over top of viewport overlays
	// (Items rendered earlier take priority when using IsItemHovered)

	split.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);

	const auto &projected = m_map->GetProjected();
	std::vector<Projectable::GroupInfo> groups = m_map->GroupProjectables({ 10.f, 10.f }, {});

	// Depth sort groups (further groups drawn first / under closer groups)
	std::sort(groups.begin(), groups.end(), [](const auto &a, const auto &b){ return a.screenpos.z > b.screenpos.z; });

	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	ImRect screen_rect = ImRect(ImVec2(0, 0), ImGui::GetWindowSize());

	// Then draw "under" the GUI elements so we can use ImGui::IsItemHovered et al.
	for (auto &group : groups) {
		ImVec2 itempos = { group.screenpos.x, group.screenpos.y };

		// Simple screen clipping rejection test
		if (!screen_rect.Contains(itempos))
			continue;

		const Projectable &item = projected[group.tracks[0]];
		if (group.type == Projectable::OBJECT && item.base == Projectable::SYSTEMBODY) {
			ImColor icon_col(0xFFC8C8C8);

			std::string label = group.tracks.size() > 1 ?
				fmt::format("{} ({})", item.ref.sbody->GetName(), group.tracks.size()) :
				item.ref.sbody->GetName();

			bool clicked = DrawIcon(ImGui::GetID(item.ref.sbody), itempos, icon_col, GetBodyIcon(item.ref.sbody), label.c_str());

			if (clicked) {
				m_map->SetSelectedObject(item);
				m_editor->SetSelectedBody(const_cast<SystemBody *>(item.ref.sbody));

				if (ImGui::IsMouseDoubleClicked(0)) {
					m_map->ViewSelectedObject();
				}
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", label.c_str());

			m_editor->DrawBodyContextMenu(const_cast<SystemBody *>(item.ref.sbody));

		}

		#if 0 // TODO: visual edit gizmos for body axes
		if (int(group.type) == Projectable::_MAX + 1) {
			DrawIcon(itempos, IM_COL32_WHITE, EICON_AXES);
		}
		#endif
	}

	ImGui::PopFont();

	split.Merge(ImGui::GetWindowDrawList());
}

void SystemEditorViewport::DrawTimelineControls()
{
	double timeAccel = 0.0;

	ImGui::PushFont(m_app->GetPiGui()->GetFont("icons", ImGui::GetFrameHeight()));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImGui::GetStyle().ItemInnerSpacing);

	ImGui::Button(EICON_REWIND3);
	if (ImGui::IsItemActive())
		timeAccel = -3600.0 * 24.0 * 730.0; // 2 years per second

	ImGui::Button(EICON_REWIND2);
	if (ImGui::IsItemActive())
		timeAccel = -3600.0 * 24.0 * 60.0; // 2 months per second

	ImGui::Button(EICON_REWIND1);
	if (ImGui::IsItemActive())
		timeAccel = -3600.0 * 24.0 * 5.0; // 5 days per second

	bool timeStop = ImGui::ButtonEx(EICON_TIMESTOP, ImVec2(0,0), ImGuiButtonFlags_PressedOnClick);

	ImGui::Button(EICON_FORWARD1);
	if (ImGui::IsItemActive())
		timeAccel = 3600.0 * 24.0 * 5.0; // 5 days per second

	ImGui::Button(EICON_FORWARD2);
	if (ImGui::IsItemActive())
		timeAccel = 3600.0 * 24.0 * 60.0; // 2 months per second

	ImGui::Button(EICON_FORWARD3);
	if (ImGui::IsItemActive())
		timeAccel = 3600.0 * 24.0 * 730.0; // 2 years per second

	ImGui::PopStyleVar(2);
	ImGui::PopFont();

	ImGui::AlignTextToFramePadding();

	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		timeAccel *= 0.1;

	if (!timeStop)
		m_map->AccelerateTime(timeAccel);
	else
		m_map->SetRealTime();

	ImGui::Text("%s", format_date(m_map->GetTime()).c_str());
}

bool SystemEditorViewport::DrawIcon(ImGuiID id, const ImVec2 &icon_pos, const ImColor &color, const char *icon, const char *label)
{
	ImVec2 icon_size = ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize());
	ImVec2 draw_pos = ImGui::GetWindowPos() + icon_pos - icon_size * 0.5f;

	ImGui::GetWindowDrawList()->AddText(draw_pos, color, icon);

	ImRect hover_rect = ImRect(draw_pos, draw_pos + icon_size);

	if (label) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 12));
		ImVec2 text_pos = ImGui::GetWindowPos() + icon_pos + ImVec2(icon_size.x, -ImGui::GetFontSize() * 0.5f);
		ImVec2 text_size = ImGui::CalcTextSize(label);
		// label shadow
		ImGui::GetWindowDrawList()->AddText(text_pos + ImVec2(1.f, 1.f), IM_COL32_BLACK, label);
		// body label
		ImGui::GetWindowDrawList()->AddText(text_pos, color, label);
		ImGui::PopFont();

		hover_rect.Add(text_pos + text_size);
	}

	ImGuiID prevHovered = ImGui::GetHoveredID();

	ImGuiButtonFlags flags =
		ImGuiButtonFlags_MouseButtonLeft |
		ImGuiButtonFlags_PressedOnClickRelease |
		ImGuiButtonFlags_PressedOnDoubleClick;

	ImGui::ItemAdd(hover_rect, id);

	// Allow interaction with this label
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(hover_rect, id, &hovered, &held, flags);

	// Reset hovered state so viewport ButtonBehavior receives middle-mouse clicks
	// (otherwise this label "steals" hovered state and blocks all interaction)
	// NOTE: this works with practically any button-like widget, not just ButtonBehavior
	if (ImGui::IsItemHovered())
		ImGui::SetHoveredID(prevHovered);

	return pressed;
}
