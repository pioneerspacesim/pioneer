// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "pigui/ModelSpinner.h"
#include "Pi.h"
#include "PiGui.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"

#include <algorithm>

using namespace PiGUI;

ModelSpinner::ModelSpinner() :
	m_pauseTime(.0f),
	m_rot(vector2f(DEG2RAD(-15.0), DEG2RAD(180.0)))
{
	Color lc(Color::WHITE);
	m_light.SetDiffuse(lc);
	m_light.SetSpecular(lc);
	m_light.SetPosition(vector3f(0.f, 1.f, 1.f));
	m_light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);
}

void ModelSpinner::CreateRenderTarget()
{
	if (m_renderTarget)
		m_renderTarget.reset();

	Graphics::RenderTargetDesc rtDesc{
		uint16_t(m_size.x), uint16_t(m_size.y),
		Graphics::TextureFormat::TEXTURE_RGBA_8888,
		Graphics::TextureFormat::TEXTURE_DEPTH, true
	};

	m_renderTarget.reset(Pi::renderer->CreateRenderTarget(rtDesc));
	if (!m_renderTarget) Error("Error creating render target for model viewer.");

	m_needsResize = false;
}

void ModelSpinner::SetModel(SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, unsigned int pattern)
{
	m_model.reset(model->MakeInstance());
	skin.Apply(m_model.get());
	m_model->SetPattern(pattern);
	m_shields.reset(new Shields(model));
}

void ModelSpinner::Render()
{
	PROFILE_SCOPED()
	// Resizing a render target involves destroying the old one and creating a new one.
	if (m_needsResize) CreateRenderTarget();
	if (!m_renderTarget) return;

	Graphics::Renderer *r = Pi::renderer;

	Graphics::Renderer::StateTicket ticket(r);

	r->SetRenderTarget(m_renderTarget.get());

	r->SetClearColor(Color(0, 0, 0, 0));
	r->ClearScreen();

	const float fov = 45.f;
	r->SetPerspectiveProjection(fov, m_size.x / m_size.y, 1.f, 10000.f);
	r->SetTransform(matrix4x4f::Identity());
	const auto &desc = m_renderTarget.get()->GetDesc();
	r->SetViewport(0, 0, desc.width, desc.height);

	r->SetLights(1, &m_light);

	matrix4x4f rot = matrix4x4f::RotateXMatrix(m_rot.x);
	rot.RotateY(m_rot.y);
	const float dist = m_model->GetDrawClipRadius() / sinf(DEG2RAD(fov * 0.5f));
	rot[14] = -dist;
	m_model->Render(rot);
	r->SetRenderTarget(0);
}

ImTextureID ModelSpinner::GetTextureID()
{
	// Upconvert a GLuint to uint64_t before casting to void *.
	// This is downconverted to GLuint later by ImGui_ImplOpenGL3.
	return reinterpret_cast<ImTextureID>(m_renderTarget.get()->GetColorTexture()->GetTextureID() | 0UL);
}

void ModelSpinner::SetSize(vector2d size)
{
	vector2f new_size = static_cast<vector2f>(size);
	if (new_size == m_size) return;

	m_size = new_size;
	m_needsResize = true;
}

void ModelSpinner::DrawPiGui()
{
	ImVec2 size(m_size.x, m_size.y);

	if (m_renderTarget) {
		// Draw the image and stretch it over the available region.
		// ImGui inverts the vertical axis to get top-left coordinates, so we need to invert our UVs to match.
		ImGui::Image(GetTextureID(), size, ImVec2(0, 1), ImVec2(1, 0));
	} else {
		ImGui::Dummy(size);
	}

	const ImGuiIO &io = ImGui::GetIO();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseDown(0)) {
		m_rot.x -= 0.005 * io.MouseDelta.y;
		m_rot.y -= 0.005 * io.MouseDelta.x;
		m_pauseTime = 1.0f;
	} else if (m_pauseTime > 0.0) {
		m_pauseTime = std::max(0.0f, m_pauseTime - io.DeltaTime);
	} else {
		m_rot.y += io.DeltaTime;
	}
}

vector2d ModelSpinner::ModelSpaceToScreenSpace(vector3d modelSpaceVec)
{
	return { 0.0, 0.0 };
}
