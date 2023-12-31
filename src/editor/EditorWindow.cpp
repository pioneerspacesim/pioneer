// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorWindow.h"

#include "editor/EditorApp.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

EditorWindow::EditorWindow(EditorApp *app) :
	m_app(app)
{}

EditorWindow::~EditorWindow()
{}

void EditorWindow::Update(float deltaTime)
{
	ImGuiWindowFlags flags = {};

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.f, 1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	bool shouldClose = false;
	bool open = ImGui::Begin(GetWindowName(), &shouldClose, flags);

	ImGui::PopStyleVar(2);

	if (open) {
		OnDraw();
	}

	ImGui::End();

	if (shouldClose && OnCloseRequested()) {
		OnDisappearing();

		// TODO: close window + inform editor of such
	}
}
