// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "HudTrail.h"
#include "Player.h"
#include "Planet.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/GalaxyCache.h"
#include "SectorView.h"
#include "SystemView.h"
#include "Serializer.h"
#include "ShipCpanel.h"
#include "Sound.h"
#include "Space.h"
#include "SpaceStation.h"
#include "galaxy/StarSystem.h"
#include "HyperspaceCloud.h"
#include "KeyBindings.h"
#include "perlin.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"
#include "ui/Context.h"
#include "ui/Align.h"
#include "ui/Label.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Drawables.h"
#include "matrix4x4.h"
#include "Quaternion.h"
#include "LuaObject.h"
#include "utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <SDL_stdinc.h>

const double WorldView::PICK_OBJECT_RECT_SIZE = 20.0;
namespace {
	static const Color s_hudTextColor(0,255,0,230);
	static const float ZOOM_SPEED = 1.f;
	static const float WHEEL_SENSITIVITY = .05f;	// Should be a variable in user settings.
	static const float HUD_CROSSHAIR_SIZE = 8.0f;
	static const Uint8 HUD_ALPHA          = 87;
	static const Color white(255, 255, 255, 204);
	static const Color green(0, 255, 0, 204);
	static const Color yellow(230, 230, 77, 255);
	static const Color red(255, 0, 0, 128);
}

WorldView::WorldView(Game* game): UIView(), m_game(game)
{
	m_camType = CAM_INTERNAL;
	InitObject();
}

WorldView::WorldView(const Json::Value &jsonObj, Game* game) : UIView(), m_game(game)
{
	if (!jsonObj.isMember("world_view")) throw SavedGameCorruptException();
	Json::Value worldViewObj = jsonObj["world_view"];

	if (!worldViewObj.isMember("cam_type")) throw SavedGameCorruptException();
	m_camType = CamType(worldViewObj["cam_type"].asInt());
	InitObject();

	m_internalCameraController->LoadFromJson(worldViewObj);
	m_externalCameraController->LoadFromJson(worldViewObj);
	m_siderealCameraController->LoadFromJson(worldViewObj);
}

void WorldView::InitObject()
{
	float size[2];
	GetSizeRequested(size);

	m_labelsOn = true;
	SetTransparency(true);

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	rsd.depthTest = false;
	m_blendState = Pi::renderer->CreateRenderState(rsd); //XXX m_renderer not set yet
	m_navTunnel = new NavTunnelWidget(this, m_blendState);
	Add(m_navTunnel, 0, 0);

	m_wheelsButton = new Gui::MultiStateImageButton();
	m_wheelsButton->SetShortcut(SDLK_F6, KMOD_NONE);
	m_wheelsButton->AddState(0, "icons/wheels_up.png", Lang::WHEELS_ARE_UP);
	m_wheelsButton->AddState(1, "icons/wheels_down.png", Lang::WHEELS_ARE_DOWN);
	m_wheelsButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeWheelsState));
	m_wheelsButton->SetRenderDimensions(30.0f, 22.0f);
	m_rightButtonBar->Add(m_wheelsButton, 34, 2);

	m_hyperspaceButton = new Gui::MultiStateImageButton();
	m_hyperspaceButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_hyperspaceButton->AddState(0, "icons/hyperspace_disabled_f8.png", Lang::HYPERSPACE_JUMP_DISABLED);
	m_hyperspaceButton->AddState(1, "icons/hyperspace_forbidden_f8.png", Lang::HYPERSPACE_JUMP_FORBIDDEN);
	m_hyperspaceButton->AddState(2, "icons/hyperspace_forbidden_abort_f8.png", Lang::HYPERSPACE_JUMP_ABORT);
	m_hyperspaceButton->AddState(3, "icons/hyperspace_engage_f8.png", Lang::HYPERSPACE_JUMP_ENGAGE);
	m_hyperspaceButton->AddState(4, "icons/hyperspace_abort_f8.png", Lang::HYPERSPACE_JUMP_ABORT);
	m_hyperspaceButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickHyperspace));
	m_hyperspaceButton->SetRenderDimensions(30.0f, 22.0f);
	m_rightButtonBar->Add(m_hyperspaceButton, 66, 2);

	m_launchButton = new Gui::ImageButton("icons/blastoff.png");
	m_launchButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_launchButton->SetToolTip(Lang::TAKEOFF);
	m_launchButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickBlastoff));
	m_launchButton->SetRenderDimensions(30.0f, 22.0f);
	m_rightButtonBar->Add(m_launchButton, 2, 2);

	m_flightControlButton = new Gui::MultiStateImageButton();
	m_flightControlButton->SetShortcut(SDLK_F5, KMOD_NONE);
	// these states must match Player::FlightControlState (so that the enum values match)
	m_flightControlButton->AddState(CONTROL_MANUAL, "icons/manual_control.png", Lang::MANUAL_CONTROL);
	m_flightControlButton->AddState(CONTROL_FIXSPEED, "icons/manual_control.png", Lang::COMPUTER_SPEED_CONTROL);
	m_flightControlButton->AddState(CONTROL_FIXHEADING_FORWARD, "icons/manual_control.png", Lang::COMPUTER_HEADING_CONTROL);
	m_flightControlButton->AddState(CONTROL_FIXHEADING_BACKWARD, "icons/manual_control.png", Lang::COMPUTER_HEADING_CONTROL);
	m_flightControlButton->AddState(CONTROL_AUTOPILOT, "icons/autopilot.png", Lang::AUTOPILOT_ON);
	m_flightControlButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeFlightState));
	m_flightControlButton->SetRenderDimensions(30.0f, 22.0f);
	m_rightButtonBar->Add(m_flightControlButton, 2, 2);

	m_flightStatus = (new Gui::Label(""))->Color(255, 178, 0);
	m_rightRegion2->Add(m_flightStatus, 2, 0);

#if WITH_DEVKEYS
	Gui::Screen::PushFont("ConsoleFont");
	m_debugInfo = (new Gui::Label(""))->Color(204, 204, 204);
	Add(m_debugInfo, 10, 200);
	Gui::Screen::PopFont();
#endif
	/*
	  NEW UI
	*/

	// set up anchored docking positions for new-ui HUD widgets
	m_hudDockTop.Reset(Pi::ui->Align(UI::Align::TOP));
	m_hudDockRight.Reset(Pi::ui->Align(UI::Align::RIGHT));
	m_hudDockLeft.Reset(Pi::ui->Align(UI::Align::LEFT));
	m_hudDockBottom.Reset(Pi::ui->Align(UI::Align::BOTTOM));
	m_hudDockCentre.Reset(Pi::ui->Align(UI::Align::MIDDLE));

	// It's not ideal to use a nested VBox/HBox set for this, but it's
	// probably adequate for now and we can easily replace it later
	UI::VBox *hud_root = Pi::ui->VBox();
	hud_root->PackEnd(m_hudDockTop.Get());
	hud_root->PackEnd(Pi::ui->HBox()->
										PackEnd(m_hudDockLeft.Get())->
										PackEnd(Pi::ui->Expand()->SetInnerWidget(m_hudDockCentre.Get()))->
										PackEnd(m_hudDockRight.Get()));
	hud_root->PackEnd(m_hudDockBottom.Get());

	m_hudRoot.Reset(hud_root);

	m_headingInfo.Reset(Pi::ui->Label("heading")->SetColor(s_hudTextColor));
	m_pitchInfo.Reset(Pi::ui->Label("pitch")->SetColor(s_hudTextColor));
	m_headingInfo->onClick.connect(sigc::mem_fun(*this, &WorldView::OnClickHeadingLabel));
	m_curPlane = NONE;

	// --

	m_hudHyperspaceInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_hudHyperspaceInfo, Gui::Screen::GetWidth()*0.4f, Gui::Screen::GetHeight()*0.3f);

	m_hudSensorGaugeStack = new Gui::VBox();
	m_hudSensorGaugeStack->SetSpacing(2.0f);
	Add(m_hudSensorGaugeStack, 5.0f, 5.0f);

	m_hudTargetHullIntegrity = new Gui::MeterBar(100.0f, Lang::HULL_INTEGRITY, Color(255,255,0,204));
	m_hudTargetShieldIntegrity = new Gui::MeterBar(100.0f, Lang::SHIELD_INTEGRITY, Color(255,255,0,204));
	Add(m_hudTargetHullIntegrity, Gui::Screen::GetWidth() - 105.0f, 5.0f);
	Add(m_hudTargetShieldIntegrity, Gui::Screen::GetWidth() - 105.0f, 45.0f);

	m_hudTargetInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_hudTargetInfo, 0, 85.0f);

	// XXX m_renderer not set yet
	Graphics::TextureBuilder b1 = Graphics::TextureBuilder::UI("icons/indicator_mousedir.png");
	m_indicatorMousedir.reset(new Gui::TexturedQuad(b1.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));

	const Graphics::TextureDescriptor &descriptor = b1.GetDescriptor();
	m_indicatorMousedirSize = vector2f(descriptor.dataSize.x*descriptor.texSize.x,descriptor.dataSize.y*descriptor.texSize.y);

	m_speedLines.reset(new SpeedLines(Pi::player));

	//get near & far clipping distances
	//XXX m_renderer not set yet
	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");

	m_cameraContext.Reset(new CameraContext(Graphics::GetScreenWidth(), Graphics::GetScreenHeight(), fovY, znear, zfar));
	m_camera.reset(new Camera(m_cameraContext, Pi::renderer));
	m_internalCameraController.reset(new InternalCameraController(m_cameraContext, Pi::player));
	m_externalCameraController.reset(new ExternalCameraController(m_cameraContext, Pi::player));
	m_siderealCameraController.reset(new SiderealCameraController(m_cameraContext, Pi::player));
	SetCamType(m_camType); //set the active camera

	m_onHyperspaceTargetChangedCon =
		m_game->GetSectorView()->onHyperspaceTargetChanged.connect(sigc::mem_fun(this, &WorldView::OnHyperspaceTargetChanged));

	m_onPlayerChangeTargetCon =
		Pi::onPlayerChangeTarget.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeTarget));
	m_onChangeFlightControlStateCon =
		Pi::onPlayerChangeFlightControlState.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeFlightControlState));
	m_onMouseWheelCon =
		Pi::onMouseWheel.connect(sigc::mem_fun(this, &WorldView::MouseWheel));

	Pi::player->GetPlayerController()->SetMouseForRearView(GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
	KeyBindings::toggleHudMode.onPress.connect(sigc::mem_fun(this, &WorldView::OnToggleLabels));
	KeyBindings::increaseTimeAcceleration.onPress.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelInc));
	KeyBindings::decreaseTimeAcceleration.onPress.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelDec));
}

WorldView::~WorldView()
{
	m_onHyperspaceTargetChangedCon.disconnect();
	m_onPlayerChangeTargetCon.disconnect();
	m_onChangeFlightControlStateCon.disconnect();
	m_onMouseWheelCon.disconnect();
}

void WorldView::SaveToJson(Json::Value &jsonObj)
{
	Json::Value worldViewObj(Json::objectValue); // Create JSON object to contain world view data.

	worldViewObj["cam_type"] = int(m_camType);
	m_internalCameraController->SaveToJson(worldViewObj);
	m_externalCameraController->SaveToJson(worldViewObj);
	m_siderealCameraController->SaveToJson(worldViewObj);

	jsonObj["world_view"] = worldViewObj; // Add world view object to supplied object.
}

void WorldView::SetCamType(enum CamType c)
{
	Pi::BoinkNoise();

	// don't allow external cameras when docked inside space stations.
	// they would clip through the station model
	//if (Pi::player->GetFlightState() == Ship::DOCKED && !Pi::player->GetDockedWith()->IsGroundStation())
	//	c = CAM_INTERNAL;

	m_camType = c;

	switch(m_camType) {
	case CAM_INTERNAL:
		m_activeCameraController = m_internalCameraController.get();
		Pi::player->OnCockpitActivated();
		break;
	case CAM_EXTERNAL:
		m_activeCameraController = m_externalCameraController.get();
		break;
	case CAM_SIDEREAL:
		m_activeCameraController = m_siderealCameraController.get();
		break;
	}

	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);

	m_activeCameraController->Reset();

	onChangeCamType.emit();
}

void WorldView::ChangeInternalCameraMode(InternalCameraController::Mode m)
{
	if (m_internalCameraController->GetMode() != m)
		Pi::BoinkNoise();
	m_internalCameraController->SetMode(m);
	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
}

void WorldView::OnChangeWheelsState(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	if (!Pi::player->SetWheelState(b->GetState()!=0)) {
		b->StatePrev();
	}
}

/* This is UI click to change flight control state (manual, speed ctrl) */
void WorldView::OnChangeFlightState(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	int newState = b->GetState();
	if (Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL)) {
		// skip certain states
		switch (newState) {
		case CONTROL_FIXSPEED: newState = CONTROL_FIXHEADING_FORWARD; break;
		case CONTROL_AUTOPILOT: newState = CONTROL_MANUAL; break;
		default: break;
		}
	} else {
		// skip certain states
		switch (newState) {
		case CONTROL_FIXHEADING_FORWARD: // fallthrough
		case CONTROL_FIXHEADING_BACKWARD: newState = CONTROL_MANUAL; break;
		case CONTROL_AUTOPILOT: newState = CONTROL_MANUAL; break;
		default: break;
		}
	}
	b->SetActiveState(newState);
	Pi::player->GetPlayerController()->SetFlightControlState(static_cast<FlightControlState>(newState));
}

/* This is when the flight control state actually changes... */
void WorldView::OnPlayerChangeFlightControlState()
{
	m_flightControlButton->SetActiveState(Pi::player->GetPlayerController()->GetFlightControlState());
}

void WorldView::OnClickBlastoff()
{
	Pi::BoinkNoise();
	if (Pi::player->GetFlightState() == Ship::DOCKED) {
		Pi::player->Undock();
	} else {
		Pi::player->Blastoff();
	}
}

void WorldView::OnClickHyperspace(Gui::MultiStateImageButton *b)
{
	if(Pi::player->GetFlightState() == Ship::DOCKED || Pi::player->GetFlightState() == Ship::LANDED){
		// Maybe not the best, but flip state back (from disabled to disabled, I assume?)
		m_hyperspaceButton->StatePrev();
	}

	if (Pi::player->IsHyperspaceActive()) {
		// Hyperspace countdown in effect.. abort!
		Pi::player->AbortHyperjump();
		m_game->log->Add(Lang::HYPERSPACE_JUMP_ABORTED);

		// State backs once from original state
		m_hyperspaceButton->StatePrev(); // reset to original state...
		m_hyperspaceButton->StatePrev(); // ... -1 from original state
	}
	else{
		// Initiate hyperspace drive
		SystemPath path = m_game->GetSectorView()->GetHyperspaceTarget();
		LuaObject<Player>::CallMethod(Pi::player, "HyperjumpTo", &path);
	}
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

void WorldView::ResetHyperspaceButton()
{
	// After a jump:
	m_hyperspaceButton->SetActiveState(0);
}

void WorldView::Draw3D()
{
	PROFILE_SCOPED()
		assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	m_cameraContext->ApplyDrawTransforms(m_renderer);

	Body* excludeBody = nullptr;
	ShipCockpit* cockpit = nullptr;
	if(GetCamType() == CAM_INTERNAL) {
		excludeBody = Pi::player;
		if (m_internalCameraController->GetMode() == InternalCameraController::MODE_FRONT)
			cockpit = Pi::player->GetCockpit();
	}
	m_camera->Draw(excludeBody, cockpit);

	// Draw 3D HUD
	// Speed lines
	if (Pi::AreSpeedLinesDisplayed())
		m_speedLines->Render(m_renderer);

	// Contact trails
	if( Pi::AreHudTrailsDisplayed() ) {
		for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
			it->trail->Render(m_renderer);
	}

	m_cameraContext->EndFrame();

	UIView::Draw3D();
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

void WorldView::ShowAll()
{
	View::ShowAll(); // by default, just delegate back to View
	RefreshButtonStateAndVisibility();
}

static Color get_color_for_warning_meter_bar(float v) {
	Color c;
	if (v < 50.0f)
		c = Color(255,0,0,HUD_ALPHA);
	else if (v < 75.0f)
		c = Color(255,128,0,HUD_ALPHA);
	else
		c = Color(255,255,0,HUD_ALPHA);
	return c;
}

void WorldView::RefreshHyperspaceButton() {

	// 0 = "disabled" - if target selected but landed
	// 1 = "forbidden" - if flying below allowed jump altitude
	// 2 = "forbidden_abort" - if countdown below allowed jump altitude
	// 3 = "engage" - above allowed jump distance
	// 4 = "engage_abort" - abort current countdown, above allowed altitude
	//
	// (Note: when pressing a button in state 1..4, state is auto-incremented by one).

	SystemPath target = m_game->GetSectorView()->GetHyperspaceTarget();
	if (LuaObject<Ship>::CallMethod<bool>(Pi::player, "CanHyperjumpTo", &target)){
//		std::cout << "ONE" << std::endl;
		if(Pi::player->GetFlightState() == Ship::FLYING || Pi::player->GetFlightState() == Ship::JUMPING)
		{
//			std::cout << "TWO" << std::endl;
			// leave the "disabled" state if not landed:
			if(m_hyperspaceButton->GetState() == 0)
				m_hyperspaceButton->StateNext();

			if(!LuaObject<Ship>::CallMethod<bool>(Pi::player, "IsHyperjumpAllowed")){
				// If crossing boundary from above
				if(3 <= m_hyperspaceButton->GetState()){
					m_hyperspaceButton->StatePrev();
					m_hyperspaceButton->StatePrev();
				}
			}
			else{
				// If crossing the boundary from below
				if(2 >= m_hyperspaceButton->GetState()){
					m_hyperspaceButton->StateNext();
					m_hyperspaceButton->StateNext();
				}
			}
		}
		else{
			//grayed out disabled button, if target set while LANDED/DOCKED/(UN)DOCKING
			m_hyperspaceButton->SetActiveState(0);
		}

		m_hyperspaceButton->Show();
	}
	else
		//If no target selected, then no button at all:
		m_hyperspaceButton->Hide();
}

static std::pair<double, double> calculateHeadingPitch(enum PlaneType);

void WorldView::RefreshHeadingPitch(void) {
	if(m_curPlane == NONE) {
		m_hudDockTop->SetInnerWidget(m_headingInfo.Get());
		m_hudDockRight->SetInnerWidget(m_pitchInfo.Get());
		m_curPlane = ROTATIONAL;
	}
	// heading and pitch
	auto headingPitch = calculateHeadingPitch(m_curPlane);
	char buf[6];
	const double heading_deg = RAD2DEG(headingPitch.first);
	const double pitch_deg = RAD2DEG(headingPitch.second);
	// \xC2\xB0 is the UTF-8 degree symbol
	// normal rounding (as performed by printf) is incorrect for the heading
	// because it rounds x >= 359.5 *up* to 360 without wrapping back to zero.
	snprintf(buf, sizeof(buf), "%3.0f\xC2\xB0",
					 (heading_deg < 359.5 ? heading_deg : 0.0));
	m_headingInfo->SetText(buf);
	snprintf(buf, sizeof(buf), "%3.0f\xC2\xB0", pitch_deg);
	m_pitchInfo->SetText(buf);
}

void WorldView::RefreshButtonStateAndVisibility()
{
	assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	m_game->GetCpan()->ClearOverlay();

	if (Pi::player->GetFlightState() != Ship::HYPERSPACE) {
		m_game->GetCpan()->SetOverlayToolTip(ShipCpanel::OVERLAY_TOP_LEFT,     Lang::SHIP_VELOCITY_BY_REFERENCE_OBJECT);
		m_game->GetCpan()->SetOverlayToolTip(ShipCpanel::OVERLAY_TOP_RIGHT,    Lang::DISTANCE_FROM_SHIP_TO_NAV_TARGET);
		m_game->GetCpan()->SetOverlayToolTip(ShipCpanel::OVERLAY_BOTTOM_LEFT,  Lang::EXTERNAL_ATMOSPHERIC_PRESSURE);
		m_game->GetCpan()->SetOverlayToolTip(ShipCpanel::OVERLAY_BOTTOM_RIGHT, Lang::SHIP_ALTITUDE_ABOVE_TERRAIN);
	}

	if (is_equal_exact(Pi::player->GetWheelState(), 0.0f) && Pi::player->ExtrapolateHullTemperature() > 0.7)
		m_wheelsButton->Hide();
	else
		m_wheelsButton->Show();
	m_wheelsButton->SetActiveState(int(Pi::player->GetWheelState()) || Pi::player->GetWheelTransition() == 1);

	RefreshHyperspaceButton();

	switch(Pi::player->GetFlightState()) {
	case Ship::LANDED:
		m_flightStatus->SetText(Lang::LANDED);
		m_launchButton->Show();
		m_flightControlButton->Hide();
		break;

	case Ship::DOCKING:
		m_flightStatus->SetText(Lang::DOCKING);
		m_launchButton->Hide();
		m_flightControlButton->Hide();
		break;

	case Ship::UNDOCKING:
		m_flightStatus->SetText(Lang::UNDOCKING);
		m_launchButton->Hide();
		m_flightControlButton->Hide();
		break;

	case Ship::DOCKED:
		m_flightStatus->SetText(Lang::DOCKED);
		m_launchButton->Show();
		m_flightControlButton->Hide();
		break;

	case Ship::JUMPING:
	case Ship::HYPERSPACE:
		m_flightStatus->SetText(Lang::HYPERSPACE);
		m_launchButton->Hide();
		m_flightControlButton->Hide();
		break;

	case Ship::FLYING:
	default:
		const FlightControlState fstate = Pi::player->GetPlayerController()->GetFlightControlState();
		switch (fstate) {
		case CONTROL_MANUAL:
			m_flightStatus->SetText(Lang::MANUAL_CONTROL); break;

		case CONTROL_FIXSPEED: {
			std::string msg;
			const double setspeed = Pi::player->GetPlayerController()->GetSetSpeed();
			if (setspeed > 1000) {
				msg = stringf(Lang::SET_SPEED_KM_S, formatarg("speed", setspeed*0.001));
			} else {
				msg = stringf(Lang::SET_SPEED_M_S, formatarg("speed", setspeed));
			}
			m_flightStatus->SetText(msg);
			break;
		}

		case CONTROL_FIXHEADING_FORWARD:
			m_flightStatus->SetText(Lang::HEADING_LOCK_FORWARD);
			break;
		case CONTROL_FIXHEADING_BACKWARD:
			m_flightStatus->SetText(Lang::HEADING_LOCK_BACKWARD);
			break;
		case CONTROL_FIXHEADING_NORMAL:
			m_flightStatus->SetText(Lang::HEADING_LOCK_NORMAL);
			break;
		case CONTROL_FIXHEADING_ANTINORMAL:
			m_flightStatus->SetText(Lang::HEADING_LOCK_ANTINORMAL);
			break;
		case CONTROL_FIXHEADING_RADIALLY_INWARD:
			m_flightStatus->SetText(Lang::HEADING_LOCK_RADIALLY_INWARD);
			break;
		case CONTROL_FIXHEADING_RADIALLY_OUTWARD:
			m_flightStatus->SetText(Lang::HEADING_LOCK_RADIALLY_OUTWARD);
			break;
		case CONTROL_FIXHEADING_KILLROT:
			m_flightStatus->SetText(Lang::HEADING_LOCK_KILLROT);
			break;

		case CONTROL_AUTOPILOT:
			m_flightStatus->SetText(Lang::AUTOPILOT_CONTROL);
			break;

		default: assert(0); break;
		}

		m_launchButton->Hide();
		m_flightControlButton->Show();
	}

	// Direction indicator
	vector3d vel = Pi::player->GetVelocity();

#if WITH_DEVKEYS
	if (Pi::showDebugInfo) {
		std::ostringstream ss;

		if (Pi::player->GetFlightState() != Ship::HYPERSPACE) {
			vector3d pos = Pi::player->GetPosition();
			vector3d abs_pos = Pi::player->GetPositionRelTo(m_game->GetSpace()->GetRootFrame());

			const Frame *playerFrame = Pi::player->GetFrame();

			ss << stringf("Pos: %0{f.2}, %1{f.2}, %2{f.2}\n", pos.x, pos.y, pos.z);
			ss << stringf("AbsPos: %0{f.2}, %1{f.2}, %2{f.2}\n", abs_pos.x, abs_pos.y, abs_pos.z);

			const SystemPath &path(playerFrame->GetSystemBody()->GetPath());
			ss << stringf("Rel-to: %0 [%1{d},%2{d},%3{d},%4{u},%5{u}] ",
										playerFrame->GetLabel(),
										path.sectorX, path.sectorY, path.sectorZ, path.systemIndex, path.bodyIndex);
			ss << stringf("(%0{f.2} km), rotating: %1, has rotation: %2\n",
										pos.Length()/1000, (playerFrame->IsRotFrame() ? "yes" : "no"), (playerFrame->HasRotFrame() ? "yes" : "no"));

			//Calculate lat/lon for ship position
			const vector3d dir = pos.NormalizedSafe();
			const float lat = RAD2DEG(asin(dir.y));
			const float lon = RAD2DEG(atan2(dir.x, dir.z));

			ss << stringf("Lat / Lon: %0{f.8} / %1{f.8}\n", lat, lon);
		}

		char aibuf[256];
		Pi::player->AIGetStatusText(aibuf); aibuf[255] = 0;
		ss << aibuf << std::endl;

		m_debugInfo->SetText(ss.str());
		m_debugInfo->Show();
	} else {
		m_debugInfo->Hide();
	}
#endif
	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) {
		const SystemPath dest = Pi::player->GetHyperspaceDest();
		RefCountedPtr<StarSystem> s = m_game->GetGalaxy()->GetStarSystem(dest);

		m_game->GetCpan()->SetOverlayText(ShipCpanel::OVERLAY_TOP_LEFT, stringf(Lang::IN_TRANSIT_TO_N_X_X_X,
																																						formatarg("system", dest.IsBodyPath() ? s->GetBodyByPath(dest)->GetName() : s->GetName()),
																																						formatarg("x", dest.sectorX),
																																						formatarg("y", dest.sectorY),
																																						formatarg("z", dest.sectorZ)));

		m_game->GetCpan()->SetOverlayText(ShipCpanel::OVERLAY_TOP_RIGHT, stringf(Lang::JUMP_COMPLETE,
			formatarg("percent", m_game->GetHyperspaceArrivalProbability()*100.0, "f3.1")));
	}

	else {
		{
			std::string str;
			double _vel = 0;
			const char *rel_to = 0;
			const Body *set_speed_target = Pi::player->GetSetSpeedTarget();
			if (set_speed_target) {
				rel_to = set_speed_target->GetLabel().c_str();
				_vel = Pi::player->GetVelocityRelTo(set_speed_target).Length();
			} else {
				rel_to = Pi::player->GetFrame()->GetLabel().c_str();
				_vel = vel.Length();
			}
			if (_vel > 1000) {
				str = stringf(Lang::KM_S_RELATIVE_TO, formatarg("speed", _vel*0.001), formatarg("frame", rel_to));
			} else {
				str = stringf(Lang::M_S_RELATIVE_TO, formatarg("speed", _vel), formatarg("frame", rel_to));
			}
			// m_game->GetCpan()->SetOverlayText(ShipCpanel::OVERLAY_TOP_LEFT, str);
		}

		int hasSensors = 0;
		Pi::player->Properties().Get("sensor_cap", hasSensors);
		if (hasSensors) {
			m_hudSensorGaugeStack->DeleteAllChildren();
			lua_State *l = Lua::manager->GetLuaState();
			const int clean_stack = lua_gettop(l);
			LuaObject<Ship>::CallMethod<LuaRef>(Pi::player, "GetEquip", "sensor").PushCopyToStack();
			const int numSensorSlots = LuaObject<Ship>::CallMethod<int>(Pi::player, "GetEquipSlotCapacity", "sensor");
			if (numSensorSlots) {
				lua_pushnil(l);
				while(lua_next(l, -2)) {
					if (lua_type(l, -2) == LUA_TNUMBER) {
						LuaTable sensor(l, -1);
						const float sensor_progress = sensor.CallMethod<float>("GetProgress");
						if (sensor_progress > 0.0 && sensor_progress < 100.f) {
							const auto sensor_gauge = new Gui::MeterBar(100.f, sensor.CallMethod<std::string>("GetName").c_str(), Color(255, 255, 0, 204));
							sensor_gauge->SetValue(sensor_progress/100.f);
							sensor_gauge->Show();
							m_hudSensorGaugeStack->PackEnd(sensor_gauge);
						}
					}
					lua_pop(l, 1);
				}
			}
			lua_settop(l, clean_stack);
		}
	}

	Body *b = Pi::player->GetCombatTarget() ? Pi::player->GetCombatTarget() : Pi::player->GetNavTarget();
	if (b) {
		if (b->IsType(Object::SHIP)) {
			int prop_var = 0;
			Pi::player->Properties().Get("radar_mapper_level_cap", prop_var);
			if (prop_var > 0) {
				assert(b->IsType(Object::SHIP));
				Ship *s = static_cast<Ship*>(b);

				const shipstats_t &stats = s->GetStats();

				float sShields = 0;
				float sHull = s->GetPercentHull();
				m_hudTargetHullIntegrity->SetColor(get_color_for_warning_meter_bar(sHull));
				m_hudTargetHullIntegrity->SetValue(sHull*0.01f);
				m_hudTargetHullIntegrity->Show();

				prop_var = 0;
				s->Properties().Get("shield_cap", prop_var);
				if (prop_var > 0) {
					sShields = s->GetPercentShields();
				}
				m_hudTargetShieldIntegrity->SetColor(get_color_for_warning_meter_bar(sShields));
				m_hudTargetShieldIntegrity->SetValue(sShields*0.01f);
				m_hudTargetShieldIntegrity->Show();

				std::string text;
				text += s->GetShipType()->name;
				text += "\n";
				text += s->GetLabel();
				text += "\n";

				lua_State * l = Lua::manager->GetLuaState();
				int clean_stack = lua_gettop(l);

				LuaObject<Ship>::CallMethod<LuaRef>(s, "GetEquip", "engine").PushCopyToStack();
				lua_rawgeti(l, -1, 1);
				if (lua_isnil(l, -1)) {
					text += Lang::NO_HYPERDRIVE;
				} else {
					text += LuaTable(l, -1).CallMethod<std::string>("GetName");
				}
				lua_settop(l, clean_stack);

				text += "\n";
				text += stringf(Lang::MASS_N_TONNES, formatarg("mass", stats.static_mass));
				text += "\n";
				text += stringf(Lang::SHIELD_STRENGTH_N, formatarg("shields",
																													 (sShields*0.01f) * float(prop_var))); // At that point, it still holds the property for the shields
				text += "\n";
				text += stringf(Lang::CARGO_N, formatarg("mass", stats.used_cargo));
				text += "\n";

				m_hudTargetInfo->SetText(text);
				MoveChild(m_hudTargetInfo, Gui::Screen::GetWidth() - 150.0f, 85.0f);
				m_hudTargetInfo->Show();
			}
		}

		else if (b->IsType(Object::HYPERSPACECLOUD)) {
			int cap = 0;
			Pi::player->Properties().Get("hypercloud_analyzer_cap", cap);
			if(cap) {
				HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(b);

				m_hudTargetHullIntegrity->Hide();
				m_hudTargetShieldIntegrity->Hide();

				std::string text;

				Ship *ship = cloud->GetShip();
				if (!ship) {
					text += Lang::HYPERSPACE_ARRIVAL_CLOUD_REMNANT;
				}
				else {
					const SystemPath& dest = ship->GetHyperspaceDest();
					RefCountedPtr<const Sector> s = m_game->GetGalaxy()->GetSector(dest);
					text += (cloud->IsArrival() ? Lang::HYPERSPACE_ARRIVAL_CLOUD : Lang::HYPERSPACE_DEPARTURE_CLOUD);
					text += "\n";
					text += stringf(Lang::SHIP_MASS_N_TONNES, formatarg("mass", ship->GetStats().static_mass));
					text += "\n";
					text += (cloud->IsArrival() ? Lang::SOURCE : Lang::DESTINATION);
					text += ": ";
					text += s->m_systems[dest.systemIndex].GetName();
					text += "\n";
					text += stringf(Lang::DATE_DUE_N, formatarg("date", format_date(cloud->GetDueDate())));
					text += "\n";
				}

				m_hudTargetInfo->SetText(text);
				MoveChild(m_hudTargetInfo, Gui::Screen::GetWidth() - 180.0f, 5.0f);
				m_hudTargetInfo->Show();
			}
		}

		else {
			b = 0;
		}
	}
	if (!b) {
		m_hudTargetHullIntegrity->Hide();
		m_hudTargetShieldIntegrity->Hide();
		m_hudTargetInfo->Hide();
	}

	if (Pi::player->IsHyperspaceActive()) {
		float val = Pi::player->GetHyperspaceCountdown();

		if (!(int(ceil(val*2.0)) % 2)) {
			m_hudHyperspaceInfo->SetText(stringf(Lang::HYPERSPACING_IN_N_SECONDS, formatarg("countdown", ceil(val))));
			m_hudHyperspaceInfo->Show();
		} else {
			m_hudHyperspaceInfo->Hide();
		}
	} else {
		m_hudHyperspaceInfo->Hide();
	}
}

bool WorldView::OnClickHeadingLabel(void) {
	m_curPlane = m_curPlane == ROTATIONAL ? PARENT : ROTATIONAL;
	m_game->log->Add(m_curPlane == ROTATIONAL ? Lang::SWITCHED_TO_ROTATIONAL : Lang::SWITCHED_TO_PARENT);
	return true;
}

void WorldView::Update()
{
	PROFILE_SCOPED()
		assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	const double frameTime = Pi::GetFrameTime();
	// show state-appropriate buttons
	RefreshButtonStateAndVisibility();

	bool targetObject = false;

	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
		if (GetCamType() == CAM_INTERNAL) {
			if      (KeyBindings::frontCamera.IsActive())  ChangeInternalCameraMode(InternalCameraController::MODE_FRONT);
			else if (KeyBindings::rearCamera.IsActive())   ChangeInternalCameraMode(InternalCameraController::MODE_REAR);
			else if (KeyBindings::leftCamera.IsActive())   ChangeInternalCameraMode(InternalCameraController::MODE_LEFT);
			else if (KeyBindings::rightCamera.IsActive())  ChangeInternalCameraMode(InternalCameraController::MODE_RIGHT);
			else if (KeyBindings::topCamera.IsActive())    ChangeInternalCameraMode(InternalCameraController::MODE_TOP);
			else if (KeyBindings::bottomCamera.IsActive()) ChangeInternalCameraMode(InternalCameraController::MODE_BOTTOM);
		}
		else {
			MoveableCameraController *cam = static_cast<MoveableCameraController*>(m_activeCameraController);
			if (KeyBindings::cameraRotateUp.IsActive()) cam->RotateUp(frameTime);
			if (KeyBindings::cameraRotateDown.IsActive()) cam->RotateDown(frameTime);
			if (KeyBindings::cameraRotateLeft.IsActive()) cam->RotateLeft(frameTime);
			if (KeyBindings::cameraRotateRight.IsActive()) cam->RotateRight(frameTime);
			if (KeyBindings::viewZoomOut.IsActive()) cam->ZoomEvent(ZOOM_SPEED*frameTime);		// Zoom out
			if (KeyBindings::viewZoomIn.IsActive()) cam->ZoomEvent(-ZOOM_SPEED*frameTime);
			if (KeyBindings::cameraRollLeft.IsActive()) cam->RollLeft(frameTime);
			if (KeyBindings::cameraRollRight.IsActive()) cam->RollRight(frameTime);
			if (KeyBindings::resetCamera.IsActive()) cam->Reset();
			cam->ZoomEventUpdate(frameTime);
		}

		// note if we have to target the object in the crosshairs
		targetObject = KeyBindings::targetObject.IsActive();
	}

	m_activeCameraController->Update();

	m_cameraContext->BeginFrame();
	m_camera->Update();

	UpdateProjectedObjects();

	const Frame *playerFrame = Pi::player->GetFrame();
	const Frame *camFrame = m_cameraContext->GetCamFrame();

	//speedlines and contact trails need camFrame for transform, so they
	//must be updated here
	if (Pi::AreSpeedLinesDisplayed()) {
		m_speedLines->Update(m_game->GetTimeStep());

		matrix4x4d trans;
		Frame::GetFrameTransform(playerFrame, camFrame, trans);

		if ( m_speedLines.get() && Pi::AreSpeedLinesDisplayed() ) {
			m_speedLines->Update(m_game->GetTimeStep());

			trans[12] = trans[13] = trans[14] = 0.0;
			trans[15] = 1.0;
			m_speedLines->SetTransform(trans);
		}
	}

	if( Pi::AreHudTrailsDisplayed() )
		{
			matrix4x4d trans;
			Frame::GetFrameTransform(playerFrame, camFrame, trans);

			for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
				it->trail->SetTransform(trans);
		} else {
		for (auto it = Pi::player->GetSensors()->GetContacts().begin(); it != Pi::player->GetSensors()->GetContacts().end(); ++it)
			it->trail->Reset(playerFrame);
	}

	// target object under the crosshairs. must be done after
	// UpdateProjectedObjects() to be sure that m_projectedPos does not have
	// contain references to deleted objects
	if (targetObject) {
		Body* const target = PickBody(double(Gui::Screen::GetWidth())/2.0, double(Gui::Screen::GetHeight())/2.0);
		SelectBody(target, false);
	}

	UIView::Update();
}

void WorldView::BuildUI(UI::Single *container)
{
	container->SetInnerWidget(m_hudRoot.Get());
}

void WorldView::OnSwitchTo()
{
	UIView::OnSwitchTo();
	RefreshButtonStateAndVisibility();
}

void WorldView::OnSwitchFrom()
{
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

// XXX belongs in some sort of hyperspace controller
void WorldView::OnHyperspaceTargetChanged()
{
	if (Pi::player->IsHyperspaceActive()) {
		Pi::player->AbortHyperjump();
		m_game->log->Add(Lang::HYPERSPACE_JUMP_ABORTED);
	}
}

void WorldView::OnPlayerChangeTarget()
{
	Body *b = Pi::player->GetNavTarget();
	if (b) {
		Sound::PlaySfx("OK");
		Ship *s = b->IsType(Object::HYPERSPACECLOUD) ? static_cast<HyperspaceCloud*>(b)->GetShip() : 0;
		if (!s || !m_game->GetSectorView()->GetHyperspaceTarget().IsSameSystem(s->GetHyperspaceDest()))
			m_game->GetSectorView()->FloatHyperspaceTarget();
	}

}

void WorldView::SelectBody(Body *target, bool reselectIsDeselect)
{
	if (!target || target == Pi::player) return;		// don't select self
	if (target->IsType(Object::PROJECTILE)) return;

	if (target->IsType(Object::SHIP)) {
		if (Pi::player->GetCombatTarget() == target) {
			if (reselectIsDeselect) Pi::player->SetCombatTarget(0);
		} else {
			Pi::player->SetCombatTarget(target, Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL));
		}
	} else {
		if (Pi::player->GetNavTarget() == target) {
			if (reselectIsDeselect) Pi::player->SetNavTarget(0);
		} else {
			Pi::player->SetNavTarget(target, Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL));
		}
	}
}

Body* WorldView::PickBody(const double screenX, const double screenY) const
{
	for (std::map<Body*,vector3d>::const_iterator
				 i = m_projectedPos.begin(); i != m_projectedPos.end(); ++i) {
		Body *b = i->first;

		if (b == Pi::player || b->IsType(Object::PROJECTILE))
			continue;

		const double x1 = i->second.x - PICK_OBJECT_RECT_SIZE * 0.5;
		const double x2 = x1 + PICK_OBJECT_RECT_SIZE;
		const double y1 = i->second.y - PICK_OBJECT_RECT_SIZE * 0.5;
		const double y2 = y1 + PICK_OBJECT_RECT_SIZE;
		if(screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2)
			return b;
	}

	return 0;
}

int WorldView::GetActiveWeapon() const
{
	switch (GetCamType()) {
	case CAM_INTERNAL:
		return m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR ? 1 : 0;

	case CAM_EXTERNAL:
	case CAM_SIDEREAL:
	default:
		return 0;
	}
}

static inline bool project_to_screen(const vector3d &in, vector3d &out, const Graphics::Frustum &frustum, const int guiSize[2])
{
	if (!frustum.ProjectPoint(in, out)) return false;
	out.x *= guiSize[0];
	out.y = Gui::Screen::GetHeight() - out.y * guiSize[1];
	return true;
}

void WorldView::UpdateProjectedObjects()
{
	const int guiSize[2] = { Gui::Screen::GetWidth(), Gui::Screen::GetHeight() };
	const Graphics::Frustum frustum = m_cameraContext->GetFrustum();

	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	matrix3x3d cam_rot = cam_frame->GetOrient();

	// determine projected positions and update labels
	m_projectedPos.clear();
	for (Body* b : m_game->GetSpace()->GetBodies()) {
		// don't show the player label on internal camera
		if (b->IsType(Object::PLAYER) && GetCamType() == CAM_INTERNAL)
			continue;

		vector3d pos = b->GetInterpPositionRelTo(cam_frame);
		if ((pos.z < -1.0) && project_to_screen(pos, pos, frustum, guiSize)) {
			m_projectedPos[b] = pos;
		}
	}
	{
		// directional indicators, based on ship bearing
		vector3d x = Pi::player->GetOrient() * vector3d(0,0,1) * cam_rot;
		UpdateIndicator(m_backwardIndicator, x);
		UpdateIndicator(m_forwardIndicator, -x);
		x = Pi::player->GetOrient() * vector3d(0,1,0) * cam_rot;
		UpdateIndicator(m_upIndicator, x);
		UpdateIndicator(m_downIndicator, -x);
		x = Pi::player->GetOrient() * vector3d(1,0,0) * cam_rot;
		UpdateIndicator(m_rightIndicator, x);
		UpdateIndicator(m_leftIndicator, -x);
		// orbital indicators, based on player position and velocity direction
		vector3d velocity = Pi::player->GetVelocity();
		const vector3d normal = Pi::player->GetPosition().Cross(velocity).Normalized();
		UpdateIndicator(m_normalIndicator, normal * cam_rot);
		UpdateIndicator(m_antiNormalIndicator, -(normal * cam_rot));
		const vector3d radial = normal.Cross(velocity).Normalized();
		UpdateIndicator(m_radialInIndicator, radial * cam_rot);
		UpdateIndicator(m_radialOutIndicator, -(radial * cam_rot));
	}
	// velocity relative to current frame (white)
	const vector3d camSpaceVel = Pi::player->GetVelocity() * cam_rot;
	if (camSpaceVel.LengthSqr() >= 1e-4) {
		UpdateIndicator(m_velIndicator, camSpaceVel);
		UpdateIndicator(m_retroVelIndicator, -camSpaceVel);
	} else {
		HideIndicator(m_velIndicator);
		HideIndicator(m_retroVelIndicator);
	}

	const Frame* frame = Pi::player->GetFrame();
	if(frame->IsRotFrame())
		frame = frame->GetNonRotFrame();
	const SystemBody* systemBody = frame->GetSystemBody();
	if(systemBody) {
		const vector3d pos = Pi::player->GetPosition();
		UpdateIndicator(m_awayFromFrameIndicator, (pos + pos.Normalized()) * cam_rot);
	}
	if(Pi::planner->GetOffsetVel().ExactlyEqual(vector3d(0,0,0))) {
		HideIndicator(m_burnIndicator);
	} else if(systemBody) {
		Orbit playerOrbit = Pi::player->ComputeOrbit();
		if(!is_zero_exact(playerOrbit.GetSemiMajorAxis())) {
			double mass = systemBody->GetMass();
			// XXX The best solution would be to store the mass(es) on Orbit
			const vector3d camSpacePlanSpeed = (Pi::planner->GetVel() - playerOrbit.OrbitalVelocityAtTime(mass, playerOrbit.OrbitalTimeAtPos(Pi::planner->GetPosition(), mass))) * cam_rot;
			m_burnIndicator.side = INDICATOR_TOP;
			UpdateIndicator(m_burnIndicator, camSpacePlanSpeed);
		}
	}

	// orientation according to mouse
	if (Pi::player->GetPlayerController()->IsMouseActive()) {
		vector3d mouseDir = Pi::player->GetPlayerController()->GetMouseDir() * cam_rot;
		if (GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR)
			mouseDir = -mouseDir;
		UpdateIndicator(m_mouseDirIndicator, (Pi::player->GetPhysRadius() * 1.5) * mouseDir);
	} else
		HideIndicator(m_mouseDirIndicator);
	if(Pi::player && Pi::player->GetFrame() && Pi::player->GetFrame()->GetBody())
		UpdateIndicator(m_frameIndicator, Pi::player->GetFrame()->GetBody()->GetTargetIndicatorPosition(cam_frame));

	// navtarget info
	if (Body *navtarget = Pi::player->GetNavTarget()) {
		// if navtarget and body frame are the same,
		// then we hide the frame-relative velocity indicator
		// (which would be hidden underneath anyway)
		if (navtarget == Pi::player->GetFrame()->GetBody())
			HideIndicator(m_velIndicator);

		// navtarget distance/target square indicator (displayed with navtarget label)
		UpdateIndicator(m_navTargetIndicator, navtarget->GetTargetIndicatorPosition(cam_frame));


		// velocity relative to navigation target
		vector3d navvelocity = -navtarget->GetVelocityRelTo(Pi::player);
		double navspeed = navvelocity.Length();
		const vector3d camSpaceNavVel = navvelocity * cam_rot;

		if (navspeed >= 0.01) { // 1 cm per second
			char buf[128];
			if (navspeed > 1000)
				snprintf(buf, sizeof(buf), "%.2f km/s", navspeed*0.001);
			else
				snprintf(buf, sizeof(buf), "%.0f m/s", navspeed);
			UpdateIndicator(m_navVelIndicator, camSpaceNavVel);
			UpdateIndicator(m_retroNavVelIndicator, -camSpaceNavVel);

			assert(m_navTargetIndicator.side != INDICATOR_HIDDEN);
			assert(m_navVelIndicator.side != INDICATOR_HIDDEN);
		} else
			HideIndicator(m_navVelIndicator);

	} else {
		HideIndicator(m_navTargetIndicator);
		HideIndicator(m_navVelIndicator);
	}

	// later we might want non-ship enemies (e.g., for assaults on military bases)
	assert(!Pi::player->GetCombatTarget() || Pi::player->GetCombatTarget()->IsType(Object::SHIP));

	// update combat HUD
	Ship *enemy = static_cast<Ship *>(Pi::player->GetCombatTarget());
	if (enemy) {
		char buf[128];
		const vector3d targpos = enemy->GetInterpPositionRelTo(Pi::player) * cam_rot;
		const double dist = targpos.Length();
		const vector3d targScreenPos = enemy->GetInterpPositionRelTo(cam_frame);

		snprintf(buf, sizeof(buf), "%.0fm", dist);
		UpdateIndicator(m_combatTargetIndicator, targScreenPos);

		// calculate firing solution and relative velocity along our z axis
		int laser = -1;
		double projspeed = 0;
		if (GetCamType() == CAM_INTERNAL) {
			switch (m_internalCameraController->GetMode()) {
			case InternalCameraController::MODE_FRONT: laser = 0; break;
			case InternalCameraController::MODE_REAR:  laser = 1; break;
			default: break;
			}
		}

		if (laser >= 0) {
			Pi::player->Properties().Get(laser?"laser_rear_speed":"laser_front_speed", projspeed);
		}
		if (projspeed > 0) { // only display target lead position on views with lasers
			const vector3d targvel = enemy->GetVelocityRelTo(Pi::player) * cam_rot;
			vector3d leadpos = targpos + targvel*(targpos.Length()/projspeed);
			leadpos = targpos + targvel*(leadpos.Length()/projspeed); // second order approx

			// now the text speed/distance
			// want to calculate closing velocity that you couldn't counter with retros

			UpdateIndicator(m_targetLeadIndicator, leadpos);

			if ((m_targetLeadIndicator.side != INDICATOR_ONSCREEN) || (m_combatTargetIndicator.side != INDICATOR_ONSCREEN))
				HideIndicator(m_targetLeadIndicator);

		} else
			HideIndicator(m_targetLeadIndicator);
	} else {
		HideIndicator(m_combatTargetIndicator);
		HideIndicator(m_targetLeadIndicator);
	}
}

void WorldView::UpdateIndicator(Indicator &indicator, const vector3d &cameraSpacePos)
{
	const int guiSize[2] = { Gui::Screen::GetWidth(), Gui::Screen::GetHeight() };
	const Graphics::Frustum frustum = m_cameraContext->GetFrustum();

	const float BORDER = 10.0;
	const float BORDER_BOTTOM = 10.0;
	// XXX BORDER_BOTTOM is 10+the control panel height and shouldn't be needed at all

	const float w = Gui::Screen::GetWidth();
	const float h = Gui::Screen::GetHeight();

	if (cameraSpacePos.LengthSqr() < 1e-6) { // length < 1e-3
		indicator.pos.x = w/2.0f;
		indicator.pos.y = h/2.0f;
		indicator.side = INDICATOR_ONSCREEN;
	} else {
		vector3d proj;
		bool success = project_to_screen(cameraSpacePos, proj, frustum, guiSize);
		if (! success)
			proj = vector3d(w/2.0, h/2.0, 0.0);

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
			const vector3d ptCentre(w/2.0, h/2.0, 1.0);
			const vector3d ptProj(proj.x, proj.y, 1.0);
			const vector3d lnDir = ptProj.Cross(ptCentre);

			indicator.side = INDICATOR_TOP;

			// this fallback is used if direction is close to (0, 0, +ve)
			indicator.pos.x = w/2.0;
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
				vector3d ptRight = lnDir.Cross(vector3d(-1.0, 0.0,  w - BORDER));
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
				if (ptBottom.x >= BORDER && ptBottom.x < w-BORDER) {
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

	// update the label position
	if (indicator.label) {
		if (indicator.side != INDICATOR_HIDDEN) {
			float labelSize[2] = { 500.0f, 500.0f };
			indicator.label->GetSizeRequested(labelSize);

			int pos[2] = {0,0};
			switch (indicator.side) {
			case INDICATOR_HIDDEN: break;
			case INDICATOR_ONSCREEN: // when onscreen, default to label-below unless it would clamp to be on top of the marker
				pos[0] = -(labelSize[0]/2.0f);
				if (indicator.pos.y + pos[1] + labelSize[1] + HUD_CROSSHAIR_SIZE + 2.0f > h - BORDER_BOTTOM)
					pos[1] = -(labelSize[1] + HUD_CROSSHAIR_SIZE + 2.0f);
				else
					pos[1] = HUD_CROSSHAIR_SIZE + 2.0f;
				break;
			case INDICATOR_TOP:
				pos[0] = -(labelSize[0]/2.0f);
				pos[1] = HUD_CROSSHAIR_SIZE + 2.0f;
				break;
			case INDICATOR_LEFT:
				pos[0] = HUD_CROSSHAIR_SIZE + 2.0f;
				pos[1] = -(labelSize[1]/2.0f);
				break;
			case INDICATOR_RIGHT:
				pos[0] = -(labelSize[0] + HUD_CROSSHAIR_SIZE + 2.0f);
				pos[1] = -(labelSize[1]/2.0f);
				break;
			case INDICATOR_BOTTOM:
				pos[0] = -(labelSize[0]/2.0f);
				pos[1] = -(labelSize[1] + HUD_CROSSHAIR_SIZE + 2.0f);
				break;
			}

			pos[0] = Clamp(pos[0] + indicator.pos.x, BORDER, w - BORDER - labelSize[0]);
			pos[1] = Clamp(pos[1] + indicator.pos.y, BORDER, h - BORDER_BOTTOM - labelSize[1]);
			MoveChild(indicator.label, pos[0], pos[1]);
			indicator.label->Show();
		} else {
			indicator.label->Hide();
		}
	}
}

void WorldView::HideIndicator(Indicator &indicator)
{
	indicator.side = INDICATOR_HIDDEN;
	indicator.pos = vector2f(0.0f, 0.0f);
	if (indicator.label)
		indicator.label->Hide();
}

double getSquareDistance(double initialDist, double scalingFactor, int num) {
	return pow(scalingFactor, num - 1) * num * initialDist;
}

double getSquareHeight(double distance, double angle) {
	return distance * tan(angle);
}

void WorldView::Draw()
{
	assert(m_game);
	assert(Pi::player);

	m_renderer->ClearDepthBuffer();

	View::Draw();

	// don't draw crosshairs etc in hyperspace
	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) return;

	m_renderer->CheckRenderErrors(__FUNCTION__,__LINE__);

}

void WorldView::MouseWheel(bool up)
{
	if (this == Pi::GetView())
		{
			if (m_activeCameraController->IsExternal()) {
				MoveableCameraController *cam = static_cast<MoveableCameraController*>(m_activeCameraController);

				if (!up)	// Zoom out
					cam->ZoomEvent( ZOOM_SPEED * WHEEL_SENSITIVITY);
				else
					cam->ZoomEvent(-ZOOM_SPEED * WHEEL_SENSITIVITY);
			}
		}
}
NavTunnelWidget::NavTunnelWidget(WorldView *worldview, Graphics::RenderState *rs)
	: Widget()
	, m_worldView(worldview)
	, m_renderState(rs)
{
}

void NavTunnelWidget::Draw() {
	if (!Pi::IsNavTunnelDisplayed()) return;

	Body *navtarget = Pi::player->GetNavTarget();
	if (navtarget) {
		const vector3d navpos = navtarget->GetPositionRelTo(Pi::player);
		const matrix3x3d &rotmat = Pi::player->GetOrient();
		const vector3d eyevec = rotmat * m_worldView->m_activeCameraController->GetOrient().VectorZ();
		if (eyevec.Dot(navpos) >= 0.0) return;

		const double distToDest = Pi::player->GetPositionRelTo(navtarget).Length();

		const int maxSquareHeight = std::max(Gui::Screen::GetWidth(), Gui::Screen::GetHeight()) / 2;
		const double angle = atan(maxSquareHeight / distToDest);
		const vector2f tpos(m_worldView->m_navTargetIndicator.realpos);
		const vector2f distDiff(tpos - vector2f(Gui::Screen::GetWidth() / 2.0f, Gui::Screen::GetHeight() / 2.0f));

		double dist = 0.0;
		const double scalingFactor = 1.6; // scales distance between squares: closer to 1.0, more squares
		for (int squareNum = 1; ; squareNum++) {
			dist = getSquareDistance(10.0, scalingFactor, squareNum);
			if (dist > distToDest)
				break;

			const double sqh = getSquareHeight(dist, angle);
			if (sqh >= 10) {
				const vector2f off = distDiff * (dist / distToDest);
				const vector2f sqpos(tpos-off);
				DrawTargetGuideSquare(sqpos, sqh, green);
			}
		}
	}
}

void NavTunnelWidget::DrawTargetGuideSquare(const vector2f &pos, const float size, const Color &c)
{
	const float x1 = pos.x - size;
	const float x2 = pos.x + size;
	const float y1 = pos.y - size;
	const float y2 = pos.y + size;

	Color black(c);
	black.a = c.a / 6;
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, 8);
	va.Add(vector3f(x1,    y1,    0.f),	c);
	va.Add(vector3f(pos.x, y1,    0.f),	black);
	va.Add(vector3f(x2,    y1,    0.f),	c);
	va.Add(vector3f(x2,    pos.y, 0.f),	black);
	va.Add(vector3f(x2,    y2,    0.f),	c);
	va.Add(vector3f(pos.x, y2,    0.f),	black);
	va.Add(vector3f(x1,    y2,    0.f),	c);
	va.Add(vector3f(x1,    pos.y, 0.f),	black);

	if( !m_vbuffer.get() ) {
		CreateVertexBuffer( 8 );
	}

	m_vbuffer->Populate( va );

	m_worldView->m_renderer->DrawBuffer(m_vbuffer.get(), m_renderState, m_material.Get(), Graphics::LINE_LOOP);
}

void NavTunnelWidget::GetSizeRequested(float size[2]) {
	size[0] = Gui::Screen::GetWidth();
	size[1] = Gui::Screen::GetHeight();
}

void NavTunnelWidget::CreateVertexBuffer(const Uint32 size)
{
	Graphics::Renderer *r = m_worldView->m_renderer;

	Graphics::MaterialDescriptor desc;
	desc.vertexColors = true;
	m_material.Reset(r->CreateMaterial(desc));

	Graphics::VertexBufferDesc vbd;
	vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
	vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
	vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
	vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;
	vbd.numVertices = size;
	m_vbuffer.reset(r->CreateVertexBuffer(vbd));
}

// project vector vec onto plane (normal must be normalized)
static vector3d projectVecOntoPlane(const vector3d &vec, const vector3d &normal) {
	return (vec - vec.Dot(normal)*normal);
}

static double wrapAngleToPositive(const double theta) {
	return (theta >= 0.0 ? theta : M_PI*2 + theta);
}

/*
  heading range: 0 - 359 deg
  heading  0 - north
  heading 90 - east
  pitch range: -90 - +90 deg
  pitch  0 - level with surface
  pitch 90 - up
*/
static std::pair<double, double> calculateHeadingPitch(PlaneType pt) {
	auto frame  = Pi::player->GetFrame();

	if(pt == ROTATIONAL)
		frame = frame->GetRotFrame();
	else if (pt == PARENT)
		frame = frame->GetNonRotFrame();

	// construct a frame of reference aligned with the ground plane
	// and with lines of longitude and latitude
	const vector3d up = Pi::player->GetPositionRelTo(frame).NormalizedSafe();
	const vector3d north = projectVecOntoPlane(vector3d(0,1,0), up).NormalizedSafe();
	const vector3d east = north.Cross(up);

	// find the direction that the ship is facing
	const auto shpRot = Pi::player->GetOrientRelTo(frame);
	const vector3d hed = -shpRot.VectorZ();
	const vector3d groundHed = projectVecOntoPlane(hed, up).NormalizedSafe();

	const double pitch = asin(up.Dot(hed));

	const double hedNorth = groundHed.Dot(north);
	const double hedEast = groundHed.Dot(east);
	const double heading = wrapAngleToPositive(atan2(hedEast, hedNorth));

	return std::make_pair(
		std::isnan(heading) ? 0.0 : heading,
		std::isnan(pitch) ? 0.0 : pitch);
}

const vector3d WorldView::GetNavProgradeVelocity() const {
	Body *navTarget = Pi::player->GetNavTarget();
	if(navTarget)
		return Pi::player->GetVelocityRelTo(navTarget);
	else
		return vector3d(0,0,0); }

const vector3d WorldView::GetFrameProgradeVelocity() const {
	Frame *frame = Pi::player->GetFrame();
	if(frame)
		return Pi::player->GetVelocityRelTo(frame);
	else
		return vector3d(0,0,0);
}
