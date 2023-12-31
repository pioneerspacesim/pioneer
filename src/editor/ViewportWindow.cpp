// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ViewportWindow.h"

#include "editor/EditorApp.h"
#include "graphics/Graphics.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

ViewportWindow::ViewportWindow(EditorApp *app) :
	EditorWindow(app),
	m_viewportExtents(0, 0, 0, 0),
	m_viewportActive(false),
	m_viewportHovered(false)
{}

ViewportWindow::~ViewportWindow()
{}

void ViewportWindow::OnAppearing()
{

}

void ViewportWindow::OnDisappearing()
{

}

ImGuiWindowFlags ViewportWindow::SetupWindowFlags()
{
	return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}

void ViewportWindow::Update(float deltaTime)
{
	ImGuiWindowFlags flags = SetupWindowFlags();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.f, 1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	bool shouldClose = false;
	bool open = ImGui::Begin(GetWindowName(), (flags & ImGuiWindowFlags_NoTitleBar) ? nullptr : &shouldClose, flags);

	ImGui::PopStyleVar(2);

	if (open) {
		// Perform scene updates before rendering
		OnUpdate(deltaTime);

		ImVec2 size = ImFloor(ImGui::GetContentRegionAvail());

		m_viewportExtents.w = int(size.x);
		m_viewportExtents.h = int(size.y);

		CreateRenderTarget();

		if (m_renderTarget) {
			ImVec2 screenPos = ImGui::GetCursorScreenPos();

			// Draw our render target as the window "background"
			// PiGui rendering happens after the contents of the texture are rendered
			ImGui::GetWindowDrawList()->AddImageRounded(
				ImTextureID(m_resolveTarget->GetColorTexture()),
				screenPos, screenPos + size,
				ImVec2(0, 1), ImVec2(1, 0),
				IM_COL32_WHITE,
				ImGui::GetStyle().WindowRounding,
				ImDrawFlags_RoundCornersBottomRight);

			Graphics::Renderer *r = GetApp()->GetRenderer();

			{
				Graphics::Renderer::StateTicket st(r);

				r->SetRenderTarget(m_renderTarget.get());
				r->SetViewport(m_viewportExtents);
				r->ClearScreen(Color::BLACK);

				OnRender(r);

				r->ResolveRenderTarget(m_renderTarget.get(), m_resolveTarget.get(), m_viewportExtents);
			}

			ImGuiID viewportID = ImGui::GetID("##ViewportOverlay");
			ImGui::KeepAliveID(viewportID);

			// Update mouse down state, etc; handle active layout area interaction

			ImGuiButtonFlags flags =
				ImGuiButtonFlags_FlattenChildren |
				ImGuiButtonFlags_PressedOnClick |
				ImGuiButtonFlags_AllowOverlap |
				ImGuiButtonFlags_NoSetKeyOwner |
				ImGuiButtonFlags_MouseButtonMask_;

			ImRect area = { screenPos, screenPos + size };

			bool wasPressed = m_viewportActive;
			bool clicked = ImGui::ButtonBehavior(area, viewportID,
				&m_viewportHovered, &m_viewportActive, flags);

			if (m_viewportActive)
				ImGui::GetCurrentContext()->ActiveIdAllowOverlap = true;

			// if the viewport is hovered/active or we just released it,
			// update mouse interactions with it
			if (m_viewportHovered || m_viewportActive || wasPressed) {
				// restrict the mouse pos within the viewport and convert to viewport-relative coords
				ImVec2 mousePos = ImClamp(ImGui::GetIO().MousePos, area.Min, area.Max) - area.Min;

				// disable mouse/keyboard capture so input subsystem can be used in viewport
				ImGui::CaptureMouseFromApp(false);
				ImGui::CaptureKeyboardFromApp(false);

				OnHandleInput(clicked, wasPressed && !m_viewportActive, mousePos);
			}

			ImGui::BeginChild("##ViewportContainer", ImVec2(0, 0), false,
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse |
				ImGuiWindowFlags_AlwaysUseWindowPadding);

			// set Horizontal layout type since we're using this window effectively as a toolbar
			ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;

			// Draw all "viewport overlay" UI here, to properly route inputs
			OnDraw();

			ImGui::EndChild();

			if (m_viewportActive)
				ImGui::GetCurrentContext()->ActiveIdAllowOverlap = false;
		}
	}

	ImGui::End();

	if (shouldClose && OnCloseRequested()) {
		OnDisappearing();

		// TODO: close window + inform editor of such
	}
}

void ViewportWindow::CreateRenderTarget()
{
	bool isValid = m_renderTarget && m_resolveTarget &&
		m_renderTarget->GetDesc().width == m_viewportExtents.w &&
		m_renderTarget->GetDesc().height == m_viewportExtents.h;

	if (isValid) {
		return;
	}

	Graphics::RenderTargetDesc rtDesc = Graphics::RenderTargetDesc(
		m_viewportExtents.w, m_viewportExtents.h,
		Graphics::TextureFormat::TEXTURE_RGB_888,
		Graphics::TextureFormat::TEXTURE_DEPTH, false,
		GetApp()->GetGraphicsSettings().requestedSamples
	);

	m_renderTarget.reset(GetApp()->GetRenderer()->CreateRenderTarget(rtDesc));

	Graphics::RenderTargetDesc resolveDesc = Graphics::RenderTargetDesc(
		m_viewportExtents.w, m_viewportExtents.h,
		Graphics::TextureFormat::TEXTURE_RGB_888,
		Graphics::TextureFormat::TEXTURE_NONE, true
	);

	m_resolveTarget.reset(GetApp()->GetRenderer()->CreateRenderTarget(resolveDesc));
}
