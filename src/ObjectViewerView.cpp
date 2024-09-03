// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ObjectViewerView.h"
#include "Frame.h"
#include "GameConfig.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Space.h"
#include "SpaceStation.h"
#include "WorldView.h"

#include "buildopts.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"
#include "imgui/imgui.h"
#include "pigui/LuaPiGui.h"
#include "profiler/Profiler.h"
#include "terrain/Terrain.h"
#include "utils.h"

#include <limits>

#if WITH_OBJECTVIEWER

ObjectViewerView::ObjectViewerView() :
	PiGuiView("ObjectViewerView"),
	m_targetBody(nullptr),
	m_systemBody(nullptr),
	m_state{}
{
	viewingDist = 1000.0f;
	m_camRot = matrix4x4d::Identity();

	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");
	m_cameraContext.Reset(new CameraContext(Pi::renderer->GetWindowWidth(), Pi::renderer->GetWindowHeight(), fovY, znear, zfar));
	m_camera.reset(new Camera(m_cameraContext, Pi::renderer));

	m_cameraContext->SetCameraFrame(Pi::player->GetFrame());
	m_cameraContext->SetCameraPosition(Pi::player->GetInterpPosition() + vector3d(0, 0, viewingDist));
	m_cameraContext->SetCameraOrient(matrix3x3d::Identity());
}

void ObjectViewerView::Draw3D()
{
	PROFILE_SCOPED()
	m_renderer->ClearScreen();
	float znear, zfar;
	m_renderer->GetNearFarRange(znear, zfar);
	m_renderer->SetPerspectiveProjection(75.f, m_renderer->GetDisplayAspect(), znear, zfar);
	m_renderer->SetTransform(matrix4x4f::Identity());

	Graphics::Light light;
	light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);

	const int btnState = Pi::input->MouseButtonState(SDL_BUTTON_RIGHT);
	if (btnState) {
		int m[2];
		Pi::input->GetMouseMotion(m);
		m_camRot = matrix4x4d::RotateXMatrix(-0.002 * m[1]) *
			matrix4x4d::RotateYMatrix(-0.002 * m[0]) * m_camRot;
		m_cameraContext->SetCameraPosition(Pi::player->GetInterpPosition() + vector3d(0, 0, viewingDist));
		m_cameraContext->BeginFrame();
		m_camera->Update();
	}

	if (m_targetBody) {
		if (m_targetBody->IsType(ObjectType::STAR))
			light.SetPosition(vector3f(0.f));
		else {
			light.SetPosition(vector3f(0.577f));
		}
		m_renderer->SetLights(1, &light);

		m_targetBody->Render(m_renderer, m_camera.get(), vector3d(0, 0, -viewingDist), m_camRot);

		// industry-standard red/green/blue XYZ axis indicator
		matrix4x4d trans = matrix4x4d::Translation(vector3d(0, 0, -viewingDist)) * m_camRot * matrix4x4d::ScaleMatrix(m_targetBody->GetClipRadius() * 2.0);
		m_renderer->SetTransform(matrix4x4f(trans));
		Graphics::Drawables::GetAxes3DDrawable(m_renderer)->Draw(m_renderer);
	}

	if (btnState) {
		m_cameraContext->EndFrame();
	}
}

void ObjectViewerView::OnSwitchTo()
{
	// rotate X is vertical
	// rotate Y is horizontal
	m_camRot = matrix4x4d::RotateXMatrix(DEG2RAD(-30.0)) * matrix4x4d::RotateYMatrix(DEG2RAD(-15.0));
}

void ObjectViewerView::ReloadState()
{
	if (m_targetBody->IsType(ObjectType::SPACESTATION)) {
		m_systemBody = static_cast<SpaceStation *>(m_targetBody)->GetSystemBody();
	} else if (m_targetBody->IsType(ObjectType::TERRAINBODY)) {
		m_systemBody = static_cast<TerrainBody *>(m_targetBody)->GetSystemBody();
		m_isTerrainBody = m_systemBody != nullptr;
	}

	if (!m_isTerrainBody)
		return;

	m_state.seed = m_systemBody->GetSeed();
	m_state.mass = m_systemBody->GetMassAsFixed().ToDouble();
	m_state.radius = m_systemBody->GetRadiusAsFixed().ToDouble();
	m_state.life = m_systemBody->GetLife();
	m_state.volatileGas = m_systemBody->GetVolatileGas();
	m_state.volatileIces = m_systemBody->GetVolatileIces();
	m_state.volatileLiquid = m_systemBody->GetVolatileLiquid();
	m_state.metallicity = m_systemBody->GetMetallicity();
	m_state.volcanicity = m_systemBody->GetVolcanicity();
}

void ObjectViewerView::Update()
{
	if (Pi::input->KeyState(SDLK_EQUALS)) viewingDist *= 0.99f;
	if (Pi::input->KeyState(SDLK_MINUS)) viewingDist *= 1.01f;
	viewingDist = Clamp(viewingDist, 10.0f, 1e12f);

	Body *body = Pi::player->GetNavTarget();
	if (body != m_targetBody) {
		m_targetBody = body;
		m_isTerrainBody = false;
		m_systemBody = nullptr;

		if (body) {
			ReloadState();

			// Reset view distance for new target.
			viewingDist = body->GetClipRadius() * 2.0f;
		}
	}
}

void ObjectViewerView::DrawInfoWindow()
{
	std::string infoLabel = fmt::format("View dist: {} Object: {}",
		format_distance(viewingDist), m_targetBody->GetLabel());

	ImVec2 vpSize = ImGui::GetMainViewport()->Size;

	float xpos = ImGui::GetStyle().WindowPadding.x * 2;
	float ypos = vpSize.y - (ImGui::GetTextLineHeightWithSpacing() * 5 + ImGui::GetFrameHeightWithSpacing());

	ImGui::SetNextWindowPos({ xpos, ypos });
	ImGui::SetNextWindowSize({ vpSize.x - xpos, vpSize.y - ypos });
	ImGui::Begin("ObjectViewerView#Info", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

	ImGui::TextUnformatted(infoLabel.c_str());

	if (m_systemBody)
		ImGui::TextUnformatted(fmt::format("SystemPath: {} {}", m_systemBody->GetName(), to_string(m_systemBody->GetPath())).c_str());

	// TODO: any more information about the current object here

	ImGui::End();
}

namespace ImGui {
	bool DragDouble(const char *label, double *v, double v_speed = 1.0f, double v_min = 0.0f, double v_max = 0.0f, const char *format = "%.3f")
	{
		return DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format);
	}
} // namespace ImGui

void ObjectViewerView::DrawControlsWindow()
{
	ImVec2 vpSize = ImGui::GetMainViewport()->Size;

	float xpos = vpSize.x - vpSize.x / 5.0;
	float ypos = ImGui::GetStyle().WindowPadding.y;

	ImGui::SetNextWindowPos({ xpos, ypos });
	ImGui::SetNextWindowSize({ vpSize.x - xpos, vpSize.y - ypos });
	ImGui::Begin("ObjectViewerView#Controls", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

	if (m_isTerrainBody) {
		bool didChange = false;
		uint32_t prevSeed = m_state.seed;

		ImGui::TextUnformatted("Seed");
		int *seed = reinterpret_cast<int32_t *>(&m_state.seed);
		didChange |= ImGui::DragInt("##seed", seed);

		// Seed control buttons
		if (ImGui::Button("<"))
			m_state.seed--;
		ImGui::SameLine();
		if (ImGui::Button("Random Seed"))
			m_state.seed = Pi::rng.Int32();
		ImGui::SameLine();
		if (ImGui::Button(">"))
			m_state.seed++;

		if (m_state.seed != prevSeed)
			didChange = true;

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::TextUnformatted("Mass (earths):");
		didChange |= ImGui::DragDouble("##mass", &m_state.mass, 0.1, 0.01, std::numeric_limits<uint32_t>::max());

		ImGui::TextUnformatted("Radius (earths):");
		didChange |= ImGui::DragDouble("##radius", &m_state.radius, 0.1, 0.01, 10000.0);

		ImGui::TextUnformatted("Volatile gases (>= 0)");
		didChange |= ImGui::DragDouble("##volatile-gas", &m_state.volatileGas, 0.01, 0.0, 1000.0);

		ImGui::TextUnformatted("Volatile liquid (0-1)");
		didChange |= ImGui::DragDouble("##volatile-liquid", &m_state.volatileLiquid, 0.01, 0.0, 1.0);

		ImGui::TextUnformatted("Volatile ices (0-1)");
		didChange |= ImGui::DragDouble("##volatile-ices", &m_state.volatileIces, 0.01, 0.0, 1.0);

		ImGui::TextUnformatted("Life (0-1)");
		didChange |= ImGui::DragDouble("##life", &m_state.life, 0.01, 0.0, 1.0);

		ImGui::TextUnformatted("Volcanicity (0-1)");
		didChange |= ImGui::DragDouble("##volcanicity", &m_state.volcanicity, 0.01, 0.0, 1.0);

		ImGui::TextUnformatted("Crust metallicity (0-1)");
		didChange |= ImGui::DragDouble("##metallicity", &m_state.metallicity, 0.01, 0.0, 1.0);

		if (ImGui::Button("Change Planet Terrain Type"))
			didChange = true;

		if (didChange)
			OnChangeTerrain();
	}

	PiGui::RunHandler(Pi::GetFrameTime(), GetViewName() + ".Controls");

	ImGui::End();
}

void ObjectViewerView::DrawPiGui()
{
	if (m_targetBody) {
		DrawInfoWindow();

		DrawControlsWindow();
	}

	PiGuiView::DrawPiGui();
}

static constexpr fixed dtofixed(double val, uint32_t denom = 1 << 16)
{
	return fixed(denom * val, denom);
}

void ObjectViewerView::OnChangeTerrain()
{
	if (!m_isTerrainBody)
		return;

	// XXX this is horrendous, but probably safe for the moment. all bodies,
	// terrain, whatever else holds a const pointer to the same toplevel
	// sbody. one day objectviewer should be far more contained and not
	// actually modify the space
	SystemBody *sbody = const_cast<SystemBody *>(m_systemBody);

	sbody->m_seed = m_state.seed;
	sbody->m_radius = dtofixed(std::abs(Clamp(m_state.radius, 0.1, 10000.0)));
	sbody->m_mass = dtofixed(std::abs(m_state.mass));
	sbody->m_metallicity = dtofixed(std::abs(m_state.metallicity));
	sbody->m_volatileGas = dtofixed(std::abs(m_state.volatileGas));
	sbody->m_volatileLiquid = dtofixed(std::abs(m_state.volatileLiquid));
	sbody->m_volatileIces = dtofixed(std::abs(m_state.volatileIces));
	sbody->m_volcanicity = dtofixed(std::abs(m_state.volcanicity));
	sbody->m_life = dtofixed(std::abs(m_state.life));

	// FIXME: need a better solution to queue terrain updates for a specific planet
	// that doesn't involve destroying geopatches while they're being rendered
	// (and a way run those updates at the right time)
	Pi::renderer->FlushCommandBuffers();

	// force reload
	TerrainBody::OnChangeDetailLevel();
	ReloadState();
}

#endif
