#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Planet.h"
#include "Space.h"
#include "SpaceStation.h"
#include "ShipCpanel.h"
#include "Serializer.h"
#include "StarSystem.h"
#include "Sector.h"
#include "HyperspaceCloud.h"
#include "KeyBindings.h"
#include "perlin.h"
#include "SectorView.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/TextureBuilder.h"
#include "matrix4x4.h"

static const std::string indicatorMousedirTextureFilename("icons/indicator_mousedir.png");

const double WorldView::PICK_OBJECT_RECT_SIZE = 20.0;
static const Color s_hudTextColor(0.0f,1.0f,0.0f,0.9f);

#define HUD_CROSSHAIR_SIZE	24.0f

WorldView::WorldView(): View()
{
	m_camType = CAM_FRONT;
	InitObject();
}

WorldView::WorldView(Serializer::Reader &rd): View()
{
	m_camType = CamType(rd.Int32());
	InitObject();
	m_externalCamera->Load(rd);
	m_siderealCamera->Load(rd);
}

void WorldView::InitObject()
{
	float size[2];
	GetSize(size);
	
	m_showTargetActionsTimeout = 0;
	m_numLights = 1;
	m_labelsOn = true;
	SetTransparency(true);

	m_commsOptions = new Fixed(size[0], size[1]/2);
	m_commsOptions->SetTransparency(true);
	Add(m_commsOptions, 10, 200);

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


	m_wheelsButton = new Gui::MultiStateImageButton();
	m_wheelsButton->SetShortcut(SDLK_F6, KMOD_NONE);
	m_wheelsButton->AddState(0, "icons/wheels_up.png", Lang::WHEELS_ARE_UP);
	m_wheelsButton->AddState(1, "icons/wheels_down.png", Lang::WHEELS_ARE_DOWN);
	m_wheelsButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeWheelsState));
	m_rightButtonBar->Add(m_wheelsButton, 34, 2);

	Gui::MultiStateImageButton *labels_button = new Gui::MultiStateImageButton();
	labels_button->SetShortcut(SDLK_F8, KMOD_NONE);
	labels_button->AddState(1, "icons/labels_on.png", Lang::OBJECT_LABELS_ARE_ON);
	labels_button->AddState(0, "icons/labels_off.png", Lang::OBJECT_LABELS_ARE_OFF);
	labels_button->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeLabelsState));
	m_rightButtonBar->Add(labels_button, 98, 2);

	m_hyperspaceButton = new Gui::ImageButton("icons/hyperspace_f8.png");
	m_hyperspaceButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_hyperspaceButton->SetToolTip(Lang::HYPERSPACE_JUMP);
	m_hyperspaceButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickHyperspace));
	m_rightButtonBar->Add(m_hyperspaceButton, 66, 2);

	m_launchButton = new Gui::ImageButton("icons/blastoff.png");
	m_launchButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_launchButton->SetToolTip(Lang::TAKEOFF);
	m_launchButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickBlastoff));
	m_rightButtonBar->Add(m_launchButton, 2, 2);

	m_flightControlButton = new Gui::MultiStateImageButton();
	m_flightControlButton->SetShortcut(SDLK_F5, KMOD_NONE);
	// these states must match Player::FlightControlState (so that the enum values match)
	m_flightControlButton->AddState(Player::CONTROL_MANUAL, "icons/manual_control.png", Lang::MANUAL_CONTROL);
	m_flightControlButton->AddState(Player::CONTROL_FIXSPEED, "icons/manual_control.png", Lang::COMPUTER_SPEED_CONTROL);
	m_flightControlButton->AddState(Player::CONTROL_FIXHEADING_FORWARD, "icons/manual_control.png", Lang::COMPUTER_HEADING_CONTROL);
	m_flightControlButton->AddState(Player::CONTROL_FIXHEADING_BACKWARD, "icons/manual_control.png", Lang::COMPUTER_HEADING_CONTROL);
	m_flightControlButton->AddState(Player::CONTROL_AUTOPILOT, "icons/autopilot.png", Lang::AUTOPILOT_ON);
	m_flightControlButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeFlightState));
	m_rightButtonBar->Add(m_flightControlButton, 2, 2);

	m_flightStatus = (new Gui::Label(""))->Color(1.0f, 0.7f, 0.0f);
	m_rightRegion2->Add(m_flightStatus, 2, 0);

#if WITH_DEVKEYS
	Gui::Screen::PushFont("ConsoleFont");
	m_debugInfo = (new Gui::Label(""))->Color(0.8f, 0.8f, 0.8f);
	Add(m_debugInfo, 10, 200);
	Gui::Screen::PopFont();
#endif

	m_hudVelocity = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudTargetDist = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudAltitude = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudPressure = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudHyperspaceInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudVelocity->SetToolTip(Lang::SHIP_VELOCITY_BY_REFERENCE_OBJECT);
	m_hudTargetDist->SetToolTip(Lang::DISTANCE_FROM_SHIP_TO_NAV_TARGET);
	m_hudAltitude->SetToolTip(Lang::SHIP_ALTITUDE_ABOVE_TERRAIN);
	m_hudPressure->SetToolTip(Lang::EXTERNAL_ATMOSPHERIC_PRESSURE);
	Add(m_hudVelocity, 170.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-65.0f);
	Add(m_hudTargetDist, 500.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-65.0f);
	Add(m_hudAltitude, 580.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-4.0f);
	Add(m_hudPressure, 150.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-4.0f);
	Add(m_hudHyperspaceInfo, Gui::Screen::GetWidth()*0.4f, Gui::Screen::GetHeight()*0.3f);

	m_hudHullTemp = new Gui::MeterBar(100.0f, Lang::HULL_TEMP, Color(1.0f,0.0f,0.0f,0.8f));
	m_hudWeaponTemp = new Gui::MeterBar(100.0f, Lang::WEAPON_TEMP, Color(1.0f,0.5f,0.0f,0.8f));
	m_hudHullIntegrity = new Gui::MeterBar(100.0f, Lang::HULL_INTEGRITY, Color(1.0f,1.0f,0.0f,0.8f));
	m_hudShieldIntegrity = new Gui::MeterBar(100.0f, Lang::SHIELD_INTEGRITY, Color(1.0f,1.0f,0.0f,0.8f));
	m_hudFuelGauge = new Gui::MeterBar(100.f, Lang::FUEL, Color(1.f, 1.f, 0.f, 0.8f));
	Add(m_hudFuelGauge, 5.0f, Gui::Screen::GetHeight() - 104.0f);
	Add(m_hudHullTemp, 5.0f, Gui::Screen::GetHeight() - 144.0f);
	Add(m_hudWeaponTemp, 5.0f, Gui::Screen::GetHeight() - 184.0f);
	Add(m_hudHullIntegrity, Gui::Screen::GetWidth() - 105.0f, Gui::Screen::GetHeight() - 104.0f);
	Add(m_hudShieldIntegrity, Gui::Screen::GetWidth() - 105.0f, Gui::Screen::GetHeight() - 144.0f);

	m_hudTargetHullIntegrity = new Gui::MeterBar(100.0f, Lang::HULL_INTEGRITY, Color(1.0f,1.0f,0.0f,0.8f));
	m_hudTargetShieldIntegrity = new Gui::MeterBar(100.0f, Lang::SHIELD_INTEGRITY, Color(1.0f,1.0f,0.0f,0.8f));
	Add(m_hudTargetHullIntegrity, Gui::Screen::GetWidth() - 105.0f, 5.0f);
	Add(m_hudTargetShieldIntegrity, Gui::Screen::GetWidth() - 105.0f, 45.0f);

	m_hudTargetInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_hudTargetInfo, 0, 85.0f);

	Gui::Screen::PushFont("OverlayFont");
	m_bodyLabels = new Gui::LabelSet();
	m_bodyLabels->SetLabelColor(Color(1.0f, 1.0f, 1.0f, 0.9f));
	Add(m_bodyLabels, 0, 0);
	Gui::Screen::PopFont();

	m_navTargetIndicator.label = (new Gui::Label(""))->Color(0.0f, 1.0f, 0.0f);
	m_navVelIndicator.label = (new Gui::Label(""))->Color(0.0f, 1.0f, 0.0f);
	m_combatTargetIndicator.label = new Gui::Label(""); // colour set dynamically
	m_targetLeadIndicator.label = new Gui::Label("");

	// these labels are repositioned during Draw3D()
	Add(m_navTargetIndicator.label, 0, 0);
	Add(m_navVelIndicator.label, 0, 0);
	Add(m_combatTargetIndicator.label, 0, 0);
	Add(m_targetLeadIndicator.label, 0, 0);

	// XXX m_renderer not set yet
	Graphics::TextureBuilder b = Graphics::TextureBuilder::UI(indicatorMousedirTextureFilename);
	m_indicatorMousedir.Reset(new Gui::TexturedQuad(b.GetOrCreateTexture(Gui::Screen::GetRenderer(), "ui")));

	const Graphics::TextureDescriptor &descriptor = b.GetDescriptor();
	m_indicatorMousedirSize = vector2f(descriptor.dataSize.x*descriptor.texSize.x,descriptor.dataSize.y*descriptor.texSize.y);

	//get near & far clipping distances
	//XXX m_renderer not set yet
	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");
	const vector2f camSize(Pi::GetScrWidth(), Pi::GetScrHeight());
	m_frontCamera = new FrontCamera(Pi::player, camSize, fovY, znear, zfar);
	m_rearCamera = new RearCamera(Pi::player, camSize, fovY, znear, zfar);
	m_externalCamera = new ExternalCamera(Pi::player, camSize, fovY, znear, zfar);
	m_siderealCamera = new SiderealCamera(Pi::player, camSize, fovY, znear, zfar);
	SetCamType(m_camType); //set the active camera
	
	m_onHyperspaceTargetChangedCon =
		Pi::sectorView->onHyperspaceTargetChanged.connect(sigc::mem_fun(this, &WorldView::OnHyperspaceTargetChanged));

	m_onPlayerChangeTargetCon =
		Pi::onPlayerChangeTarget.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeTarget));
	m_onChangeFlightControlStateCon =
		Pi::onPlayerChangeFlightControlState.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeFlightControlState));
	m_onMouseButtonDown =
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &WorldView::MouseButtonDown));

	Pi::player->SetMouseForRearView(m_camType == CAM_REAR);
}

WorldView::~WorldView()
{
	delete m_frontCamera;
	delete m_rearCamera;
	delete m_externalCamera;
	delete m_siderealCamera;

	m_onHyperspaceTargetChangedCon.disconnect();
	m_onPlayerChangeTargetCon.disconnect();
	m_onChangeFlightControlStateCon.disconnect();
	m_onMouseButtonDown.disconnect();
}

void WorldView::Save(Serializer::Writer &wr)
{
	wr.Int32(int(m_camType));
	m_externalCamera->Save(wr);
	m_siderealCamera->Save(wr);
}

void WorldView::SetCamType(enum CamType c)
{
	if (c != m_camType) {
		m_camType = c;
		Pi::player->SetMouseForRearView(c == CAM_REAR);
		onChangeCamType.emit();
	}
	switch(m_camType) {
		case CAM_REAR:
			m_activeCamera = m_rearCamera;
			break;
		case CAM_EXTERNAL:
			m_activeCamera = m_externalCamera;
			break;
		case CAM_SIDEREAL:
			m_activeCamera = m_siderealCamera;
			break;
		default:
			m_activeCamera = m_frontCamera;
	}
	m_activeCamera->Activate();
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
			case Player::CONTROL_FIXSPEED: newState = Player::CONTROL_FIXHEADING_FORWARD; break;
			case Player::CONTROL_AUTOPILOT: newState = Player::CONTROL_MANUAL; break;
			default: break;
		}
	} else {
		// skip certain states
		switch (newState) {
			case Player::CONTROL_FIXHEADING_FORWARD: // fallthrough
			case Player::CONTROL_FIXHEADING_BACKWARD: newState = Player::CONTROL_MANUAL; break;
			case Player::CONTROL_AUTOPILOT: newState = Player::CONTROL_MANUAL; break;
			default: break;
		}
	}
	b->SetActiveState(newState);
	Pi::player->SetFlightControlState(static_cast<Player::FlightControlState>(newState));
}

/* This is when the flight control state actually changes... */
void WorldView::OnPlayerChangeFlightControlState()
{
	m_flightControlButton->SetActiveState(Pi::player->GetFlightControlState());
}

void WorldView::OnChangeLabelsState(Gui::MultiStateImageButton *b)
{
	Pi::BoinkNoise();
	m_labelsOn = b->GetState()!=0;
}

void WorldView::OnClickBlastoff()
{
	Pi::BoinkNoise();
	if (Pi::player->GetFlightState() == Ship::DOCKED) {
		if (!Pi::player->Undock()) {
			Pi::cpan->MsgLog()->ImportantMessage(Pi::player->GetDockedWith()->GetLabel(),
					Lang::LAUNCH_PERMISSION_DENIED_BUSY);
		}
	} else {
		Pi::player->Blastoff();
	}
}

void WorldView::OnClickHyperspace()
{
	if (Pi::player->IsHyperspaceActive()) {
		// Hyperspace countdown in effect.. abort!
		Pi::player->ResetHyperspaceCountdown();
		Pi::cpan->MsgLog()->Message("", Lang::HYPERSPACE_JUMP_ABORTED);
	} else {
		// Initiate hyperspace drive
		SystemPath path = Pi::sectorView->GetHyperspaceTarget();
		Pi::player->StartHyperspaceCountdown(path);
	}
}

WorldView::CamType WorldView::GetCamType() const
{
	if (m_camType == CAM_EXTERNAL || m_camType == CAM_SIDEREAL) {
		// don't allow external view when docked with an orbital starport
		if (Pi::player->GetFlightState() == Ship::DOCKED && !Pi::player->GetDockedWith()->IsGroundStation()) {
			return CAM_FRONT;
		} else {
			return m_camType;
		}
	} else {
		return m_camType;
	}
}

void WorldView::Draw3D()
{
	m_activeCamera->Draw(m_renderer);
}

void WorldView::ShowAll()
{
	View::ShowAll(); // by default, just delegate back to View
	RefreshButtonStateAndVisibility();
}


static Color get_color_for_warning_meter_bar(float v) {
	Color c;
	if (v < 50.0f)
		c = Color(1,0,0,HUD_ALPHA);
	else if (v < 75.0f)
		c = Color(1,0.5,0,HUD_ALPHA);
	else
		c = Color(1,1,0,HUD_ALPHA);
	return c;
}

void WorldView::RefreshHyperspaceButton() {
	if (Pi::player->CanHyperspaceTo(Pi::sectorView->GetHyperspaceTarget()))
		m_hyperspaceButton->Show();
	else
		m_hyperspaceButton->Hide();
}

void WorldView::RefreshButtonStateAndVisibility()
{
	if (!Pi::player || Pi::player->IsDead() || !Pi::game) {
		HideAll();
		return;
	}
	else {
		m_wheelsButton->SetActiveState(int(Pi::player->GetWheelState()));

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

			case Ship::DOCKED:
				m_flightStatus->SetText(Lang::DOCKED);
				m_launchButton->Show();
				m_flightControlButton->Hide();
				break;

			case Ship::HYPERSPACE:
				m_flightStatus->SetText(Lang::HYPERSPACE);
				m_launchButton->Hide();
				m_flightControlButton->Hide();
				break;

			case Ship::FLYING:
			default:
				Player::FlightControlState fstate = Pi::player->GetFlightControlState();
				switch (fstate) {
					case Player::CONTROL_MANUAL:
						m_flightStatus->SetText(Lang::MANUAL_CONTROL); break;

					case Player::CONTROL_FIXSPEED: {
						std::string msg;
						if (Pi::player->GetSetSpeed() > 1000) {
							msg = stringf(Lang::SET_SPEED_KM_S, formatarg("speed", Pi::player->GetSetSpeed()*0.001));
						} else {
							msg = stringf(Lang::SET_SPEED_M_S, formatarg("speed", Pi::player->GetSetSpeed()));
						}
						m_flightStatus->SetText(msg);
						break;
					}

					case Player::CONTROL_FIXHEADING_FORWARD:
						m_flightStatus->SetText(Lang::HEADING_LOCK_FORWARD);
						break;
					case Player::CONTROL_FIXHEADING_BACKWARD:
						m_flightStatus->SetText(Lang::HEADING_LOCK_BACKWARD);
						break;

					case Player::CONTROL_AUTOPILOT:
						m_flightStatus->SetText(Lang::AUTOPILOT);
						break;

					default: assert(0); break;
				}

				m_launchButton->Hide();
				m_flightControlButton->Show();
		}
	}

	// Direction indicator
	vector3d vel = Pi::player->GetVelocityRelTo(Pi::player->GetFrame());

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
#if WITH_DEVKEYS
	if (Pi::showDebugInfo) {
		char buf[1024], aibuf[256];
		vector3d pos = Pi::player->GetPosition();
		vector3d abs_pos = Pi::player->GetPositionRelTo(Pi::game->GetSpace()->GetRootFrame());
		const char *rel_to = (Pi::player->GetFrame() ? Pi::player->GetFrame()->GetLabel() : "System");
		const char *rot_frame = (Pi::player->GetFrame()->IsRotatingFrame() ? "yes" : "no");
		Pi::player->AIGetStatusText(aibuf); aibuf[255] = 0;
		snprintf(buf, sizeof(buf), "Pos: %.1f,%.1f,%.1f\n"
			"AbsPos: %.1f,%.1f,%.1f (%.3f AU)\n"
			"Rel-to: %s (%.0f km), rotating: %s\n" "%s",
			pos.x, pos.y, pos.z,
			abs_pos.x, abs_pos.y, abs_pos.z, abs_pos.Length()/AU,
			rel_to, pos.Length()/1000, rot_frame, aibuf);

		m_debugInfo->SetText(buf);
		m_debugInfo->Show();
	} else {
		m_debugInfo->Hide();
	}
#endif

	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) {
		const SystemPath dest = Pi::player->GetHyperspaceDest();
		RefCountedPtr<StarSystem> s = StarSystem::GetCached(dest);
		m_hudVelocity->SetText(stringf(Lang::IN_TRANSIT_TO_N_X_X_X,
			formatarg("system", s->GetName()),
			formatarg("x", dest.sectorX),
			formatarg("y", dest.sectorY),
			formatarg("z", dest.sectorZ)));
		m_hudVelocity->Show();

		m_hudTargetDist->SetText(stringf(Lang::PROBABILITY_OF_ARRIVAL_X_PERCENT,
			formatarg("probability", Pi::game->GetHyperspaceArrivalProbability()*100.0, "f3.1")));
		m_hudTargetDist->Show();

		m_hudAltitude->Hide();
		m_hudPressure->Hide();
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
				rel_to = Pi::player->GetFrame()->GetLabel();
				_vel = vel.Length();
			}
			if (_vel > 1000) {
				str = stringf(Lang::KM_S_RELATIVE_TO, formatarg("speed", _vel*0.001), formatarg("frame", rel_to));
			} else {
				str = stringf(Lang::M_S_RELATIVE_TO, formatarg("speed", _vel), formatarg("frame", rel_to));
			}
			m_hudVelocity->SetText(str);
		}

		if (Body *navtarget = Pi::player->GetNavTarget()) {
			double dist = Pi::player->GetPositionRelTo(navtarget).Length();
			m_hudTargetDist->SetText(stringf(Lang::N_DISTANCE_TO_TARGET,
				formatarg("distance", format_distance(dist))));
			m_hudTargetDist->Show();
		}
		else
			m_hudTargetDist->Hide();

		// altitude
		if (Pi::player->GetFrame()->m_astroBody) {
			Body *astro = Pi::player->GetFrame()->m_astroBody;
			//(GetFrame()->m_sbody->GetSuperType() == SUPERTYPE_ROCKY_PLANET)) {
			double radius;
			vector3d surface_pos = Pi::player->GetPosition().Normalized();
			if (astro->IsType(Object::TERRAINBODY)) {
				radius = static_cast<TerrainBody*>(astro)->GetTerrainHeight(surface_pos);
			} else {
				// XXX this is an improper use of GetBoundingRadius
				// since it is not a surface radius
				radius = Pi::player->GetFrame()->m_astroBody->GetBoundingRadius();
			}
			double altitude = Pi::player->GetPosition().Length() - radius;
			if (altitude > 9999999.0 || astro->IsType(Object::SPACESTATION)) {
				m_hudAltitude->Hide();
			} else {
				if (altitude < 0) altitude = 0;
				m_hudAltitude->SetText(stringf(Lang::ALT_IN_METRES, formatarg("altitude", altitude)));
				m_hudAltitude->Show();
			}

			if (astro->IsType(Object::PLANET)) {
				double dist = Pi::player->GetPosition().Length();
				double pressure, density;
				reinterpret_cast<Planet*>(astro)->GetAtmosphericState(dist, &pressure, &density);

				m_hudPressure->SetText(stringf(Lang::PRESSURE_N_BAR, formatarg("pressure", pressure)));
				m_hudPressure->Show();

				m_hudHullTemp->SetValue(float(Pi::player->GetHullTemperature()));
				m_hudHullTemp->Show();
			} else {
				m_hudPressure->Hide();
				m_hudHullTemp->Hide();
			}
		} else {
			m_hudAltitude->Hide();
			m_hudPressure->Hide();
			m_hudHullTemp->Hide();
		}

		m_hudFuelGauge->SetValue(Pi::player->GetFuel());
	}

	const float activeWeaponTemp = Pi::player->GetGunTemperature(GetActiveWeapon());
	if (activeWeaponTemp > 0.0f) {
		m_hudWeaponTemp->SetValue(activeWeaponTemp);
		m_hudWeaponTemp->Show();
	} else {
		m_hudWeaponTemp->Hide();
	}

	float hull = Pi::player->GetPercentHull();
	if (hull < 100.0f) {
		m_hudHullIntegrity->SetColor(get_color_for_warning_meter_bar(hull));
		m_hudHullIntegrity->SetValue(hull*0.01f);
		m_hudHullIntegrity->Show();
	} else {
		m_hudHullIntegrity->Hide();
	}
	float shields = Pi::player->GetPercentShields();
	if (shields < 100.0f) {
		m_hudShieldIntegrity->SetColor(get_color_for_warning_meter_bar(shields));
		m_hudShieldIntegrity->SetValue(shields*0.01f);
		m_hudShieldIntegrity->Show();
	} else {
		m_hudShieldIntegrity->Hide();
	}

	Body *b = Pi::player->GetCombatTarget() ? Pi::player->GetCombatTarget() : Pi::player->GetNavTarget();
	if (b) {
		if (b->IsType(Object::SHIP) && Pi::player->m_equipment.Get(Equip::SLOT_RADARMAPPER) == Equip::RADAR_MAPPER) {
			assert(b->IsType(Object::SHIP));
			Ship *s = static_cast<Ship*>(b);

			const ShipFlavour *flavour = s->GetFlavour();
			const shipstats_t *stats = s->CalcStats();

			float sHull = s->GetPercentHull();
			m_hudTargetHullIntegrity->SetColor(get_color_for_warning_meter_bar(sHull));
			m_hudTargetHullIntegrity->SetValue(sHull*0.01f);
			m_hudTargetHullIntegrity->Show();

			float sShields = 0;
			if (s->m_equipment.Count(Equip::SLOT_SHIELD, Equip::SHIELD_GENERATOR) > 0) {
				sShields = s->GetPercentShields();
			}
			m_hudTargetShieldIntegrity->SetColor(get_color_for_warning_meter_bar(sShields));
			m_hudTargetShieldIntegrity->SetValue(sShields*0.01f);
			m_hudTargetShieldIntegrity->Show();

			std::string text;
			text += ShipType::types[flavour->type].name;
			text += "\n";
			text += flavour->regid;
			text += "\n";

			if (s->m_equipment.Get(Equip::SLOT_ENGINE) == Equip::NONE) {
				text += Lang::NO_HYPERDRIVE;
			} else {
				text += Equip::types[s->m_equipment.Get(Equip::SLOT_ENGINE)].name;
			}

			text += "\n";
			text += stringf(Lang::MASS_N_TONNES, formatarg("mass", stats->total_mass));
			text += "\n";
			text += stringf(Lang::SHIELD_STRENGTH_N, formatarg("shields",
				(sShields*0.01f) * float(s->m_equipment.Count(Equip::SLOT_SHIELD, Equip::SHIELD_GENERATOR))));
			text += "\n";
			text += stringf(Lang::CARGO_N, formatarg("mass", stats->used_cargo));
			text += "\n";

			m_hudTargetInfo->SetText(text);
			MoveChild(m_hudTargetInfo, Gui::Screen::GetWidth() - 150.0f, 85.0f);
			m_hudTargetInfo->Show();
		}

		else if (b->IsType(Object::HYPERSPACECLOUD) && Pi::player->m_equipment.Get(Equip::SLOT_HYPERCLOUD) == Equip::HYPERCLOUD_ANALYZER) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(b);

			m_hudTargetHullIntegrity->Hide();
			m_hudTargetShieldIntegrity->Hide();

			std::string text;

			Ship *ship = cloud->GetShip();
			if (!ship) {
				text += Lang::HYPERSPACE_ARRIVAL_CLOUD_REMNANT;
			}
			else {
				const SystemPath dest = ship->GetHyperspaceDest();
				Sector s(dest.sectorX, dest.sectorY, dest.sectorZ);
				text += (cloud->IsArrival() ? Lang::HYPERSPACE_ARRIVAL_CLOUD : Lang::HYPERSPACE_DEPARTURE_CLOUD);
				text += "\n";
				text += stringf(Lang::SHIP_MASS_N_TONNES, formatarg("mass", ship->CalcStats()->total_mass));
				text += "\n";
				text += (cloud->IsArrival() ? Lang::SOURCE : Lang::DESTINATION);
				text += ": ";
				text += s.m_systems[dest.systemIndex].name;
				text += "\n";
				text += stringf(Lang::DATE_DUE_N, formatarg("date", format_date(cloud->GetDueDate())));
				text += "\n";
			}

			m_hudTargetInfo->SetText(text);
			MoveChild(m_hudTargetInfo, Gui::Screen::GetWidth() - 180.0f, 5.0f);
			m_hudTargetInfo->Show();
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

void WorldView::Update()
{
	const double frameTime = Pi::GetFrameTime();
	// show state-appropriate buttons
	RefreshButtonStateAndVisibility();

	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		// when controlling your ship with the mouse you don't want to pick targets
		m_bodyLabels->SetLabelsClickable(false);
	} else {
		m_bodyLabels->SetLabelsClickable(true);
	}

	m_bodyLabels->SetLabelsVisible(m_labelsOn);

	//death animation: slowly pan out
	if (Pi::player->IsDead()) {
		SetCamType(CAM_EXTERNAL);
		static_cast<ExternalCamera*>(m_externalCamera)->SetRotationAngles(0.0, 0.0);
		m_externalCamera->ZoomOut(frameTime * 0.4);
		m_labelsOn = false;
	} else {
		// XXX ugly hack checking for console here
		if (!Pi::IsConsoleActive()) {
			if (Pi::KeyState(SDLK_UP)) m_activeCamera->RotateUp(frameTime);
			if (Pi::KeyState(SDLK_DOWN)) m_activeCamera->RotateDown(frameTime);
			if (Pi::KeyState(SDLK_LEFT)) m_activeCamera->RotateLeft(frameTime);
			if (Pi::KeyState(SDLK_RIGHT)) m_activeCamera->RotateRight(frameTime);
			if (Pi::KeyState(SDLK_EQUALS)) m_activeCamera->ZoomOut(frameTime);
			if (Pi::KeyState(SDLK_MINUS)) m_activeCamera->ZoomIn(frameTime);
			if (Pi::KeyState(SDLK_COMMA)) m_activeCamera->RollLeft(frameTime);
			if (Pi::KeyState(SDLK_PERIOD)) m_activeCamera->RollRight(frameTime);
			if (Pi::KeyState(SDLK_HOME)) m_activeCamera->Reset();

			if (KeyBindings::targetObject.IsActive()) {
				/* Hitting tab causes objects in the crosshairs to be selected */
				Body* const target = PickBody(double(Gui::Screen::GetWidth())/2.0, double(Gui::Screen::GetHeight())/2.0);
				SelectBody(target, false);
			}
		}
	}

	m_activeCamera->UpdateTransform();
	m_activeCamera->Update();
	UpdateProjectedObjects();
}

void WorldView::OnSwitchTo()
{
	RefreshButtonStateAndVisibility();
}

void WorldView::ToggleTargetActions()
{
	if (Pi::game->IsHyperspace() || m_showTargetActionsTimeout)
		HideTargetActions();
	else
		ShowTargetActions();
}

void WorldView::ShowTargetActions()
{
	m_showTargetActionsTimeout = SDL_GetTicks();
	UpdateCommsOptions();
}

void WorldView::HideTargetActions()
{
	m_showTargetActionsTimeout = 0;
	UpdateCommsOptions();
}

Gui::Button *WorldView::AddCommsOption(std::string msg, int ypos, int optnum)
{
	Gui::Label *l = new Gui::Label(msg);
	m_commsOptions->Add(l, 50, float(ypos));

	char buf[8];
	snprintf(buf, sizeof(buf), "%d", optnum);
	Gui::LabelButton *b = new Gui::LabelButton(new Gui::Label(buf));
	b->SetShortcut(SDLKey(SDLK_0 + optnum), KMOD_NONE);
	// hide target actions when things get clicked on
	b->onClick.connect(sigc::mem_fun(this, &WorldView::ToggleTargetActions));
	m_commsOptions->Add(b, 16, float(ypos));
	return b;
}

void WorldView::OnClickCommsNavOption(Body *target)
{
	Pi::player->SetNavTarget(target);
	m_showTargetActionsTimeout = SDL_GetTicks();
}

void WorldView::AddCommsNavOption(std::string msg, Body *target)
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
	std::map< Uint32,std::vector<SBody*> > groups;

	m_commsNavOptions->PackEnd(new Gui::Label(std::string("#ff0")+std::string(Lang::NAVIGATION_TARGETS_IN_THIS_SYSTEM)+std::string("\n")));

	for ( std::vector<SBody*>::const_iterator i = Pi::game->GetSpace()->GetStarSystem()->m_spaceStations.begin();
	      i != Pi::game->GetSpace()->GetStarSystem()->m_spaceStations.end(); ++i) {

		groups[(*i)->parent->path.bodyIndex].push_back(*i);
	}

	for ( std::map< Uint32,std::vector<SBody*> >::const_iterator i = groups.begin(); i != groups.end(); ++i ) {
		m_commsNavOptions->PackEnd(new Gui::Label("#f0f" + Pi::game->GetSpace()->GetStarSystem()->m_bodies[(*i).first]->name));

		for ( std::vector<SBody*>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
			SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPathOf(*j);
			Body *body = Pi::game->GetSpace()->FindBodyForPath(&path);
			AddCommsNavOption((*j)->name, body);
		}
	}
}

static void PlayerRequestDockingClearance(SpaceStation *s)
{
	std::string msg;
	s->GetDockingClearance(Pi::player, msg);
	Pi::cpan->MsgLog()->ImportantMessage(s->GetLabel(), msg);
}

static void PlayerPayFine()
{
	Sint64 crime, fine;
	Polit::GetCrime(&crime, &fine);
	if (Pi::player->GetMoney() == 0) {
		Pi::cpan->MsgLog()->Message("", Lang::YOU_NO_MONEY);
	} else if (fine > Pi::player->GetMoney()) {
		Polit::AddCrime(0, -Pi::player->GetMoney());
		Polit::GetCrime(&crime, &fine);
		Pi::cpan->MsgLog()->Message("", stringf(
			Lang::FINE_PAID_N_BUT_N_REMAINING,
				formatarg("paid", format_money(Pi::player->GetMoney())),
				formatarg("fine", format_money(fine))));
		Pi::player->SetMoney(0);
	} else {
		Pi::player->SetMoney(Pi::player->GetMoney() - fine);
		Pi::cpan->MsgLog()->Message("", stringf(Lang::FINE_PAID_N,
				formatarg("fine", format_money(fine))));
		Polit::AddCrime(0, -fine);
	}
}

void WorldView::OnHyperspaceTargetChanged()
{
	if (Pi::player->IsHyperspaceActive()) {
		Pi::player->ResetHyperspaceCountdown();
		Pi::cpan->MsgLog()->Message("", Lang::HYPERSPACE_JUMP_ABORTED);
	}

	const SystemPath path = Pi::sectorView->GetHyperspaceTarget();

	RefCountedPtr<StarSystem> system = StarSystem::GetCached(path);
	Pi::cpan->MsgLog()->Message("", stringf(Lang::SET_HYPERSPACE_DESTINATION_TO, formatarg("system", system->GetName())));
}

void WorldView::OnPlayerChangeTarget()
{
	Body *b = Pi::player->GetNavTarget();
	if (b) {
		Ship *s = b->IsType(Object::HYPERSPACECLOUD) ? static_cast<HyperspaceCloud*>(b)->GetShip() : 0;
		if (!s || Pi::sectorView->GetHyperspaceTarget() != s->GetHyperspaceDest())
			Pi::sectorView->FloatHyperspaceTarget();
	}

	UpdateCommsOptions();
}

static void autopilot_flyto(Body *b)
{
	Pi::player->SetFlightControlState(Player::CONTROL_AUTOPILOT);
	Pi::player->AIFlyTo(b);
}
static void autopilot_dock(Body *b)
{
	Pi::player->SetFlightControlState(Player::CONTROL_AUTOPILOT);
	Pi::player->AIDock(static_cast<SpaceStation*>(b));
}
static void autopilot_orbit(Body *b, double alt)
{
	Pi::player->SetFlightControlState(Player::CONTROL_AUTOPILOT);
	Pi::player->AIOrbit(b, alt);
}

static void player_target_hypercloud(HyperspaceCloud *cloud)
{
	Pi::sectorView->SetHyperspaceTarget(cloud->GetShip()->GetHyperspaceDest());
}

void WorldView::UpdateCommsOptions()
{
	m_commsOptions->DeleteAllChildren();
	m_commsNavOptions->DeleteAllChildren();

	if (m_showTargetActionsTimeout == 0) return;

	if (Pi::game->GetSpace()->GetStarSystem()->m_spaceStations.size() > 0)
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
	}

	bool hasAutopilot = Pi::player->m_equipment.Get(Equip::SLOT_AUTOPILOT) == Equip::AUTOPILOT;

	if (navtarget) {
		m_commsOptions->Add(new Gui::Label("#0f0"+navtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		if (navtarget->IsType(Object::SPACESTATION)) {
			button = AddCommsOption(Lang::REQUEST_DOCKING_CLEARANCE, ypos, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&PlayerRequestDockingClearance), reinterpret_cast<SpaceStation*>(navtarget)));
			ypos += 32;

			if (Pi::player->m_equipment.Get(Equip::SLOT_AUTOPILOT) == Equip::AUTOPILOT) {
				button = AddCommsOption(Lang::AUTOPILOT_DOCK_WITH_STATION, ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_dock), navtarget));
				ypos += 32;
			}

			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);
			if (fine) {
				button = AddCommsOption(stringf(Lang::PAY_FINE_REMOTELY,
							formatarg("amount", format_money(fine))), ypos, optnum++);
				button->onClick.connect(sigc::ptr_fun(&PlayerPayFine));
				ypos += 32;
			}
		}
		if (hasAutopilot) {
			button = AddCommsOption(stringf(Lang::AUTOPILOT_FLY_TO_VICINITY_OF, formatarg("target", navtarget->GetLabel())), ypos, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_flyto), navtarget));
			ypos += 32;

			if (navtarget->IsType(Object::PLANET) || navtarget->IsType(Object::STAR)) {
				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_LOW_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 1.1));
				ypos += 32;

				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 2.0));
				ypos += 32;

				button = AddCommsOption(stringf(Lang::AUTOPILOT_ENTER_HIGH_ORBIT_AROUND, formatarg("target", navtarget->GetLabel())), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 5.0));
				ypos += 32;
			}
		}

		const Equip::Type t = Pi::player->m_equipment.Get(Equip::SLOT_HYPERCLOUD);
		if ((t != Equip::NONE) && navtarget->IsType(Object::HYPERSPACECLOUD)) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(navtarget);
			if (!cloud->IsArrival()) {
				button = AddCommsOption(Lang::SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE, ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(player_target_hypercloud), cloud));
			}
		}
	}
	if (comtarget && hasAutopilot) {
		m_commsOptions->Add(new Gui::Label("#f00"+comtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		button = AddCommsOption(stringf(Lang::AUTOPILOT_FLY_TO_VICINITY_OF, formatarg("target", comtarget->GetLabel())), ypos, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_flyto), comtarget));
		ypos += 32;
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
		case CAM_REAR: return 1;
		case CAM_EXTERNAL:
		case CAM_FRONT:
		default: return 0;
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
	const Graphics::Frustum frustum = m_activeCamera->GetFrustum();

	const Frame *cam_frame = m_activeCamera->GetFrame();
	matrix4x4d cam_rot = cam_frame->GetTransform();
	cam_rot.ClearToRotOnly();

	// determine projected positions and update labels
	m_bodyLabels->Clear();
	m_projectedPos.clear();
	for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i) {
		Body *b = *i;

		vector3d pos = b->GetInterpolatedPositionRelTo(cam_frame);
		if ((pos.z < -1.0) && project_to_screen(pos, pos, frustum, guiSize)) {

			// only show labels on large or nearby bodies
			if (b->IsType(Object::PLANET) || b->IsType(Object::STAR) || b->IsType(Object::SPACESTATION) || Pi::player->GetPositionRelTo(b).LengthSqr() < 1000000.0*1000000.0)
				m_bodyLabels->Add((*i)->GetLabel(), sigc::bind(sigc::mem_fun(this, &WorldView::SelectBody), *i, true), float(pos.x), float(pos.y));

			m_projectedPos[b] = pos;
		}
	}

	// velocity relative to current frame (white)
	// GetVelocityRelTo considers statis velocity of rotating frame, which GetVelocity() doesn't
	const vector3d camSpaceVel = Pi::player->GetVelocityRelTo(Pi::player->GetFrame()) * cam_rot;
	if (camSpaceVel.LengthSqr() >= 1e-4)
		UpdateIndicator(m_velIndicator, camSpaceVel);
	else
		HideIndicator(m_velIndicator);

	// orientation according to mouse
	if (Pi::player->IsMouseActive()) {
		vector3d mouseDir = Pi::player->GetMouseDir() * cam_rot;
		if (GetCamType() == CAM_REAR)
			mouseDir = -mouseDir;
		UpdateIndicator(m_mouseDirIndicator, (Pi::player->GetBoundingRadius() * 1.5) * mouseDir);
	} else
		HideIndicator(m_mouseDirIndicator);

	// navtarget info
	if (Body *navtarget = Pi::player->GetNavTarget()) {
		// if navtarget and body frame are the same,
		// then we hide the frame-relative velocity indicator
		// (which would be hidden underneath anyway)
		if (navtarget == Pi::player->GetFrame()->GetBodyFor())
			HideIndicator(m_velIndicator);

		// navtarget distance/target square indicator (displayed with navtarget label)
		double dist = (navtarget->GetTargetIndicatorPosition(cam_frame) 
			- Pi::player->GetPositionRelTo(cam_frame)).Length();
		m_navTargetIndicator.label->SetText(format_distance(dist).c_str());
		UpdateIndicator(m_navTargetIndicator, navtarget->GetTargetIndicatorPosition(cam_frame));

		// velocity relative to navigation target
		vector3d navvelocity = Pi::player->GetVelocityRelTo(navtarget);
		double navspeed = navvelocity.Length();
		const vector3d camSpaceNavVel = navvelocity * cam_rot;

		if (navspeed >= 0.01) { // 1 cm per second
			char buf[128];
			if (navspeed > 1000)
				snprintf(buf, sizeof(buf), "%.2f km/s", navspeed*0.001);
			else
				snprintf(buf, sizeof(buf), "%.0f m/s", navspeed);
			m_navVelIndicator.label->SetText(buf);
			UpdateIndicator(m_navVelIndicator, camSpaceNavVel);

			assert(m_navTargetIndicator.side != INDICATOR_HIDDEN);
			assert(m_navVelIndicator.side != INDICATOR_HIDDEN);
			SeparateLabels(m_navTargetIndicator.label, m_navVelIndicator.label);
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
		const vector3d targpos = enemy->GetInterpolatedPositionRelTo(Pi::player) * cam_rot;
		const double dist = targpos.Length();
		const vector3d targScreenPos = enemy->GetInterpolatedPositionRelTo(cam_frame);

		snprintf(buf, sizeof(buf), "%.0fm", dist);
		m_combatTargetIndicator.label->SetText(buf);
		UpdateIndicator(m_combatTargetIndicator, targScreenPos);

		// calculate firing solution and relative velocity along our z axis
		int laser;
		switch (GetCamType()) {
			case CAM_FRONT: laser = 0; break;
			case CAM_REAR: laser = 1; break;
			default: laser = -1; break;
		}
		if (laser >= 0) {
			laser = Pi::player->m_equipment.Get(Equip::SLOT_LASER, laser);
			laser = Equip::types[laser].tableIndex;
		}
		if (laser >= 0) { // only display target lead position on views with lasers
			double projspeed = Equip::lasers[laser].speed;

			const vector3d targvel = enemy->GetVelocityRelTo(Pi::player) * cam_rot;
			vector3d leadpos = targpos + targvel*(targpos.Length()/projspeed);
			leadpos = targpos + targvel*(leadpos.Length()/projspeed); // second order approx

			// now the text speed/distance
			// want to calculate closing velocity that you couldn't counter with retros

			double vel = targvel.Dot(targpos.NormalizedSafe()); // position should be towards
			double raccel =
				Pi::player->GetShipType().linThrust[ShipType::THRUSTER_REVERSE] / Pi::player->GetMass();

			double c = Clamp(vel / sqrt(2.0 * raccel * dist), -1.0, 1.0);
			float r = float(0.2+(c+1.0)*0.4);
			float b = float(0.2+(1.0-c)*0.4);

			m_combatTargetIndicator.label->Color(r, 0.0f, b);
			m_targetLeadIndicator.label->Color(r, 0.0f, b);

			snprintf(buf, sizeof(buf), "%0.fm/s", vel);
			m_targetLeadIndicator.label->SetText(buf);
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
	const Graphics::Frustum frustum = m_activeCamera->GetFrustum();

	const float BORDER = 10.0;
	const float BORDER_BOTTOM = 90.0;
	// XXX BORDER_BOTTOM is 10+the control panel height and shouldn't be needed at all

	const float w = Gui::Screen::GetWidth();
	const float h = Gui::Screen::GetHeight();

	if (cameraSpacePos.LengthSqr() < 1e-6) { // length < 1e-3
		indicator.pos[0] = w/2.0f;
		indicator.pos[1] = h/2.0f;
		indicator.side = INDICATOR_ONSCREEN;
	} else {
		vector3d proj;
		bool success = project_to_screen(cameraSpacePos, proj, frustum, guiSize);
		if (! success)
			proj = vector3d(w/2.0, h/2.0, 0.0);

		bool onscreen =
			(cameraSpacePos.z < 0.0) &&
			(proj.x >= BORDER) && (proj.x < w - BORDER) &&
			(proj.y >= BORDER) && (proj.y < h - BORDER_BOTTOM);

		if (onscreen) {
			indicator.pos[0] = int(proj.x);
			indicator.pos[1] = int(proj.y);
			indicator.side = INDICATOR_ONSCREEN;

		} else {
			// homogeneous 2D points and lines are really useful
			const vector3d ptCentre(w/2.0, h/2.0, 1.0);
			const vector3d ptProj(proj.x, proj.y, 1.0);
			const vector3d lnDir = ptProj.Cross(ptCentre);

			indicator.side = INDICATOR_TOP;

			// this fallback is used if direction is close to (0, 0, +ve)
			indicator.pos[0] = w/2.0;
			indicator.pos[1] = BORDER;

			if (cameraSpacePos.x < -1e-3) {
				vector3d ptLeft = lnDir.Cross(vector3d(-1.0, 0.0, BORDER));
				ptLeft /= ptLeft.z;
				if (ptLeft.y >= BORDER && ptLeft.y < h - BORDER_BOTTOM) {
					indicator.pos[0] = ptLeft.x;
					indicator.pos[1] = ptLeft.y;
					indicator.side = INDICATOR_LEFT;
				}
			} else if (cameraSpacePos.x > 1e-3) {
				vector3d ptRight = lnDir.Cross(vector3d(-1.0, 0.0,  w - BORDER));
				ptRight /= ptRight.z;
				if (ptRight.y >= BORDER && ptRight.y < h - BORDER_BOTTOM) {
					indicator.pos[0] = ptRight.x;
					indicator.pos[1] = ptRight.y;
					indicator.side = INDICATOR_RIGHT;
				}
			}

			if (cameraSpacePos.y < -1e-3) {
				vector3d ptBottom = lnDir.Cross(vector3d(0.0, -1.0, h - BORDER_BOTTOM));
				ptBottom /= ptBottom.z;
				if (ptBottom.x >= BORDER && ptBottom.x < w-BORDER) {
					indicator.pos[0] = ptBottom.x;
					indicator.pos[1] = ptBottom.y;
					indicator.side = INDICATOR_BOTTOM;
				}
			} else if (cameraSpacePos.y > 1e-3) {
				vector3d ptTop = lnDir.Cross(vector3d(0.0, -1.0, BORDER));
				ptTop /= ptTop.z;
				if (ptTop.x >= BORDER && ptTop.x < w - BORDER) {
					indicator.pos[0] = ptTop.x;
					indicator.pos[1] = ptTop.y;
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
				if (indicator.pos[1] + pos[1] + labelSize[1] + HUD_CROSSHAIR_SIZE + 2.0f > h - BORDER_BOTTOM)
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

			pos[0] = Clamp(pos[0] + indicator.pos[0], BORDER, w - BORDER - labelSize[0]);
			pos[1] = Clamp(pos[1] + indicator.pos[1], BORDER, h - BORDER_BOTTOM - labelSize[1]);
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
	indicator.pos[0] = indicator.pos[1] = 0;
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

void WorldView::Draw()
{
	View::Draw();

	// don't draw crosshairs etc in hyperspace
	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) return;

	m_renderer->SetBlendMode(Graphics::BLEND_ALPHA);

	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
	glLineWidth(2.0f);

	Color white(1.f, 1.f, 1.f, 0.8f);
	Color green(0.f, 1.f, 0.f, 0.8f);
	Color yellow(0.9f, 0.9f, 0.3f, 1.f);
	Color red(1.f, 0.f, 0.f, 0.5f);

	// nav target square
	DrawTargetSquare(m_navTargetIndicator, green);

	glLineWidth(1.0f);

	// velocity indicators
	DrawVelocityIndicator(m_velIndicator, white);
	DrawVelocityIndicator(m_navVelIndicator, green);

	glLineWidth(2.0f);

	DrawImageIndicator(m_mouseDirIndicator, m_indicatorMousedir.Get(), yellow);

	// combat target indicator
	DrawCombatTargetIndicator(m_combatTargetIndicator, m_targetLeadIndicator, red);

	glLineWidth(1.0f);

	// normal crosshairs
	if (GetCamType() == WorldView::CAM_FRONT)
		DrawCrosshair(Gui::Screen::GetWidth()/2.0f, Gui::Screen::GetHeight()/2.0f, HUD_CROSSHAIR_SIZE, white);
	else if (GetCamType() == WorldView::CAM_REAR)
		DrawCrosshair(Gui::Screen::GetWidth()/2.0f, Gui::Screen::GetHeight()/2.0f, HUD_CROSSHAIR_SIZE/2.0f, white);

	glPopAttrib();

	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
}

void WorldView::DrawCrosshair(float px, float py, float sz, const Color &c)
{
	const vector2f vts[] = {
		vector2f(px-sz, py),
		vector2f(px-0.5f*sz, py),
		vector2f(px+sz, py),
		vector2f(px+0.5f*sz, py),
		vector2f(px, py-sz),
		vector2f(px, py-0.5f*sz),
		vector2f(px, py+sz),
		vector2f(px, py+0.5f*sz)
	};
	m_renderer->DrawLines2D(8, vts, c);
}

void WorldView::DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c)
{
	if (target.side == INDICATOR_HIDDEN) return;

	if (target.side == INDICATOR_ONSCREEN) {
		float x1 = target.pos[0], y1 = target.pos[1];
		float x2 = lead.pos[0], y2 = lead.pos[1];

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

		const vector2f vts[] = {
			// target crosshairs
			vector2f(x1+10*xd, y1+10*yd),
			vector2f(x1+20*xd, y1+20*yd),
			vector2f(x1-10*xd, y1-10*yd),
			vector2f(x1-20*xd, y1-20*yd),
			vector2f(x1-10*yd, y1+10*xd),
			vector2f(x1-20*yd, y1+20*xd),
			vector2f(x1+10*yd, y1-10*xd),
			vector2f(x1+20*yd, y1-20*xd),

			// lead crosshairs
			vector2f(x2-10*xd, y2-10*yd),
			vector2f(x2+10*xd, y2+10*yd),  
			vector2f(x2-10*yd, y2+10*xd),
			vector2f(x2+10*yd, y2-10*xd),

			// line between crosshairs
			vector2f(x1+20*xd, y1+20*yd),
			vector2f(x2-10*xd, y2-10*yd)
		};
		if (lead.side == INDICATOR_ONSCREEN)
			m_renderer->DrawLines2D(14, vts, c); //draw all
		else
			m_renderer->DrawLines2D(8, vts, c); //only crosshair
	} else
		DrawEdgeMarker(target, c);
}

void WorldView::DrawTargetSquare(const Indicator &marker, const Color &c)
{
	if (marker.side == INDICATOR_HIDDEN) return;
	if (marker.side != INDICATOR_ONSCREEN)
		DrawEdgeMarker(marker, c);

	// if the square is off-screen, draw a little square at the edge
	const float sz = (marker.side == INDICATOR_ONSCREEN)
		? float(WorldView::PICK_OBJECT_RECT_SIZE * 0.5) : 3.0f;

	const float x1 = float(marker.pos[0] - sz);
	const float x2 = float(marker.pos[0] + sz);
	const float y1 = float(marker.pos[1] - sz);
	const float y2 = float(marker.pos[1] + sz);

	const vector2f vts[] = {
		vector2f(x1, y1),
		vector2f(x2, y1),
		vector2f(x2, y2),
		vector2f(x1, y2)
	};
	m_renderer->DrawLines2D(4, vts, c, Graphics::LINE_LOOP);
}

void WorldView::DrawVelocityIndicator(const Indicator &marker, const Color &c)
{
	if (marker.side == INDICATOR_HIDDEN) return;

	const float sz = HUD_CROSSHAIR_SIZE;
	if (marker.side == INDICATOR_ONSCREEN) {
		const float posx = marker.pos[0];
		const float posy = marker.pos[1];
		const vector2f vts[] = {
			vector2f(posx-sz, posy-sz),
			vector2f(posx-0.5f*sz, posy-0.5f*sz),
			vector2f(posx+sz, posy-sz),
			vector2f(posx+0.5f*sz, posy-0.5f*sz),
			vector2f(posx+sz, posy+sz),
			vector2f(posx+0.5f*sz, posy+0.5f*sz),
			vector2f(posx-sz, posy+sz),
			vector2f(posx-0.5f*sz, posy+0.5f*sz)
		};
		m_renderer->DrawLines2D(8, vts, c);
	} else
		DrawEdgeMarker(marker, c);

}

void WorldView::DrawCircleIndicator(const Indicator &marker, const Color &c)
{
	if (marker.side == INDICATOR_HIDDEN) return;

	// XXX implement DrawCircle
	const float sz = HUD_CROSSHAIR_SIZE*0.5;
	if (marker.side == INDICATOR_ONSCREEN) {
		const float posx = marker.pos[0];
		const float posy = marker.pos[1];
		std::vector<vector2f> vts;
		for (int i = 0; i < 72; i++) {
			vts.push_back(vector2f(
				posx+sinf(DEG2RAD(i*5))*sz,
				posy+cosf(DEG2RAD(i*5))*sz));
		}
		m_renderer->DrawLines2D(72, &vts[0], c, Graphics::LINE_LOOP);
	} else
		DrawEdgeMarker(marker, c);
}

void WorldView::DrawImageIndicator(const Indicator &marker, Gui::TexturedQuad *quad, const Color &c)
{
	if (marker.side == INDICATOR_HIDDEN) return;

	if (marker.side == INDICATOR_ONSCREEN) {
		const float x = marker.pos[0] - m_indicatorMousedirSize.x/2.0f;
		const float y = marker.pos[1] - m_indicatorMousedirSize.y/2.0f;
		quad->Draw(Pi::renderer, vector2f(x,y), m_indicatorMousedirSize, c);
	} else
		DrawEdgeMarker(marker, c);
}

void WorldView::DrawEdgeMarker(const Indicator &marker, const Color &c)
{
	const float sz = HUD_CROSSHAIR_SIZE;

	// this would be easier with a vector2 class
	float dirx = Gui::Screen::GetWidth()/2.0f - float(marker.pos[0]);
	float diry = Gui::Screen::GetHeight()/2.0f - float(marker.pos[1]);
	float len = sqrt(dirx*dirx + diry*diry);
	dirx *= sz/len;
	diry *= sz/len;
	const vector2f vts[] = {
		vector2f(marker.pos[0], marker.pos[1]),
		vector2f(marker.pos[0] + dirx, marker.pos[1] + diry)
	};
	m_renderer->DrawLines2D(2, vts, c);
}

void WorldView::MouseButtonDown(int button, int x, int y)
{
	if (this == Pi::GetView())
	{
		const float ft = Pi::GetFrameTime();
		if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN))
			m_activeCamera->ZoomOut(ft);
		if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP))
			m_activeCamera->ZoomIn(ft);
	}
}
