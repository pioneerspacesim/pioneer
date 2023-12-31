// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Modal.h"

#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "pigui/PiGui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

Modal::Modal(EditorApp *app, const char *title, bool canClose) :
	m_app(app),
	m_title(title),
	m_id(0),
	m_shouldClose(false),
	m_canClose(canClose)
{
}

bool Modal::Ready()
{
	return m_id != 0 && !ImGui::IsPopupOpen(m_id, ImGuiPopupFlags_AnyPopupLevel);
}

void Modal::Close()
{
	m_shouldClose = true;
}

void Modal::Draw()
{
	if (!m_id) {
		m_id = ImGui::GetID(m_title);
		ImGui::OpenPopup(m_id);
	}

	ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize * 0.5, ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	if (ImGui::BeginPopupModal(m_title, m_canClose ? &m_canClose : nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 14));

		if (m_shouldClose)
			ImGui::CloseCurrentPopup();
		else
			DrawInternal();

		ImGui::PopFont();
		ImGui::EndPopup();
	}

	ImGui::PopFont();
}
