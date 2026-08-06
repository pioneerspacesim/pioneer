// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "View.h"
#include "Pi.h"
#include "pigui/LuaPiGui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

View::View(const std::string &name) :
	m_renderer(nullptr),
	m_handlerName(name)
{
}

View::~View()
{
}

void View::Attach()
{
	// Store the fact that we were recently activated to trigger Lua UI to
	// refresh its state.
	m_activated = true;

	OnSwitchTo();
}

void View::Detach()
{
	OnSwitchFrom();
}

void View::DrawPiGui()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoCaptureMouse;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

	bool open = ImGui::Begin(m_handlerName.c_str(), nullptr, flags);
	ImGui::BringWindowToDisplayBack(ImGui::GetCurrentWindow());

	if (open) {
		DrawUIContents();
		PiGui::RunHandler(Pi::GetFrameTime(), m_handlerName, m_activated);

		m_activated = false;
	}

	ImGui::End();
}
