// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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

static const float LOW_THRUST_LEVELS[] = { 0.75, 0.5, 0.25, 0.1, 0.05, 0.01 };

void WorldView::InitObject()
{
	float size[2];
	GetSizeRequested(size);

	m_showTargetActionsTimeout = 0;
	m_showLowThrustPowerTimeout = 0;
	m_labelsOn = true;
	SetTransparency(true);

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	rsd.depthTest = false;
	m_blendState = Pi::renderer->CreateRenderState(rsd); //XXX m_renderer not set yet
	m_navTunnel = new NavTunnelWidget(this, m_blendState);
	Add(m_navTunnel, 0, 0);

	m_commsOptions = new Fixed(size[0], size[1]/1.8);
	m_commsOptions->SetTransparency(true);
	Add(m_commsOptions, 10, 150);

	m_commsNavOptionsContainer = new Gui::HBox();
	m_commsNavOptionsContainer->SetSpacing(5);
	m_commsNavOptionsContainer->SetSizeRequest(220, size[1]-50);
	Add(m_commsNavOptionsContainer, size[0]-230, 20);

	Gui::VScrollPortal *portal = new Gui::VScrollPortal(200);
	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	scroll->SetAdjustment(&portal->vscrollAdjust);
	m_commsNavOptionsContainer->PackStart(scroll);
	m_commsNavOptionsContainer->PackStart(portal);

	m_commsNavOptions = new Gui::VBox();
	m_commsNavOptions->SetSpacing(5);
	portal->Add(m_commsNavOptions);

	m_lowThrustPowerOptions = new Gui::Fixed(size[0], size[1]/2);
	m_lowThrustPowerOptions->SetTransparency(true);
	Add(m_lowThrustPowerOptions, 10, 200);
	for (int i = 0; i < int(COUNTOF(LOW_THRUST_LEVELS)); ++i) {
		assert(i < 9); // otherwise the shortcuts break
		const int ypos = i*32;

		Gui::Label *label = new Gui::Label(
																			 stringf(Lang::SET_LOW_THRUST_POWER_LEVEL_TO_X_PERCENT,
																							 formatarg("power", 100.0f * LOW_THRUST_LEVELS[i], "f.0")));
		m_lowThrustPowerOptions->Add(label, 50, float(ypos));

		char buf[8];
		snprintf(buf, sizeof(buf), "%d", (i + 1));
		Gui::Button *btn = new Gui::LabelButton(new Gui::Label(buf));
		btn->SetShortcut(SDL_Keycode(SDLK_1 + i), KMOD_NONE);
		m_lowThrustPowerOptions->Add(btn, 16, float(ypos));

		btn->onClick.connect(sigc::bind(sigc::mem_fun(this, &WorldView::OnSelectLowThrustPower), LOW_THRUST_LEVELS[i]));
	}

#if WITH_DEVKEYS
	Gui::Screen::PushFont("ConsoleFont");
	m_debugInfo = (new Gui::Label(""))->Color(204, 204, 204);
	Add(m_debugInfo, 10, 200);
	Gui::Screen::PopFont();
#endif
	/*
	  NEW UI
	*/

	// --

	m_hudSensorGaugeStack = new Gui::VBox();
	m_hudSensorGaugeStack->SetSpacing(2.0f);
	Add(m_hudSensorGaugeStack, 5.0f, 5.0f);

	Gui::Screen::PushFont("OverlayFont");

	{
		m_pauseText = new Gui::Label(std::string("#f7f") + Lang::PAUSED);
		float w, h;
		Gui::Screen::MeasureString(Lang::PAUSED, w, h);
		Add(m_pauseText, 0.5f * (Gui::Screen::GetWidth() - w), 100);
	}
	Gui::Screen::PopFont();

	m_combatTargetIndicator.label = new Gui::Label(""); // colour set dynamically
	m_targetLeadIndicator.label = new Gui::Label("");

	// these labels are repositioned during Draw3D()
	Add(m_combatTargetIndicator.label, 0, 0);
	Add(m_targetLeadIndicator.label, 0, 0);

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

	m_onPlayerChangeTargetCon =
		Pi::onPlayerChangeTarget.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeTarget));
	m_onMouseWheelCon =
		Pi::onMouseWheel.connect(sigc::mem_fun(this, &WorldView::MouseWheel));

	Pi::player->GetPlayerController()->SetMouseForRearView(GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
	m_onToggleHudModeCon = KeyBindings::toggleHudMode.onPress.connect(sigc::mem_fun(this, &WorldView::OnToggleLabels));
	m_onIncTimeAccelCon = KeyBindings::increaseTimeAcceleration.onPress.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelInc));
	m_onDecTimeAccelCon = KeyBindings::decreaseTimeAcceleration.onPress.connect(sigc::mem_fun(this, &WorldView::OnRequestTimeAccelDec));
}

WorldView::~WorldView()
{
	m_onPlayerChangeTargetCon.disconnect();
	m_onMouseWheelCon.disconnect();
	m_onToggleHudModeCon.disconnect();
	m_onIncTimeAccelCon.disconnect();
	m_onDecTimeAccelCon.disconnect();
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

/* This is UI click to change flight control state (manual, speed ctrl) */
void WorldView::ChangeFlightState()
{
	Pi::BoinkNoise();
	static int newState = CONTROL_MANUAL;
	if (Pi::KeyState(SDLK_LCTRL) || Pi::KeyState(SDLK_RCTRL)) {
		// skip certain states
		switch (newState) {
		case CONTROL_MANUAL: // fallthrough
		case CONTROL_FIXSPEED: newState = CONTROL_FIXHEADING_FORWARD; break;
		case CONTROL_FIXHEADING_FORWARD: // fallthrough
		case CONTROL_FIXHEADING_BACKWARD: // fallthrough
		case CONTROL_AUTOPILOT: newState = CONTROL_MANUAL; break;
		default: break;
		}
	} else {
		// skip certain states
		switch (newState) {
		case CONTROL_FIXHEADING_FORWARD: // fallthrough
		case CONTROL_FIXSPEED: // fallthrough
		case CONTROL_FIXHEADING_BACKWARD: // fallthrough
		case CONTROL_AUTOPILOT: newState = CONTROL_MANUAL; break;
		case CONTROL_MANUAL: newState = CONTROL_FIXSPEED; break;
		default: break;
		}
	}
	Pi::player->GetPlayerController()->SetFlightControlState(static_cast<FlightControlState>(newState));
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

void WorldView::RefreshButtonStateAndVisibility()
{
	assert(m_game);
	assert(Pi::player);
	assert(!Pi::player->IsDead());

	if (m_game->IsPaused())
		m_pauseText->Show();
	else
		m_pauseText->Hide();

	if (m_showTargetActionsTimeout) {
		if (SDL_GetTicks() - m_showTargetActionsTimeout > 20000) {
			m_showTargetActionsTimeout = 0;
			m_commsOptions->DeleteAllChildren();
			m_commsNavOptions->DeleteAllChildren();
		}
		m_commsOptions->ShowAll();
		m_commsNavOptionsContainer->ShowAll();
	} else {
		m_commsOptions->Hide();
		m_commsNavOptionsContainer->Hide();
	}

	if (m_showLowThrustPowerTimeout) {
		if (SDL_GetTicks() - m_showLowThrustPowerTimeout > 20000) {
			m_showLowThrustPowerTimeout = 0;
		}
		m_lowThrustPowerOptions->Show();
	} else {
		m_lowThrustPowerOptions->Hide();
	}
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

	UIView::Update();
}

void WorldView::BuildUI(UI::Single *container)
{
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

void WorldView::ToggleTargetActions()
{
	if (m_game->IsHyperspace() || m_showTargetActionsTimeout)
		HideTargetActions();
	else
		ShowTargetActions();
}

void WorldView::ShowTargetActions()
{
	m_showTargetActionsTimeout = SDL_GetTicks();
	UpdateCommsOptions();
	HideLowThrustPowerOptions();
}

void WorldView::HideTargetActions()
{
	m_showTargetActionsTimeout = 0;
	UpdateCommsOptions();
}

Gui::Button *WorldView::AddCommsOption(const std::string &msg, int ypos, int xoffset, int optnum)
{
	Gui::Label *l = new Gui::Label(msg);
	m_commsOptions->Add(l, 50 + xoffset, float(ypos));

	char buf[8];
	snprintf(buf, sizeof(buf), "%d", optnum);
	Gui::LabelButton *b = new Gui::LabelButton(new Gui::Label(buf));
	b->SetShortcut(SDL_Keycode(SDLK_0 + optnum), KMOD_NONE);
	// hide target actions when things get clicked on
	b->onClick.connect(sigc::mem_fun(this, &WorldView::ToggleTargetActions));
	m_commsOptions->Add(b, 16 + xoffset, float(ypos));
	return b;
}

void WorldView::OnClickCommsNavOption(Body *target)
{
	Pi::player->SetNavTarget(target);
	m_showTargetActionsTimeout = SDL_GetTicks();
	HideLowThrustPowerOptions();
}

void WorldView::AddCommsNavOption(const std::string &msg, Body *target)
{
	Gui::HBox *hbox = new Gui::HBox();
	hbox->SetSpacing(5);

	Gui::Label *l = new Gui::Label(msg);
	hbox->PackStart(l);

	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &WorldView::OnClickCommsNavOption), target));
	hbox->PackStart(b);

	m_commsNavOptions->PackEnd(hbox);
}

void WorldView::BuildCommsNavOptions()
{
	std::map< Uint32,std::vector<SystemBody*> > groups;

	m_commsNavOptions->PackEnd(new Gui::Label(std::string("#ff0")+std::string(Lang::NAVIGATION_TARGETS_IN_THIS_SYSTEM)+std::string("\n")));

	for (SystemBody* station : m_game->GetSpace()->GetStarSystem()->GetSpaceStations()) {
		groups[station->GetParent()->GetPath().bodyIndex].push_back(station);
	}

	for ( std::map< Uint32,std::vector<SystemBody*> >::const_iterator i = groups.begin(); i != groups.end(); ++i ) {
		m_commsNavOptions->PackEnd(new Gui::Label("#f0f" + m_game->GetSpace()->GetStarSystem()->GetBodies()[(*i).first]->GetName()));

		for ( std::vector<SystemBody*>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
			SystemPath path = m_game->GetSpace()->GetStarSystem()->GetPathOf(*j);
			Body *body = m_game->GetSpace()->FindBodyForPath(&path);
			AddCommsNavOption((*j)->GetName(), body);
		}
	}
}

void WorldView::HideLowThrustPowerOptions()
{
	m_showLowThrustPowerTimeout = 0;
	m_lowThrustPowerOptions->Hide();
}

void WorldView::ShowLowThrustPowerOptions()
{
	m_showLowThrustPowerTimeout = SDL_GetTicks();
	m_lowThrustPowerOptions->Show();
	HideTargetActions();
}

void WorldView::ToggleLowThrustPowerOptions()
{
	Pi::BoinkNoise();
	if (m_showLowThrustPowerTimeout)
		HideLowThrustPowerOptions();
	else
		ShowLowThrustPowerOptions();
}

void WorldView::OnSelectLowThrustPower(float power)
{
	Pi::player->GetPlayerController()->SetLowThrustPower(power);
	HideLowThrustPowerOptions();
}

static void PlayerRequestDockingClearance(SpaceStation *s)
{
	std::string msg;
	s->GetDockingClearance(Pi::player, msg);
	Pi::game->log->Add(s->GetLabel(), msg, GameLog::Priority::PRIORITY_NORMAL);
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

void WorldView::OnPlayerChangeTarget()
{
	Body *b = Pi::player->GetNavTarget();
	if (b) {
		Sound::PlaySfx("OK");
		Ship *s = b->IsType(Object::HYPERSPACECLOUD) ? static_cast<HyperspaceCloud*>(b)->GetShip() : 0;
		if (!s || !m_game->GetSectorView()->GetHyperspaceTarget().IsSameSystem(s->GetHyperspaceDest()))
			m_game->GetSectorView()->FloatHyperspaceTarget();
	}

	UpdateCommsOptions();
}

static void autopilot_flyto(Body *b)
{
	Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
	Pi::player->AIFlyTo(b);
}
static void autopilot_dock(Body *b)
{
	if(Pi::player->GetFlightState() != Ship::FLYING)
		return;

	Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
	Pi::player->AIDock(static_cast<SpaceStation*>(b));
}
static void autopilot_orbit(Body *b, double alt)
{
	Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
	Pi::player->AIOrbit(b, alt);
}

static void player_target_hypercloud(HyperspaceCloud *cloud)
{
	Pi::game->GetSectorView()->SetHyperspaceTarget(cloud->GetShip()->GetHyperspaceDest());
}

static void OnCommsSelectAttitude(FlightControlState s) {
	Pi::player->GetPlayerController()->SetFlightControlState(s);
}

void WorldView::UpdateCommsOptions()
{
	m_commsOptions->DeleteAllChildren();
	m_commsNavOptions->DeleteAllChildren();

	if (m_showTargetActionsTimeout == 0) return;

	if (m_game->GetSpace()->GetStarSystem()->HasSpaceStations())
	{
		BuildCommsNavOptions();
	}

	Body * const navtarget = Pi::player->GetNavTarget();
	Body * const comtarget = Pi::player->GetCombatTarget();
	Gui::Button *button;
	int ypos = 0;
	int optnum = 1;
	if (!(navtarget || comtarget)) {
		m_commsOptions->Add(new Gui::Label("#0f0"+std::string(Lang::NO_TARGET_SELECTED)), 16, float(ypos));
		ypos += 32;
	}

	int hasAutopilot = 0;
	Pi::player->Properties().Get("autopilot_cap", hasAutopilot);
	hasAutopilot = hasAutopilot && (Pi::player->GetFlightState() == Ship::FLYING);

	if (navtarget) {
		m_commsOptions->Add(new Gui::Label("#0f0"+navtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		if (navtarget->IsType(Object::SPACESTATION)) {
			SpaceStation *pStation = static_cast<SpaceStation *>(navtarget);
			if( pStation->GetMyDockingPort(Pi::player) == -1 )
			{
				button = AddCommsOption(Lang::REQUEST_DOCKING_CLEARANCE, ypos, 0, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(&PlayerRequestDockingClearance), static_cast<SpaceStation*>(navtarget)));
				ypos += 32;
			}

			if( hasAutopilot )
			{
				button = AddCommsOption(Lang::AUTOPILOT_DOCK_WITH_STATION, ypos, 0, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_dock), navtarget));
				ypos += 32;
			}

			// XXX paying fine remotely can't really be done until crime and
			// worldview are in Lua. I'm leaving this code here so its not
			// forgotten
			/*
			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);
			if (fine) {
				button = AddCommsOption(stringf(Lang::PAY_FINE_REMOTELY,
							formatarg("amount", format_money(fine))), ypos, optnum++);
				button->onClick.connect(sigc::ptr_fun(&PlayerPayFine));
				ypos += 32;
			}
			*/
		}
		if (hasAutopilot) {
			button = AddCommsOption(stringf(Lang::AUTOPILOT_FLY_TO_VICINITY_OF, formatarg("target", navtarget->GetLabel())), ypos, 0, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_flyto), navtarget));
			ypos += 32;

			if (navtarget->IsType(Object::PLANET) || navtarget->IsType(Object::STAR)) {
				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_LOW_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, 0, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 1.2));
				ypos += 32;

				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, 0,  optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 1.6));
				ypos += 32;

				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_HIGH_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, 0, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 3.2));
				ypos += 32;
			}
		}

		int analyzer = 0;
		Pi::player->Properties().Get("hypercloud_analyzer_cap", analyzer);
		if (analyzer && navtarget->IsType(Object::HYPERSPACECLOUD)) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(navtarget);
			if (!cloud->IsArrival()) {
				button = AddCommsOption(Lang::SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE, ypos, 0, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(player_target_hypercloud), cloud));
				ypos += 32;
			}
		}
	}
	if (comtarget && hasAutopilot) {
		m_commsOptions->Add(new Gui::Label("#f00"+comtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		button = AddCommsOption(stringf(Lang::AUTOPILOT_FLY_TO_VICINITY_OF, formatarg("target", comtarget->GetLabel())), ypos, 0, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_flyto), comtarget));
		ypos += 32;
	}

	if (Pi::player->GetFlightState() == Ship::FLYING) {
		button = AddCommsOption(Lang::HEADING_LOCK_FORWARD, ypos, 0, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_FORWARD));

		button = AddCommsOption(Lang::HEADING_LOCK_BACKWARD, ypos, 180, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_BACKWARD));
		ypos += 32;

		button = AddCommsOption(Lang::HEADING_LOCK_NORMAL, ypos, 0, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_NORMAL));

		button = AddCommsOption(Lang::HEADING_LOCK_ANTINORMAL, ypos, 180, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_ANTINORMAL));
		ypos += 32;

		button = AddCommsOption(Lang::HEADING_LOCK_RADIALLY_INWARD, ypos, 0, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_RADIALLY_INWARD));

		button = AddCommsOption(Lang::HEADING_LOCK_RADIALLY_OUTWARD, ypos, 180, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(OnCommsSelectAttitude), CONTROL_FIXHEADING_RADIALLY_OUTWARD));
		ypos += 32;
	}
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
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	matrix3x3d cam_rot = cam_frame->GetOrient();

	// later we might want non-ship enemies (e.g., for assaults on military bases)
	assert(!Pi::player->GetCombatTarget() || Pi::player->GetCombatTarget()->IsType(Object::SHIP));

	// update combat HUD
	Ship *enemy = static_cast<Ship *>(Pi::player->GetCombatTarget());
	if (enemy) {
		const vector3d targpos = enemy->GetInterpPositionRelTo(Pi::player) * cam_rot;
		const double dist = targpos.Length();
		const vector3d targScreenPos = enemy->GetInterpPositionRelTo(cam_frame);

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

			double vel = targvel.Dot(targpos.NormalizedSafe()); // position should be towards
			double raccel =
				Pi::player->GetShipType()->linThrust[Thruster::THRUSTER_REVERSE] / Pi::player->GetMass();

			double c = Clamp(vel / sqrt(2.0 * raccel * dist), -1.0, 1.0);
			float r = float(0.2+(c+1.0)*0.4);
			float b = float(0.2+(1.0-c)*0.4);

			m_combatTargetIndicator.label->Color(r*255, 0, b*255);
			m_targetLeadIndicator.label->Color(r*255, 0, b*255);

			UpdateIndicator(m_targetLeadIndicator, leadpos);

			if ((m_targetLeadIndicator.side != INDICATOR_ONSCREEN) || (m_combatTargetIndicator.side != INDICATOR_ONSCREEN))
				HideIndicator(m_targetLeadIndicator);

			// if the lead indicator is very close to the position indicator
			// try (just a little) to keep the labels from interfering with one another
			if (m_targetLeadIndicator.side == INDICATOR_ONSCREEN) {
				assert(m_combatTargetIndicator.side == INDICATOR_ONSCREEN);
				SeparateLabels(m_combatTargetIndicator.label, m_targetLeadIndicator.label);
			}
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
	const float BORDER_BOTTOM = 90.0;
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

void WorldView::SeparateLabels(Gui::Label *a, Gui::Label *b)
{
	float posa[2], posb[2], sizea[2], sizeb[2];
	GetChildPosition(a, posa);
	a->GetSize(sizea);
	sizea[0] *= 0.5f;
	sizea[1] *= 0.5f;
	posa[0] += sizea[0];
	posa[1] += sizea[1];
	GetChildPosition(b, posb);
	b->GetSize(sizeb);
	sizeb[0] *= 0.5f;
	sizeb[1] *= 0.5f;
	posb[0] += sizeb[0];
	posb[1] += sizeb[1];

	float overlapX = sizea[0] + sizeb[0] - fabs(posa[0] - posb[0]);
	float overlapY = sizea[1] + sizeb[1] - fabs(posa[1] - posb[1]);

	if (overlapX > 0.0f && overlapY > 0.0f) {
		if (overlapX <= 4.0f) {
			// small horizontal overlap; bump horizontally
			if (posa[0] > posb[0]) overlapX *= -1.0f;
			MoveChild(a, posa[0] - overlapX*0.5f - sizea[0], posa[1] - sizea[1]);
			MoveChild(b, posb[0] + overlapX*0.5f - sizeb[0], posb[1] - sizeb[1]);
		} else {
			// large horizonal overlap; bump vertically
			if (posa[1] > posb[1]) overlapY *= -1.0f;
			MoveChild(a, posa[0] - sizea[0], posa[1] - overlapY*0.5f - sizea[1]);
			MoveChild(b, posb[0] - sizeb[0], posb[1] + overlapY*0.5f - sizeb[1]);
		}
	}
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

	// glLineWidth(2.0f);

	// glLineWidth(1.0f);

	// glLineWidth(2.0f);

	// combat target indicator
	DrawCombatTargetIndicator(m_combatTargetIndicator, m_targetLeadIndicator, red);

	// glLineWidth(1.0f);
	m_renderer->CheckRenderErrors(__FUNCTION__,__LINE__);
}

void WorldView::DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c)
{
	if (target.side == INDICATOR_HIDDEN) return;

	if (target.side == INDICATOR_ONSCREEN) {
		const float x1 = target.pos.x, y1 = target.pos.y;
		const float x2 = lead.pos.x, y2 = lead.pos.y;

		float xd = x2 - x1, yd = y2 - y1;
		if (lead.side != INDICATOR_ONSCREEN) {
			xd = 1.0f; yd = 0.0f;
		} else {
			float len = xd*xd + yd*yd;
			if (len < 1e-6) {
				xd = 1.0f; yd = 0.0f;
			} else {
				len = sqrt(len);
				xd /= len;
				yd /= len;
			}
		}

		const vector3f vts[] = {
			// target crosshairs
			vector3f(x1+10*xd, y1+10*yd, 0.0f),
			vector3f(x1+20*xd, y1+20*yd, 0.0f),
			vector3f(x1-10*xd, y1-10*yd, 0.0f),
			vector3f(x1-20*xd, y1-20*yd, 0.0f),
			vector3f(x1-10*yd, y1+10*xd, 0.0f),
			vector3f(x1-20*yd, y1+20*xd, 0.0f),
			vector3f(x1+10*yd, y1-10*xd, 0.0f),
			vector3f(x1+20*yd, y1-20*xd, 0.0f),

			// lead crosshairs
			vector3f(x2-10*xd, y2-10*yd, 0.0f),
			vector3f(x2+10*xd, y2+10*yd, 0.0f),
			vector3f(x2-10*yd, y2+10*xd, 0.0f),
			vector3f(x2+10*yd, y2-10*xd, 0.0f),

			// line between crosshairs
			vector3f(x1+20*xd, y1+20*yd, 0.0f),
			vector3f(x2-10*xd, y2-10*yd, 0.0f)
		};
		if (lead.side == INDICATOR_ONSCREEN) {
			m_indicator.SetData(14, vts, c);
		} else {
			m_indicator.SetData(8, vts, c);
		}
		m_indicator.Draw(m_renderer, m_blendState);
	} else {
		DrawEdgeMarker(target, c);
	}
}

void WorldView::DrawEdgeMarker(const Indicator &marker, const Color &c)
{
	const vector2f screenCentre(Gui::Screen::GetWidth()/2.0f, Gui::Screen::GetHeight()/2.0f);
	vector2f dir = screenCentre - marker.pos;
	float len = dir.Length();
	dir *= HUD_CROSSHAIR_SIZE/len;
	m_edgeMarker.SetColor(c);
	m_edgeMarker.SetStart(vector3f(marker.pos, 0.0f));
	m_edgeMarker.SetEnd(vector3f(marker.pos + dir, 0.0f));
	m_edgeMarker.Draw(m_renderer, m_blendState);
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
		// ECRAVEN: TODO not the ideal way to handle Begin/EndCameraFrame here :-/
		m_worldView->BeginCameraFrame();
		const vector3d nav_screen = m_worldView->WorldSpaceToScreenSpace(navtarget);
		m_worldView->EndCameraFrame();
		const vector2f tpos(vector2f(nav_screen.x / Graphics::GetScreenWidth() * Gui::Screen::GetWidth(), nav_screen.y / Graphics::GetScreenHeight() * Gui::Screen::GetHeight()));
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
std::tuple<double, double, double> WorldView::CalculateHeadingPitchRoll(PlaneType pt) {
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

static vector3d projectToScreenSpace(vector3d pos, RefCountedPtr<CameraContext> cameraContext, bool adjustZ = true) {
	const Graphics::Frustum frustum = cameraContext->GetFrustum();
	const float h = Graphics::GetScreenHeight();
	const float w = Graphics::GetScreenWidth();
	vector3d proj;
	if (!frustum.ProjectPoint(pos, proj)) {
		return vector3d(w / 2, h / 2, 0);
	}
	proj.x *= w;
	proj.y = h - proj.y * h;
	// set z to -1 if in front of camera, 1 else
	if(adjustZ)
		proj.z = pos.z < 0 ? -1 : 1;
	return proj;
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::WorldSpaceToScreenSpace(Body *body) const {
	if (body->IsType(Object::PLAYER) && GetCamType() == CAM_INTERNAL)
		return vector3d(0, 0, 0);
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	vector3d pos = body->GetInterpPositionRelTo(cam_frame);
	return projectToScreenSpace(pos, m_cameraContext);
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::WorldSpaceToScreenSpace(vector3d position) const {
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	matrix3x3d cam_rot = cam_frame->GetInterpOrient();
	vector3d pos = position * cam_rot;
	return projectToScreenSpace(pos, m_cameraContext);
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::ShipSpaceToScreenSpace(vector3d pos) const {
	matrix3x3d orient = Pi::player->GetInterpOrient();
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	matrix3x3d cam_rot = cam_frame->GetInterpOrient();
	vector3d camspace = orient * pos * cam_rot;
	return projectToScreenSpace(camspace, m_cameraContext, false);
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::CameraSpaceToScreenSpace(vector3d pos) const {
	return projectToScreenSpace(pos, m_cameraContext);
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::GetTargetIndicatorScreenPosition(Body *body) const {
	if (body->IsType(Object::PLAYER) && GetCamType() == CAM_INTERNAL)
		return vector3d(0, 0, 0);
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	vector3d pos = body->GetTargetIndicatorPosition(cam_frame);
	return projectToScreenSpace(pos, m_cameraContext);
}

// needs to run inside m_cameraContext->Begin/EndFrame();
vector3d WorldView::GetMouseDirection() const {
	// orientation according to mouse
	const Frame *cam_frame = m_cameraContext->GetCamFrame();
	matrix3x3d cam_rot = cam_frame->GetInterpOrient();
	vector3d mouseDir = Pi::player->GetPlayerController()->GetMouseDir() * cam_rot;
	if (GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR)
		mouseDir = -mouseDir;
	return (Pi::player->GetPhysRadius() * 1.5) * mouseDir;
}
