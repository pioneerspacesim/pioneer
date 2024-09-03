// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "WorldView.h"

#include "Frame.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "HudTrail.h"
#include "HyperspaceCloud.h"
#include "Input.h"
#include "Json.h"
#include "Lang.h"
#include "Pi.h"
#include "Player.h"
#include "SDL_keycode.h"
#include "SectorView.h"
#include "Sensors.h"
#include "SpeedLines.h"
#include "StringF.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/RenderState.h"
#include "matrix4x4.h"
#include "ship/PlayerShipController.h"
#include "ship/ShipViewController.h"
#include "sound/Sound.h"

WorldView::~WorldView() {}

namespace {
	static const float HUD_CROSSHAIR_SIZE = 8.0f;
	static const Color green(0, 255, 0, 204);
	static const Color red(255, 0, 0, 128);
} // namespace

REGISTER_INPUT_BINDING(WorldView)
{
	using namespace InputBindings;
	Input::BindingGroup *group = input->GetBindingPage("General")->GetBindingGroup("Miscellaneous");

	input->AddActionBinding("BindToggleHudMode", group, Action({ SDLK_TAB }));
	input->AddActionBinding("BindIncreaseTimeAcceleration", group, Action({ SDLK_PAGEUP }));
	input->AddActionBinding("BindDecreaseTimeAcceleration", group, Action({ SDLK_PAGEDOWN }));
	// universal axes for selecting an item from a radial menu
	input->AddAxisBinding("BindRadialHorizontalSelection", group, Axis({}, { SDLK_LEFT }, { SDLK_RIGHT }));
	input->AddAxisBinding("BindRadialVerticalSelection", group, Axis({}, { SDLK_UP }, { SDLK_DOWN }));
	// radial menu activators
	input->AddActionBinding("BindFlightAssistRadial", group, Action{});
	input->AddActionBinding("BindFixheadingRadial", group, Action{});
}

void WorldView::InputBinding::RegisterBindings()
{
	toggleHudMode = AddAction("BindToggleHudMode");
	increaseTimeAcceleration = AddAction("BindIncreaseTimeAcceleration");
	decreaseTimeAcceleration = AddAction("BindDecreaseTimeAcceleration");
	AddAxis("BindRadialVerticalSelection");
	AddAxis("BindRadialHorizontalSelection");
	AddAction("BindFlightAssistRadial");
	AddAction("BindFixheadingRadial");
}

WorldView::WorldView(Game *game) :
	PiGuiView("WorldView"),
	m_game(game),
	InputBindings(Pi::input)
{
	InitObject();
}

WorldView::WorldView(const Json &jsonObj, Game *game) :
	PiGuiView("WorldView"),
	m_game(game),
	InputBindings(Pi::input)
{
	if (!jsonObj["world_view"].is_object()) throw SavedGameCorruptException();
	Json worldViewObj = jsonObj["world_view"];

	InitObject();

	shipView->LoadFromJson(worldViewObj);
}

void WorldView::InitObject()
{
	m_labelsOn = true;

	Graphics::MaterialDescriptor desc;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	rsd.depthTest = false;
	rsd.primitiveType = Graphics::LINE_SINGLE;
	m_indicatorMat.reset(Pi::renderer->CreateMaterial("vtxColor", desc, rsd));

	/*
	  NEW UI
	*/

	m_speedLines.reset(new SpeedLines(Pi::player));

	//get near & far clipping distances
	//XXX m_renderer not set yet
	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");

	m_cameraContext.Reset(new CameraContext(Pi::renderer->GetWindowWidth(), Pi::renderer->GetWindowHeight(), fovY, znear, zfar));
	m_camera.reset(new Camera(m_cameraContext, Pi::renderer));

	InputBindings.RegisterBindings();
	shipView.reset(new ShipViewController(this));
	shipView->Init();
	SetViewController(shipView.get());

	m_onToggleHudModeCon = InputBindings.toggleHudMode->onPressed.connect(sigc::mem_fun(this, &WorldView::OnToggleLabels));
	m_onIncTimeAccelCon = InputBindings.increaseTimeAcceleration->onPressed.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelInc));
	m_onDecTimeAccelCon = InputBindings.decreaseTimeAcceleration->onPressed.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelDec));
}


void WorldView::SaveToJson(Json &jsonObj)
{
	Json worldViewObj = Json::object(); // Create JSON object to contain world view data.

	shipView->SaveToJson(worldViewObj);

	jsonObj["world_view"] = worldViewObj; // Add world view object to supplied object.
}

void WorldView::OnRequestTimeAccelInc()
{
	// requests an increase in time acceleration
	Pi::game->RequestTimeAccelInc();
}

void WorldView::OnRequestTimeAccelDec()
{
	// requests a decrease in time acceleration
	Pi::game->RequestTimeAccelDec();
}

void WorldView::SetViewController(ViewController *newView)
{
	m_viewController = newView;
}

void WorldView::Draw3D()
{
	PROFILE_SCOPED()
	assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	m_cameraContext->ApplyDrawTransforms(m_renderer);

	m_camera->Draw();

	// NB: Do any screen space rendering after here:
	// Things like the cockpit and AR features like hudtrails, space dust etc.
	m_viewController->Draw(m_camera.get());

	// Draw 3D HUD
	// Speed lines
	if (Pi::AreSpeedLinesDisplayed())
		m_speedLines->Render(m_renderer);

	// Contact trails
	if (Pi::AreHudTrailsDisplayed()) {
		for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
			it->trail->Render(m_renderer);
	}

	m_cameraContext->EndFrame();

	// don't draw crosshairs etc in hyperspace
	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) return;

	// setup orthographic projection the indicator coordinate system expects
	// (could also draw this using ImGui methods in DrawPiGui, but this is a quick patch for release)
	m_renderer->SetProjection(matrix4x4f::OrthoFrustum(0, m_renderer->GetWindowWidth(), m_renderer->GetWindowHeight(), 0, 0, 1));
	m_renderer->SetTransform(matrix4x4f::Identity());

	// combat target indicator
	DrawCombatTargetIndicator(m_combatTargetIndicator, m_targetLeadIndicator, red);
}

void WorldView::OnToggleLabels()
{
	if (Pi::GetView() == this) {
		if (Pi::DrawGUI && m_labelsOn) {
			m_labelsOn = false;
		} else if (Pi::DrawGUI && !m_labelsOn) {
			Pi::DrawGUI = false;
		} else if (!Pi::DrawGUI) {
			Pi::DrawGUI = true;
			m_labelsOn = true;
		}
	}
}

void WorldView::Update()
{
	PROFILE_SCOPED()
	assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	m_viewController->Update();

	m_cameraContext->BeginFrame();
	m_camera->Update();

	UpdateProjectedObjects();

	FrameId playerFrameId = Pi::player->GetFrame();
	FrameId camFrameId = m_cameraContext->GetTempFrame();

	//speedlines and contact trails need camFrame for transform, so they
	//must be updated here
	if (Pi::AreSpeedLinesDisplayed()) {
		m_speedLines->Update(m_game->GetTimeStep());

		matrix4x4d trans;
		Frame::GetFrameTransform(playerFrameId, camFrameId, trans);

		if (m_speedLines.get() && Pi::AreSpeedLinesDisplayed()) {
			m_speedLines->Update(m_game->GetTimeStep());

			trans[12] = trans[13] = trans[14] = 0.0;
			trans[15] = 1.0;
			m_speedLines->SetTransform(trans);
		}
	}

	if (Pi::AreHudTrailsDisplayed()) {
		matrix4x4d trans;
		Frame::GetFrameTransform(playerFrameId, camFrameId, trans);

		for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
			it->trail->SetTransform(trans);
	} else {
		for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
			it->trail->Reset(playerFrameId);
	}
}

void WorldView::OnSwitchTo()
{
	if (m_viewController)
		m_viewController->Activated();
	Pi::input->AddInputFrame(&InputBindings);
}

void WorldView::OnSwitchFrom()
{
	if (m_viewController)
		m_viewController->Deactivated();
	Pi::input->RemoveInputFrame(&InputBindings);
	Pi::DrawGUI = true;
}

// XXX paying fine remotely can't really be done until crime and
// worldview are in Lua. I'm leaving this code here so its not
// forgotten
/*
static void PlayerPayFine()
{
	Sint64 crime, fine;
	Polit::GetCrime(&crime, &fine);
	if (Pi::player->GetMoney() == 0) {
		m_game->log->Add(Lang::YOU_NO_MONEY);
	} else if (fine > Pi::player->GetMoney()) {
		Polit::AddCrime(0, -Pi::player->GetMoney());
		Polit::GetCrime(&crime, &fine);
		m_game->log->Add(stringf(
			Lang::FINE_PAID_N_BUT_N_REMAINING,
				formatarg("paid", format_money(Pi::player->GetMoney())),
				formatarg("fine", format_money(fine))));
		Pi::player->SetMoney(0);
	} else {
		Pi::player->SetMoney(Pi::player->GetMoney() - fine);
		m_game->log->Add(stringf(Lang::FINE_PAID_N,
				formatarg("fine", format_money(fine))));
		Polit::AddCrime(0, -fine);
	}
}
*/

int WorldView::GetActiveWeapon() const
{
	using CamType = ShipViewController::CamType;
	switch (shipView->GetCamType()) {
	case CamType::CAM_INTERNAL:
		return shipView->m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR ? 1 : 0;

	case CamType::CAM_EXTERNAL:
	case CamType::CAM_SIDEREAL:
	case CamType::CAM_FLYBY:
	default:
		return 0;
	}
}

void WorldView::UpdateProjectedObjects()
{
	const Frame *cam_frame = Frame::GetFrame(m_cameraContext->GetTempFrame());
	matrix3x3d cam_rot = cam_frame->GetOrient().Inverse() * Pi::player->GetOrient();

	// later we might want non-ship enemies (e.g., for assaults on military bases)
	assert(!Pi::player->GetCombatTarget() || Pi::player->GetCombatTarget()->IsType(ObjectType::SHIP));

	// update combat HUD
	Ship *enemy = static_cast<Ship *>(Pi::player->GetCombatTarget());
	if (enemy) {
		const vector3d targScreenPos = enemy->GetInterpPositionRelTo(cam_frame->GetId());

		UpdateIndicator(m_combatTargetIndicator, targScreenPos);

		// calculate firing solution and relative velocity along our z axis
		int laser = -1;
		if (shipView->GetCamType() == ShipViewController::CAM_INTERNAL) {
			switch (shipView->m_internalCameraController->GetMode()) {
			case InternalCameraController::MODE_FRONT: laser = 0; break;
			case InternalCameraController::MODE_REAR: laser = 1; break;
			default: break;
			}
		}

		FixedGuns *gunManager = Pi::player->GetComponent<FixedGuns>();
		if (laser >= 0 && gunManager->IsGunMounted(laser) && gunManager->IsFiringSolutionOk()) {
			UpdateIndicator(m_targetLeadIndicator, cam_rot * gunManager->GetTargetLeadPos());
			if ((m_targetLeadIndicator.side != INDICATOR_ONSCREEN) || (m_combatTargetIndicator.side != INDICATOR_ONSCREEN))
				HideIndicator(m_targetLeadIndicator);
		} else {
			HideIndicator(m_targetLeadIndicator);
		}
	} else {
		HideIndicator(m_combatTargetIndicator);
		HideIndicator(m_targetLeadIndicator);
	}
}

void WorldView::UpdateIndicator(Indicator &indicator, const vector3d &cameraSpacePos)
{
	const Graphics::Frustum frustum = m_cameraContext->GetFrustum();

	const float BORDER = 10.0;
	const float BORDER_BOTTOM = 90.0;
	// XXX BORDER_BOTTOM is 10+the control panel height and shouldn't be needed at all

	const float w = m_renderer->GetWindowWidth();
	const float h = m_renderer->GetWindowHeight();

	if (cameraSpacePos.LengthSqr() < 1e-6) { // length < 1e-3
		indicator.pos.x = w / 2.0f;
		indicator.pos.y = h / 2.0f;
		indicator.side = INDICATOR_ONSCREEN;
		return;
	}

	vector3d proj;
	if (frustum.ProjectPoint(cameraSpacePos, proj)) {
		proj.x *= w;
		proj.y = (1.0f - proj.y) * h;
	} else {
		proj = vector3d(w / 2.0, h / 2.0, 0.0);
	}

	indicator.realpos.x = int(proj.x);
	indicator.realpos.y = int(proj.y);

	bool onscreen =
		(cameraSpacePos.z < 0.0) &&
		(proj.x >= BORDER) && (proj.x < w - BORDER) &&
		(proj.y >= BORDER) && (proj.y < h - BORDER_BOTTOM);

	if (onscreen) {
		indicator.pos.x = int(proj.x);
		indicator.pos.y = int(proj.y);
		indicator.side = INDICATOR_ONSCREEN;
	} else {
		// homogeneous 2D points and lines are really useful
		const vector3d ptCentre(w / 2.0, h / 2.0, 1.0);
		const vector3d ptProj(proj.x, proj.y, 1.0);
		const vector3d lnDir = ptProj.Cross(ptCentre);

		indicator.side = INDICATOR_TOP;

		// this fallback is used if direction is close to (0, 0, +ve)
		indicator.pos.x = w / 2.0;
		indicator.pos.y = BORDER;

		if (cameraSpacePos.x < -1e-3) {
			vector3d ptLeft = lnDir.Cross(vector3d(-1.0, 0.0, BORDER));
			ptLeft /= ptLeft.z;
			if (ptLeft.y >= BORDER && ptLeft.y < h - BORDER_BOTTOM) {
				indicator.pos.x = ptLeft.x;
				indicator.pos.y = ptLeft.y;
				indicator.side = INDICATOR_LEFT;
			}
		} else if (cameraSpacePos.x > 1e-3) {
			vector3d ptRight = lnDir.Cross(vector3d(-1.0, 0.0, w - BORDER));
			ptRight /= ptRight.z;
			if (ptRight.y >= BORDER && ptRight.y < h - BORDER_BOTTOM) {
				indicator.pos.x = ptRight.x;
				indicator.pos.y = ptRight.y;
				indicator.side = INDICATOR_RIGHT;
			}
		}

		if (cameraSpacePos.y < -1e-3) {
			vector3d ptBottom = lnDir.Cross(vector3d(0.0, -1.0, h - BORDER_BOTTOM));
			ptBottom /= ptBottom.z;
			if (ptBottom.x >= BORDER && ptBottom.x < w - BORDER) {
				indicator.pos.x = ptBottom.x;
				indicator.pos.y = ptBottom.y;
				indicator.side = INDICATOR_BOTTOM;
			}
		} else if (cameraSpacePos.y > 1e-3) {
			vector3d ptTop = lnDir.Cross(vector3d(0.0, -1.0, BORDER));
			ptTop /= ptTop.z;
			if (ptTop.x >= BORDER && ptTop.x < w - BORDER) {
				indicator.pos.x = ptTop.x;
				indicator.pos.y = ptTop.y;
				indicator.side = INDICATOR_TOP;
			}
		}
	}
}

void WorldView::HideIndicator(Indicator &indicator)
{
	indicator.side = INDICATOR_HIDDEN;
	indicator.pos = vector2f(0.0f, 0.0f);
}

void WorldView::Draw()
{
	assert(m_game);
	assert(Pi::player);

	m_renderer->ClearDepthBuffer();

	View::Draw();

	// glLineWidth(1.0f);
	m_renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
}

void WorldView::DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c)
{
	if (target.side == INDICATOR_HIDDEN) return;

	if (target.side == INDICATOR_ONSCREEN) {
		const float x1 = target.pos.x, y1 = target.pos.y;
		const float x2 = lead.pos.x, y2 = lead.pos.y;

		float xd = x2 - x1, yd = y2 - y1;
		if (lead.side != INDICATOR_ONSCREEN) {
			xd = 1.0f;
			yd = 0.0f;
		} else {
			float len = xd * xd + yd * yd;
			if (len < 1e-6) {
				xd = 1.0f;
				yd = 0.0f;
			} else {
				len = sqrt(len);
				xd /= len;
				yd /= len;
			}
		}

		const vector3f vts[] = {
			// target crosshairs
			vector3f(x1 + 10 * xd, y1 + 10 * yd, 0.0f),
			vector3f(x1 + 20 * xd, y1 + 20 * yd, 0.0f),
			vector3f(x1 - 10 * xd, y1 - 10 * yd, 0.0f),
			vector3f(x1 - 20 * xd, y1 - 20 * yd, 0.0f),
			vector3f(x1 - 10 * yd, y1 + 10 * xd, 0.0f),
			vector3f(x1 - 20 * yd, y1 + 20 * xd, 0.0f),
			vector3f(x1 + 10 * yd, y1 - 10 * xd, 0.0f),
			vector3f(x1 + 20 * yd, y1 - 20 * xd, 0.0f),

			// lead crosshairs
			vector3f(x2 - 10 * xd, y2 - 10 * yd, 0.0f),
			vector3f(x2 + 10 * xd, y2 + 10 * yd, 0.0f),
			vector3f(x2 - 10 * yd, y2 + 10 * xd, 0.0f),
			vector3f(x2 + 10 * yd, y2 - 10 * xd, 0.0f),

			// line between crosshairs
			vector3f(x1 + 20 * xd, y1 + 20 * yd, 0.0f),
			vector3f(x2 - 10 * xd, y2 - 10 * yd, 0.0f)
		};
		if (lead.side == INDICATOR_ONSCREEN) {
			m_indicator.SetData(14, vts, c);
		} else {
			m_indicator.SetData(8, vts, c);
		}
		m_indicator.Draw(m_renderer, m_indicatorMat.get());
	} else {
		DrawEdgeMarker(target, c);
	}
}

void WorldView::DrawEdgeMarker(const Indicator &marker, const Color &c)
{
	const vector2f screenCentre(m_renderer->GetWindowWidth() / 2.0f, m_renderer->GetWindowHeight() / 2.0f);
	vector2f dir = screenCentre - marker.pos;
	float len = dir.Length();
	dir *= HUD_CROSSHAIR_SIZE / len;

	const vector3f vts[2] = {
		vector3f(marker.pos, 0.f),
		vector3f(marker.pos + dir, 0.f)
	};
	m_indicator.SetData(2, vts, c);
	m_indicator.Draw(m_renderer, m_indicatorMat.get());
}

// project vector vec onto plane (normal must be normalized)
static vector3d projectVecOntoPlane(const vector3d &vec, const vector3d &normal)
{
	return (vec - vec.Dot(normal) * normal);
}

static double wrapAngleToPositive(const double theta)
{
	return (theta >= 0.0 ? theta : M_PI * 2 + theta);
}

/*
  heading range: 0 - 359 deg
  heading  0 - north
  heading 90 - east
  pitch range: -90 - +90 deg
  pitch  0 - level with surface
  pitch 90 - up
*/
std::tuple<double, double, double> WorldView::CalculateHeadingPitchRoll(PlaneType pt)
{
	FrameId frameId = Pi::player->GetFrame();

	if (pt == ROTATIONAL)
		frameId = Frame::GetFrame(frameId)->GetRotFrame();
	else if (pt == PARENT)
		frameId = Frame::GetFrame(frameId)->GetNonRotFrame();

	// construct a frame of reference aligned with the ground plane
	// and with lines of longitude and latitude
	const vector3d up = Pi::player->GetPositionRelTo(frameId).NormalizedSafe();
	const vector3d north = projectVecOntoPlane(vector3d(0, 1, 0), up).NormalizedSafe();
	const vector3d east = north.Cross(up);

	// find the direction that the ship is facing
	const auto shpRot = Pi::player->GetOrientRelTo(frameId);
	const vector3d hed = -shpRot.VectorZ();
	const vector3d right = shpRot.VectorX();
	const vector3d groundHed = projectVecOntoPlane(hed, up).NormalizedSafe();

	const double pitch = asin(up.Dot(hed));

	const double hedNorth = groundHed.Dot(north);
	const double hedEast = groundHed.Dot(east);
	const double heading = wrapAngleToPositive(atan2(hedEast, hedNorth));
	const double roll = (acos(right.Dot(up.Cross(hed).Normalized())) - M_PI) * (right.Dot(up) >= 0 ? -1 : 1);

	return std::make_tuple(
		std::isnan(heading) ? 0.0 : heading,
		std::isnan(pitch) ? 0.0 : pitch,
		std::isnan(roll) ? 0.0 : roll);
}

static vector3d projectToScreenSpace(const vector3d &pos, RefCountedPtr<CameraContext> cameraContext, const bool adjustZ = true)
{
	const Graphics::Frustum &frustum = cameraContext->GetFrustum();
	const float h = cameraContext->GetHeight();
	const float w = cameraContext->GetWidth();
	vector3d proj;
	if (!frustum.ProjectPoint(pos, proj)) {
		return vector3d(w / 2, h / 2, 0);
	}
	// convert NDC to top-left screen coordinates
	proj.x *= w;
	proj.y = h - proj.y * h;

	// linearize depth coordinate
	// see https://thxforthefish.com/posts/reverse_z/
	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);
	proj.z = -znear / proj.z;

	// set z to -1 if in front of camera, 1 else
	if (adjustZ)
		proj.z = pos.z < 0 ? -1 : 1;
	return proj;
}

// project a body in world-space to a screen-space location
vector3d WorldView::WorldSpaceToScreenSpace(const Body *body) const
{
	if (body->IsType(ObjectType::PLAYER) && !shipView->IsExteriorView())
		return vector3d(0, 0, 0);

	vector3d pos = body->GetInterpPositionRelTo(m_cameraContext->GetCameraFrame());
	return WorldSpaceToScreenSpace(pos);
}

// position is relative to the parent frame of the camera
// use the Body* overload to convert from other frames' spaces
vector3d WorldView::WorldSpaceToScreenSpace(const vector3d &position) const
{
	vector3d pos = (position - m_cameraContext->GetCameraPos()) * m_cameraContext->GetCameraOrient();
	return projectToScreenSpace(pos, m_cameraContext);
}

// convert a direction in world-space coordinates to a position in screen-space coordinates
vector3d WorldView::WorldDirToScreenSpace(const vector3d &pos) const
{
	// rotate world-space coordinates into the camera frame orientation
	return projectToScreenSpace(pos * m_cameraContext->GetCameraOrient(), m_cameraContext, false);
}

vector3d WorldView::CameraSpaceToScreenSpace(const vector3d &pos) const
{
	return projectToScreenSpace(pos, m_cameraContext);
}

vector3d WorldView::GetTargetIndicatorScreenPosition(const Body *body) const
{
	if (body->IsType(ObjectType::PLAYER) && !shipView->IsExteriorView())
		return vector3d(0, 0, 0);

	// get the target indicator position in body-local coordinates
	vector3d pos = body->GetInterpPositionRelTo(m_cameraContext->GetCameraFrame());
	pos += body->GetInterpOrientRelTo(m_cameraContext->GetCameraFrame()) * body->GetTargetIndicatorPosition();
	return WorldSpaceToScreenSpace(pos);
}
