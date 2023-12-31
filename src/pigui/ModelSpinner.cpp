// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "pigui/ModelSpinner.h"
#include "AnimationCurves.h"
#include "Pi.h"
#include "PiGui.h"
#include "graphics/Graphics.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "scenegraph/Tag.h"

#include <algorithm>

using namespace PiGui;

ModelSpinner::ModelSpinner() :
	m_spinning(true),
	m_pauseTime(.0f),
	m_rot(vector2f(DEG2RAD(-15.0), DEG2RAD(120.0))),
	m_zoom(1.0f),
	m_zoomTo(1.0f)
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
	if (m_resolveTarget)
		m_resolveTarget.reset();

	Graphics::RenderTargetDesc rtDesc{
		uint16_t(m_size.x), uint16_t(m_size.y),
		Graphics::TextureFormat::TEXTURE_RGBA_8888,
		Graphics::TextureFormat::TEXTURE_DEPTH, true,
		uint16_t(Pi::GetApp()->GetGraphicsSettings().requestedSamples)
	};

	m_renderTarget.reset(Pi::renderer->CreateRenderTarget(rtDesc));
	if (!m_renderTarget) Error("Error creating render target for model viewer.");


	Graphics::RenderTargetDesc resolveDesc{
		uint16_t(m_size.x), uint16_t(m_size.y),
		Graphics::TextureFormat::TEXTURE_RGBA_8888,
		Graphics::TextureFormat::TEXTURE_NONE, true
	};

	m_resolveTarget.reset(Pi::renderer->CreateRenderTarget(resolveDesc));
	if (!m_resolveTarget) Error("Error creating MSAA resolve render target for model viewer.");

	m_needsResize = false;
}

void ModelSpinner::SetModel(SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, unsigned int pattern)
{
	m_model.reset(model->MakeInstance());
	skin.Apply(m_model.get());
	m_model->SetPattern(pattern);
	m_shields.reset(new Shields(model));
	// m_model->SetDebugFlags(SceneGraph::Model::DEBUG_BBOX);
}

constexpr float SPINNER_FOV = 45.f;
void ModelSpinner::Render()
{
	PROFILE_SCOPED()
	// Resizing a render target involves destroying the old one and creating a new one.
	if (m_needsResize) CreateRenderTarget();
	if (!m_renderTarget) return;
	if (!m_model) return;

	Graphics::Renderer *r = Pi::renderer;
	Graphics::Renderer::StateTicket ticket(r);

	const auto &desc = m_renderTarget.get()->GetDesc();
	Graphics::ViewportExtents extents = { 0, 0, desc.width, desc.height };

	r->SetRenderTarget(m_renderTarget.get());
	r->SetViewport(extents);

	float lightIntensity[4] = { 0.75f, 0.f, 0.f, 0.f };
	r->SetLightIntensity(4, lightIntensity);
	r->SetAmbientColor(Color(64, 64, 64));

	r->ClearScreen(Color(0, 0, 0, 0));

	r->SetProjection(matrix4x4f::PerspectiveMatrix(DEG2RAD(SPINNER_FOV), m_size.x / m_size.y, 1.f, 10000.f, true));
	r->SetTransform(matrix4x4f::Identity());

	r->SetLights(1, &m_light);
	AnimationCurves::Approach(m_zoom, m_zoomTo, Pi::GetFrameTime(), 5.0f, 0.4f);
	m_model->Render(MakeModelViewMat());

	r->ResolveRenderTarget(m_renderTarget.get(), m_resolveTarget.get(), extents);
}

void ModelSpinner::SetSize(vector2d size)
{
	vector2f new_size = static_cast<vector2f>(size);
	if (new_size == m_size) return;

	m_size = new_size;
	m_needsResize = true;
}

matrix4x4f ModelSpinner::MakeModelViewMat()
{
	const float dist = m_model->GetDrawClipRadius() / sinf(DEG2RAD(SPINNER_FOV * 0.5f));

	matrix4x4f rot = matrix4x4f::Identity();
	rot.Translate(vector3f(0, 0, -dist * m_zoom));
	rot.RotateX(m_rot.x);
	rot.RotateY(m_rot.y);

	return rot;
}

void ModelSpinner::DrawPiGui()
{
	ImVec2 size(m_size.x, m_size.y);

	if (m_renderTarget) {
		// Draw the image and stretch it over the available region.
		// ImGui inverts the vertical axis to get top-left coordinates, so we need to invert our UVs to match.
		ImGui::Image(m_resolveTarget->GetColorTexture(), size, ImVec2(0, 1), ImVec2(1, 0));
	} else {
		ImGui::Dummy(size);
	}

	const ImGuiIO &io = ImGui::GetIO();
	bool hovered = ImGui::IsItemHovered();
	if (hovered && ImGui::IsMouseDown(0)) {
		m_rot.x -= 0.005 * io.MouseDelta.y;
		m_rot.y -= 0.005 * io.MouseDelta.x;
		m_pauseTime = 1.0f;
	} else if (m_pauseTime > 0.0) {
		m_pauseTime = std::max(0.0f, m_pauseTime - io.DeltaTime);
	} else if (m_spinning) {
		m_rot.y += io.DeltaTime;
	}

	if (hovered && io.MouseWheel != 0) {
		m_zoomTo = Clamp(m_zoomTo - io.MouseWheel * 0.08, 0.5, 1.0);
	}
}

vector3f ModelSpinner::ModelSpaceToScreenSpace(vector3f modelSpaceVec)
{
	matrix4x4f projection = matrix4x4f::PerspectiveMatrix(DEG2RAD(SPINNER_FOV), m_size.x / m_size.y, 1.f, 10000.f, true);
	matrix4x4f modelView = MakeModelViewMat();
	Graphics::ViewportExtents vp = { 0, 0, int32_t(m_size.x), int32_t(m_size.y) };

	return Graphics::ProjectToScreen(modelView * modelSpaceVec, projection, vp);
}

vector2d ModelSpinner::GetTagPos(const char *tagName)
{
	if (!m_model)
		return vector2d(0);

	std::string tName = tagName;
	const SceneGraph::Tag *tagNode = m_model->FindTagByName(tName);

	if (!tagNode)
		return vector2d(0);

	vector3f screenSpace = ModelSpaceToScreenSpace(tagNode->GetGlobalTransform().GetTranslate());
	return { screenSpace.x, m_size.y - screenSpace.y };
}
