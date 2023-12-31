// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditorModals.h"
#include "SystemEditor.h"

#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "editor/EditorIcons.h"
#include "editor/Modal.h"

#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "pigui/PiGui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

FileActionOpenModal::FileActionOpenModal(EditorApp *app) :
	Modal(app, "File Window Open", false)
{
}

void FileActionOpenModal::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 30));

	Modal::Draw();

	ImGui::PopStyleVar();
}

void FileActionOpenModal::DrawInternal()
{
	ImGui::TextUnformatted("Waiting on a file action to complete...");
}

// =============================================================================

UnsavedFileModal::UnsavedFileModal(EditorApp *app) :
	Modal(app, "Unsaved Changes", true)
{
}

void UnsavedFileModal::DrawInternal()
{
	ImGui::TextUnformatted("The current file has unsaved changes.");
	ImGui::Spacing();
	ImGui::TextUnformatted("Do you want to save the current file before proceeding?");

	ImGui::NewLine();

	float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

	if (ImGui::Button("No", ImVec2(width, 0))) {
		m_result = Result_No;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Yes", ImVec2(width, 0))) {
		m_result = Result_Yes;
		ImGui::CloseCurrentPopup();
	}
}

// =============================================================================

NewSystemModal::NewSystemModal(EditorApp *app, SystemEditor *editor, SystemPath *path) :
	Modal(app, "New System from Galaxy", true),
	m_editor(editor),
	m_path(path)
{
}

void NewSystemModal::Draw()
{
	ImVec2 vpSize = ImGui::GetMainViewport()->Size;
	ImVec2 windSize = ImVec2(vpSize.x * 0.5, vpSize.y * 0.333);
	ImVec2 windSizeMax = ImVec2(vpSize.x * 0.5, vpSize.y * 0.5);
	ImGui::SetNextWindowSizeConstraints(windSize, windSizeMax);

	Modal::Draw();
}

void NewSystemModal::DrawInternal()
{
	if (Draw::LayoutHorizontal("Sector", 3, ImGui::GetFontSize())) {
		bool changed = false;
		changed |= ImGui::InputInt("X", &m_path->sectorX, 1, 0);
		changed |= ImGui::InputInt("Y", &m_path->sectorY, 1, 0);
		changed |= ImGui::InputInt("Z", &m_path->sectorZ, 1, 0);

		if (changed)
			m_path->systemIndex = 0;

		Draw::EndLayout();
	}

	ImGui::Separator();

	RefCountedPtr<const Sector> sec = m_editor->GetGalaxy()->GetSector(m_path->SectorOnly());

	ImGui::BeginGroup();
	if (ImGui::BeginChild("Systems", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, -ImGui::GetFrameHeightWithSpacing()))) {

		for (const Sector::System &system : sec->m_systems) {
			std::string label = fmt::format("{} ({}x{})", system.GetName(), EICON_SUN, system.GetNumStars());

			if (ImGui::Selectable(label.c_str(), system.idx == m_path->systemIndex))
				m_path->systemIndex = system.idx;

			if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
				m_editor->LoadSystem(system.GetPath());
				ImGui::CloseCurrentPopup();
			}

		}

	}
	ImGui::EndChild();

	if (ImGui::Button("New System")) {
		m_editor->NewSystem(m_path->SectorOnly());
		ImGui::CloseCurrentPopup();
	}

	ImGui::SetItemTooltip("Create a new empty system in this sector.");

	ImGui::SameLine();

	if (ImGui::Button("Edit Selected")) {
		m_editor->LoadSystem(m_path->SystemOnly());
		ImGui::CloseCurrentPopup();
	}

	ImGui::SetItemTooltip("Load the selected system as a template.");

	ImGui::EndGroup();

	ImGui::SameLine();
	ImGui::BeginGroup();

	if (m_path->systemIndex < sec->m_systems.size()) {
		const Sector::System &system = sec->m_systems[m_path->systemIndex];

		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(system.GetName().c_str());

		ImGui::PopFont();

		ImGui::Spacing();

		ImGui::TextUnformatted("Is Custom:");
		ImGui::SameLine(ImGui::CalcItemWidth());
		ImGui::TextUnformatted(system.GetCustomSystem() ? "yes" : "no");

		ImGui::TextUnformatted("Is Explored:");
		ImGui::SameLine(ImGui::CalcItemWidth());
		ImGui::TextUnformatted(system.GetExplored() == StarSystem::eEXPLORED_AT_START ? "yes" : "no");

		ImGui::TextUnformatted("Faction:");
		ImGui::SameLine(ImGui::CalcItemWidth());
		ImGui::TextUnformatted(system.GetFaction() ? system.GetFaction()->name.c_str() : "<none>");

		ImGui::TextUnformatted("Other Names:");
		ImGui::SameLine(ImGui::CalcItemWidth());

		ImGui::BeginGroup();
		for (auto &name : system.GetOtherNames())
			ImGui::TextUnformatted(name.c_str());
		ImGui::EndGroup();
	}
	ImGui::EndGroup();
}
