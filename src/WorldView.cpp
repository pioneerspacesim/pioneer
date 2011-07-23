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

const double WorldView::PICK_OBJECT_RECT_SIZE = 20.0;
static const Color s_hudTextColor(0.0f,1.0f,0.0f,0.8f);

#define HUD_CROSSHAIR_SIZE	24.0f

WorldView::WorldView(): View(),
	m_showHyperspaceButton(false)
{
	float size[2];
	GetSize(size);
	
	m_showTargetActionsTimeout = 0;
	m_numLights = 1;
	m_labelsOn = true;
	m_camType = CAM_FRONT;
	SetTransparency(true);
	m_externalViewRotX = m_externalViewRotY = 0;
	m_externalViewDist = 200;
	
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
	m_wheelsButton->AddState(0, PIONEER_DATA_DIR "/icons/wheels_up.png", "Wheels are up");
	m_wheelsButton->AddState(1, PIONEER_DATA_DIR "/icons/wheels_down.png", "Wheels are down");
	m_wheelsButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeWheelsState));
	m_rightButtonBar->Add(m_wheelsButton, 34, 2);

	Gui::MultiStateImageButton *labels_button = new Gui::MultiStateImageButton();
	labels_button->SetShortcut(SDLK_F8, KMOD_NONE);
	labels_button->AddState(1, PIONEER_DATA_DIR "/icons/labels_on.png", "Object labels are on");
	labels_button->AddState(0, PIONEER_DATA_DIR "/icons/labels_off.png", "Object labels are off");
	labels_button->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeLabelsState));
	m_rightButtonBar->Add(labels_button, 98, 2);

	m_hyperspaceButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/hyperspace_f8.png");
	m_hyperspaceButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_hyperspaceButton->SetToolTip("Hyperspace Jump");
	m_hyperspaceButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickHyperspace));
	m_rightButtonBar->Add(m_hyperspaceButton, 66, 2);

	m_launchButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/blastoff.png");
	m_launchButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_launchButton->SetToolTip("Takeoff");
	m_launchButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickBlastoff));
	m_rightButtonBar->Add(m_launchButton, 2, 2);

	m_flightControlButton = new Gui::MultiStateImageButton();
	m_flightControlButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_flightControlButton->AddState(Player::CONTROL_MANUAL, PIONEER_DATA_DIR "/icons/manual_control.png", "Manual control");
	m_flightControlButton->AddState(Player::CONTROL_FIXSPEED, PIONEER_DATA_DIR "/icons/manual_control.png", "Computer speed control");
	m_flightControlButton->AddState(Player::CONTROL_AUTOPILOT, PIONEER_DATA_DIR "/icons/autopilot.png", "Autopilot on");
	m_flightControlButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeFlightState));
	m_rightButtonBar->Add(m_flightControlButton, 2, 2);

	m_flightStatus = (new Gui::Label(""))->Color(1.0f, 0.7f, 0.0f);
	m_rightRegion2->Add(m_flightStatus, 2, 0);

#if DEVKEYS
	m_debugInfo = (new Gui::Label(""))->Color(0.8f, 0.8f, 0.8f);
	Add(m_debugInfo, 10, 200);
#endif

	m_hudVelocity = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudTargetDist = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudAltitude = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudPressure = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudHyperspaceInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	m_hudVelocity->SetToolTip("Ship velocity by reference object");
	m_hudTargetDist->SetToolTip("Distance from ship to navigation target");
	m_hudAltitude->SetToolTip("Ship altitude above terrain");
	m_hudPressure->SetToolTip("External atmospheric pressure");
	Add(m_hudVelocity, 170.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-65.0f);
	Add(m_hudTargetDist, 500.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-65.0f);
	Add(m_hudAltitude, 580.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-4.0f);
	Add(m_hudPressure, 150.0f, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-4.0f);
	Add(m_hudHyperspaceInfo, Gui::Screen::GetWidth()*0.4f, Gui::Screen::GetHeight()*0.3f);

	m_hudHullTemp = new Gui::MeterBar(100.0f, "Hull temp", Color(1.0f,0.0f,0.0f,0.8f));
	m_hudWeaponTemp = new Gui::MeterBar(100.0f, "Weapon temp", Color(1.0f,0.5f,0.0f,0.8f));
	m_hudHullIntegrity = new Gui::MeterBar(100.0f, "Hull integrity", Color(1.0f,1.0f,0.0f,0.8f));
	m_hudShieldIntegrity = new Gui::MeterBar(100.0f, "Shield integrity", Color(1.0f,1.0f,0.0f,0.8f));
	Add(m_hudHullTemp, 5.0f, Gui::Screen::GetHeight() - 104.0f);
	Add(m_hudWeaponTemp, 5.0f, Gui::Screen::GetHeight() - 144.0f);
	Add(m_hudHullIntegrity, Gui::Screen::GetWidth() - 105.0f, Gui::Screen::GetHeight() - 104.0f);
	Add(m_hudShieldIntegrity, Gui::Screen::GetWidth() - 105.0f, Gui::Screen::GetHeight() - 144.0f);

	m_hudTargetHullIntegrity = new Gui::MeterBar(100.0f, "Hull integrity", Color(1.0f,1.0f,0.0f,0.8f));
	m_hudTargetShieldIntegrity = new Gui::MeterBar(100.0f, "Shield integrity", Color(1.0f,1.0f,0.0f,0.8f));
	Add(m_hudTargetHullIntegrity, Gui::Screen::GetWidth() - 105.0f, 5.0f);
	Add(m_hudTargetShieldIntegrity, Gui::Screen::GetWidth() - 105.0f, 45.0f);

	m_hudTargetInfo = (new Gui::Label(""))->Color(s_hudTextColor);
	Add(m_hudTargetInfo, 0, 85.0f);

	m_bodyLabels = new Gui::LabelSet();
	m_bodyLabels->SetLabelColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
	Add(m_bodyLabels, 0, 0);

	m_targetDist = new Gui::Label("");
	m_targetSpeed = new Gui::Label("");
	m_combatDist = new Gui::Label("");
	m_combatSpeed = new Gui::Label("");
	Add(m_targetDist, 0, 0);			// text/color/position set dynamically
	Add(m_targetSpeed, 0, 0);			// text/color/position set dynamically
	Add(m_combatDist, 0, 0);			// text/color/position set dynamically
	Add(m_combatSpeed, 0, 0);			// text/color/position set dynamically

	m_onHyperspaceTargetChangedCon =
		Pi::sectorView->onHyperspaceTargetChanged.connect(sigc::mem_fun(this, &WorldView::OnHyperspaceTargetChanged));

	m_onPlayerChangeTargetCon =
		Pi::onPlayerChangeTarget.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeTarget));
	m_onChangeFlightControlStateCon =
		Pi::onPlayerChangeFlightControlState.connect(sigc::mem_fun(this, &WorldView::OnPlayerChangeFlightControlState));
	m_onMouseButtonDown =
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &WorldView::MouseButtonDown));
}

WorldView::~WorldView()
{
	m_onHyperspaceTargetChangedCon.disconnect();

	m_onPlayerChangeTargetCon.disconnect();
	m_onChangeFlightControlStateCon.disconnect();
	m_onMouseButtonDown.disconnect();
}

void WorldView::Save(Serializer::Writer &wr)
{
	wr.Float(float(m_externalViewRotX));
	wr.Float(float(m_externalViewRotY));
	wr.Float(float(m_externalViewDist));
	wr.Int32(int(m_camType));
}

void WorldView::Load(Serializer::Reader &rd)
{
	m_externalViewRotX = rd.Float();
	m_externalViewRotY = rd.Float();
	m_externalViewDist = rd.Float();
	m_camType = CamType(rd.Int32());
}

void WorldView::GetNearFarClipPlane(float *outNear, float *outFar) const
{
	if (Render::AreShadersEnabled()) {
		/* If vertex shaders are enabled then we have a lovely logarithmic
		 * z-buffer stretching out from 0.1mm to 10000km! */
		*outNear = 0.0001f;
		*outFar = 10000000.0f;
	} else {
		/* Otherwise we have the usual hopelessly crap z-buffer */
		*outNear = 10.0f;
		*outFar = 1000000.0f;
	}
}

void WorldView::SetCamType(enum CamType c)
{
	m_camType = c;
	onChangeCamType.emit();
}

vector3d WorldView::GetExternalViewTranslation()
{
	vector3d p = vector3d(0, 0, m_externalViewDist);
	p = matrix4x4d::RotateXMatrix(-DEG2RAD(m_externalViewRotX)) * p;
	p = matrix4x4d::RotateYMatrix(-DEG2RAD(m_externalViewRotY)) * p;
	matrix4x4d m = Pi::player->GetInterpolatedTransform();
	m.ClearToRotOnly();
	p = m*p;
	return p;
}

void WorldView::ApplyExternalViewRotation(matrix4x4d &m)
{
	m = matrix4x4d::RotateXMatrix(-DEG2RAD(m_externalViewRotX)) * m;
	m = matrix4x4d::RotateYMatrix(-DEG2RAD(m_externalViewRotY)) * m;
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
	if (b->GetState() == Player::CONTROL_AUTOPILOT) b->StateNext();
	Pi::player->SetFlightControlState(static_cast<Player::FlightControlState>(b->GetState()));
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
					"Permission to launch denied: docking bay busy.");
		}
	} else {
		Pi::player->Blastoff();
	}
}


void WorldView::OnClickHyperspace()
{
	if (Pi::player->GetHyperspaceCountdown() > 0.0) {
		// Hyperspace countdown in effect.. abort!
		Pi::player->ResetHyperspaceCountdown();
		Pi::cpan->MsgLog()->Message("", "Hyperspace jump aborted.");
	} else {
		// Initiate hyperspace drive
		SystemPath path = Pi::sectorView->GetHyperspaceTarget();
		Pi::player->StartHyperspaceCountdown(path);
	}
}

// This is the background starfield
void WorldView::DrawBgStars() 
{
	// make it rotated a bit so star systems are not in the same
	// plane (could make it different per system...
	glPushMatrix();
	glRotatef(40.0, 1.0,2.0,3.0);
	m_milkyWay.Draw();
	glPopMatrix();
	m_starfield.Draw();
}

static void position_system_lights(Frame *camFrame, Frame *frame, int &lightNum)
{
	if (lightNum > 3) return;
	// not using frame->GetSBodyFor() because it snoops into parent frames,
	// causing duplicate finds for static and rotating frame
	SBody *body = frame->m_sbody;

	if (body && (body->GetSuperType() == SBody::SUPERTYPE_STAR)) {
		int light;
		switch (lightNum) {
			case 3: light = GL_LIGHT3; break;
			case 2: light = GL_LIGHT2; break;
			case 1: light = GL_LIGHT1; break;
			default: light = GL_LIGHT0; break;
		}
		// position light at sol
		matrix4x4d m;
		Frame::GetFrameTransform(frame, camFrame, m);
		vector3d lpos = (m * vector3d(0,0,0));
		double dist = lpos.Length() / AU;
		lpos *= 1.0/dist; // normalize
		float lightPos[4];
		lightPos[0] = float(lpos.x);
		lightPos[1] = float(lpos.y);
		lightPos[2] = float(lpos.z);
		lightPos[3] = 0;

		const float *col = StarSystem::starRealColors[body->type];
		float lightCol[4] = { col[0], col[1], col[2], 0 };
		float ambCol[4] = { 0,0,0,0 };
		if (Render::IsHDREnabled()) {
			for (int i=0; i<4; i++) {
				// not too high or we overflow our float16 colorbuffer
				lightCol[i] *= float(std::min(10.0*StarSystem::starLuminosities[body->type] / dist, 10000.0));
			}
		}

		glLightfv(light, GL_POSITION, lightPos);
		glLightfv(light, GL_DIFFUSE, lightCol);
		glLightfv(light, GL_AMBIENT, ambCol);
		glLightfv(light, GL_SPECULAR, lightCol);
		glEnable(light);

		lightNum++;
	}

	for (std::list<Frame*>::iterator i = frame->m_children.begin(); i!=frame->m_children.end(); ++i) {
		position_system_lights(camFrame, *i, lightNum);
	}
}

WorldView::CamType WorldView::GetCamType() const
{
	if (m_camType == CAM_EXTERNAL) {
		// don't allow external view when docked with an orbital starport
		if (Pi::player->GetFlightState() == Ship::DOCKED && !Pi::player->GetDockedWith()->IsGroundStation()) {
			return CAM_FRONT;
		} else {
			return CAM_EXTERNAL;
		}
	} else {
		return m_camType;
	}
}

void WorldView::Draw3D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float znear, zfar;
	GetNearFarClipPlane(&znear, &zfar);
	// why the hell do i give these functions such big names..
   const float FOV_MAX = 170.0f; // Maximum FOV in degrees
   const float FOV_MIN = 20.0f;  // Minimum FOV in degrees
	const float zoom = tan(DEG2RAD(Clamp(Pi::config.Float("FOV"), FOV_MIN, FOV_MAX)/2.0f)); // angle of viewing = 2.0*atan(zoom);
	const float left = zoom * znear;
	const float fracH = left / Pi::GetScrAspect();
	glFrustum(-left, left, -fracH, fracH, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// interpolate between last physics tick position and current one,
	// to remove temporal aliasing
	matrix4x4d pposOrient = Pi::player->GetInterpolatedTransform();
	const vector3d ppos(pposOrient[12], pposOrient[13], pposOrient[14]);

	// make temporary camera frame at player
	Frame cam_frame(Pi::player->GetFrame(), "camera", Frame::TEMP_VIEWING);

	matrix4x4d camRot = matrix4x4d::Identity();

	enum CamType camtype = GetCamType();
	if (camtype == CAM_FRONT) {
		cam_frame.SetPosition(ppos);
	} else if (camtype == CAM_REAR) {
		camRot.RotateY(M_PI);
	//	glRotatef(180.0f, 0, 1, 0);
		cam_frame.SetPosition(ppos);
	} else /* CAM_EXTERNAL */ {
		cam_frame.SetPosition(ppos + GetExternalViewTranslation());
		ApplyExternalViewRotation(camRot);
	}

	{
		matrix4x4d prot = pposOrient;
		prot.ClearToRotOnly();
		camRot = prot * camRot;
	}
	cam_frame.SetRotationOnly(camRot);
	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	cam_frame.ClearMovement();

	matrix4x4d trans2bg;
	Frame::GetFrameTransform(Space::rootFrame, &cam_frame, trans2bg);
	trans2bg.ClearToRotOnly();
	glPushMatrix();
	glMultMatrixd(&trans2bg[0]);
	DrawBgStars();
	glPopMatrix();

	m_numLights = 0;
	position_system_lights(&cam_frame, Space::rootFrame, m_numLights);

	if (m_numLights == 0) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		// fake one up and give a little ambient light so that we can see and
		// so that things that need lights don't explode
		float lightPos[4] = { 0,0,0,0 };
		float lightCol[4] = { 1.0, 1.0, 1.0, 0 };
		float ambCol[4] = { 1.0,1.0,1.0,0 };

		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambCol);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
		glEnable(GL_LIGHT0);

		m_numLights++;
	}

	Render::State::SetNumLights(m_numLights);
	{
		GetNearFarClipPlane(&znear, &zfar);
		Render::State::SetZnearZfar(znear, zfar);
	}

	Space::Render(&cam_frame);
	if (!Pi::player->IsDead()) ProjectObjsToScreenPos(&cam_frame);

	Pi::player->GetFrame()->RemoveChild(&cam_frame);

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
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

void WorldView::RefreshButtonStateAndVisibility()
{
	if ((!Pi::player) || Pi::player->IsDead() || !Pi::IsGameStarted()) {
		HideAll();
		return;
	}
	else {
		m_wheelsButton->SetActiveState(int(Pi::player->GetWheelState()));

		if (m_showHyperspaceButton && Pi::player->GetFlightState() == Ship::FLYING)
			m_hyperspaceButton->Show();
		else
			m_hyperspaceButton->Hide();

		switch(Pi::player->GetFlightState()) {
			case Ship::LANDED:
				m_flightStatus->SetText("Landed");
				m_launchButton->Show();
				m_flightControlButton->Hide();
				break;
				
			case Ship::DOCKING:
				m_flightStatus->SetText("Docking");
				m_launchButton->Hide();
				m_flightControlButton->Hide();
				break;

			case Ship::DOCKED:
				m_flightStatus->SetText("Docked");
				m_launchButton->Show();
				m_flightControlButton->Hide();
				break;

			case Ship::HYPERSPACE:
				m_flightStatus->SetText("Hyperspace");
				m_launchButton->Hide();
				m_flightControlButton->Hide();
				break;

			case Ship::FLYING:
			default:
				Player::FlightControlState fstate = Pi::player->GetFlightControlState();
				switch (fstate) {
					case Player::CONTROL_MANUAL:
						m_flightStatus->SetText("Manual Control"); break;

					case Player::CONTROL_FIXSPEED: {
						std::string msg;
						if (Pi::player->GetSetSpeed() > 1000) {
							msg = stringf(256, "Set speed: %.2f km/s", Pi::player->GetSetSpeed()*0.001);
						} else {
							msg = stringf(256, "Set speed: %.0f m/s", Pi::player->GetSetSpeed());
						}
						m_flightStatus->SetText(msg);
						break;
					}

					case Player::CONTROL_AUTOPILOT:
						m_flightStatus->SetText("Autopilot");
						break;
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
#if DEVKEYS
	if (Pi::showDebugInfo) {
		char buf[1024];
		vector3d pos = Pi::player->GetPosition();
		vector3d abs_pos = Pi::player->GetPositionRelTo(Space::rootFrame);
		const char *rel_to = (Pi::player->GetFrame() ? Pi::player->GetFrame()->GetLabel() : "System");
		const char *rot_frame = (Pi::player->GetFrame()->IsRotatingFrame() ? "yes" : "no");
		snprintf(buf, sizeof(buf), "Pos: %.1f,%.1f,%.1f\n"
			"AbsPos: %.1f,%.1f,%.1f (%.3f AU)\n"
			"Rel-to: %s (%.0f km), rotating: %s\n",
			pos.x, pos.y, pos.z,
			abs_pos.x, abs_pos.y, abs_pos.z, abs_pos.Length()/AU,
			rel_to, pos.Length()/1000, rot_frame);

		m_debugInfo->SetText(buf);
		m_debugInfo->Show();
	} else {
		m_debugInfo->Hide();
	}
#endif

	if (const SystemPath *dest = Space::GetHyperspaceDest()) {
		StarSystem *s = StarSystem::GetCached(*dest);
		char buf[128];
		snprintf(buf, sizeof(buf), "In transit to %s [%d,%d]", s->GetName().c_str(), dest->sectorX, dest->sectorY);
		m_hudVelocity->SetText(buf);
		m_hudVelocity->Show();

		m_hudTargetDist->Hide();
		m_hudAltitude->Hide();
		m_hudPressure->Hide();
	}

	else {
		{
			double _vel = vel.Length();
			char buf[128];
			const char *rel_to = Pi::player->GetFrame()->GetLabel();
			vector3d pos = Pi::player->GetPosition();
			if (_vel > 1000) {
				snprintf(buf,sizeof(buf), "%.2f km/s rel-to %s", _vel*0.001, rel_to);
			} else {
				snprintf(buf,sizeof(buf), "%.0f m/s rel-to %s", _vel, rel_to);
			}
			m_hudVelocity->SetText(buf);
		}

		if (Body *navtarget = Pi::player->GetNavTarget()) {
			double dist = Pi::player->GetPositionRelTo(navtarget).Length();
			char buf[128];
			snprintf(buf, sizeof(buf), "%s to target", format_distance(dist).c_str());
			m_hudTargetDist->SetText(buf);
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
			if (astro->IsType(Object::PLANET)) {
				radius = static_cast<Planet*>(astro)->GetTerrainHeight(surface_pos);
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
				char buf[128];
				snprintf(buf, sizeof(buf), "Alt: %.0fm", altitude);
				m_hudAltitude->SetText(buf);
				m_hudAltitude->Show();
			}

			if (astro->IsType(Object::PLANET)) {
				double dist = Pi::player->GetPosition().Length();
				double pressure, density;
				reinterpret_cast<Planet*>(astro)->GetAtmosphericState(dist, &pressure, &density);
				char buf[128];
				snprintf(buf, sizeof(buf), "P: %.2f bar", pressure);

				m_hudPressure->SetText(buf);
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
	}

	const float activeWeaponTemp = Pi::player->GetGunTemperature(GetActiveWeapon());
	if (activeWeaponTemp != 0) {
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
			if (s->m_equipment.Count(Equip::SLOT_CARGO, Equip::SHIELD_GENERATOR) > 0) {
				sShields = s->GetPercentShields();
			}
			m_hudTargetShieldIntegrity->SetColor(get_color_for_warning_meter_bar(sShields));
			m_hudTargetShieldIntegrity->SetValue(sShields*0.01f);
			m_hudTargetShieldIntegrity->Show();

			std::string text;
			text += stringf(256, "%s\n", ShipType::types[flavour->type].name.c_str());
			text += stringf(256, "%s\n", flavour->regid);

			if (s->m_equipment.Get(Equip::SLOT_ENGINE) == Equip::NONE) {
				text += "No hyperdrive";
			} else {
				text += EquipType::types[s->m_equipment.Get(Equip::SLOT_ENGINE)].name;
			}

			text += stringf(256, "\nMass: %dt\n", stats->total_mass);
			text += stringf(256, "Shield strength: %.2f\n",
				(sShields*0.01f) * float(s->m_equipment.Count(Equip::SLOT_CARGO, Equip::SHIELD_GENERATOR)));
			text += stringf(256, "Cargo: %dt\n", stats->used_cargo);

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
				text += "Hyperspace arrival cloud remnant";
			}
			else {
				const SystemPath dest = ship->GetHyperspaceDest();
				Sector s(dest.sectorX, dest.sectorY);
				text += stringf(512,
					"Hyperspace %s cloud\n"
					"Ship mass: %dt\n"
					"%s: %s\n"
					"Date due: %s\n",
					cloud->IsArrival() ? "arrival" : "departure",
					ship->CalcStats()->total_mass,
                    cloud->IsArrival() ? "Source" : "Destination",
					s.m_systems[dest.systemIndex].name.c_str(),
					format_date(cloud->GetDueDate()).c_str()
				);
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

	if (Pi::player->GetHyperspaceCountdown() != 0) {
		float val = Pi::player->GetHyperspaceCountdown();

		if (!(int(ceil(val*2.0)) % 2)) {
			char buf[128];
			snprintf(buf, sizeof(buf), "Hyperspacing in %.0f seconds", ceil(val));
			m_hudHyperspaceInfo->SetText(buf);
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

	if (Pi::MouseButtonState(3)) {
		// when controlling your ship with the mouse you don't want to pick targets
		m_bodyLabels->SetLabelsClickable(false);
	} else {
		m_bodyLabels->SetLabelsClickable(true);
	}

	m_bodyLabels->SetLabelsVisible(m_labelsOn);

	if (Pi::player->IsDead()) {
		m_camType = CAM_EXTERNAL;
		m_externalViewRotX += 60*frameTime;
		m_externalViewDist = 200;
		m_labelsOn = false;
		return;
	}
	if (GetCamType() == CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) m_externalViewRotX -= 45*frameTime;
		if (Pi::KeyState(SDLK_DOWN)) m_externalViewRotX += 45*frameTime;
		if (Pi::KeyState(SDLK_LEFT)) m_externalViewRotY -= 45*frameTime;
		if (Pi::KeyState(SDLK_RIGHT)) m_externalViewRotY += 45*frameTime;
		if (Pi::KeyState(SDLK_EQUALS)) m_externalViewDist -= 400*frameTime;
		if (Pi::KeyState(SDLK_MINUS)) m_externalViewDist += 400*frameTime;
		if (Pi::KeyState(SDLK_HOME)) m_externalViewDist = 200;
		m_externalViewDist = std::max(Pi::player->GetBoundingRadius(), m_externalViewDist);

		// when landed don't let external view look from below
		if (Pi::player->GetFlightState() == Ship::LANDED || Pi::player->GetFlightState() == Ship::DOCKED)
			m_externalViewRotX = Clamp(m_externalViewRotX, -170.0, -10.0);
	}
	if (KeyBindings::targetObject.IsActive()) {
		/* Hitting tab causes objects in the crosshairs to be selected */
		Body* const target = PickBody(double(Gui::Screen::GetWidth())/2.0, double(Gui::Screen::GetHeight())/2.0);
		SelectBody(target, false);
	}

}

void WorldView::OnSwitchTo()
{
	RefreshButtonStateAndVisibility();
}

void WorldView::ToggleTargetActions()
{
	if (m_showTargetActionsTimeout) m_showTargetActionsTimeout = 0;
	else m_showTargetActionsTimeout = SDL_GetTicks();
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
	std::map<Uint32, std::vector<SBody*> > groups;

	m_commsNavOptions->PackEnd(new Gui::Label("#ff0Navigation targets in this system\n"));

	for ( std::vector<SBody*>::const_iterator i = Pi::currentSystem->m_spaceStations.begin();
	      i != Pi::currentSystem->m_spaceStations.end(); i++) {

		groups[(*i)->parent->id].push_back(*i);
	}

	for ( std::vector<SBody*>::const_iterator i = Pi::currentSystem->m_bodies.begin();
	      i != Pi::currentSystem->m_bodies.end(); i++) {

		std::vector<SBody*> group = groups[(*i)->id];
		if ( group.size() == 0 ) continue;

		m_commsNavOptions->PackEnd(new Gui::Label("#f0f" + (*i)->name));

		for ( std::vector<SBody*>::const_iterator j = group.begin(); j != group.end(); j++) {
			SystemPath path = Pi::currentSystem->GetPathOf(*j);
			Body *body = Space::FindBodyForPath(&path);
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
		Pi::cpan->MsgLog()->Message("", "You do not have any money.");
	} else if (fine > Pi::player->GetMoney()) {
		Polit::AddCrime(0, -Pi::player->GetMoney());
		Polit::GetCrime(&crime, &fine);
		Pi::cpan->MsgLog()->Message("", stringf(512, "You have paid %s but still have an outstanding fine of %s.",
				format_money(Pi::player->GetMoney()).c_str(),
				format_money(fine).c_str()));
		Pi::player->SetMoney(0);
	} else {
		Pi::player->SetMoney(Pi::player->GetMoney() - fine);
		Pi::cpan->MsgLog()->Message("", stringf(512, "You have paid the fine of %s.",
				format_money(fine).c_str()));
		Polit::AddCrime(0, -fine);
	}
}

void WorldView::OnHyperspaceTargetChanged()
{
	if (Pi::player->GetHyperspaceCountdown() > 0.0) {
		Pi::player->ResetHyperspaceCountdown();
		Pi::cpan->MsgLog()->Message("", "Hyperspace jump aborted.");
	}

	const SystemPath path = Pi::sectorView->GetHyperspaceTarget();

	StarSystem *system = StarSystem::GetCached(path);
	Pi::cpan->MsgLog()->Message("", std::string("Set hyperspace destination to "+system->GetName()));
	system->Release();

	int fuelReqd;
	double dur;
	m_showHyperspaceButton = Pi::player->CanHyperspaceTo(&path, fuelReqd, dur);
}

void WorldView::OnPlayerChangeTarget()
{
	Body *b = Pi::player->GetNavTarget();
	if (b &&
		(!b->IsType(Object::HYPERSPACECLOUD) ||
		 Pi::sectorView->GetHyperspaceTarget() != static_cast<HyperspaceCloud*>(b)->GetShip()->GetHyperspaceDest()))
		Pi::sectorView->FloatHyperspaceTarget();
	UpdateCommsOptions();
}

static void autopilot_flyto(Body *b)
{
	Pi::player->SetFlightControlState(Player::CONTROL_AUTOPILOT);
	Pi::player->AIFlyTo(b);
}
static void autopilot_attack(Body *b)
{
	Pi::player->SetFlightControlState(Player::CONTROL_AUTOPILOT);
	Pi::player->AIKill(static_cast<Ship*>(b));
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
	Pi::player->SetFollowCloud(cloud);
	Pi::sectorView->SetHyperspaceTarget(cloud->GetShip()->GetHyperspaceDest());
}

void WorldView::UpdateCommsOptions()
{
	m_commsOptions->DeleteAllChildren();
	m_commsNavOptions->DeleteAllChildren();

	if (m_showTargetActionsTimeout == 0) return;

	if (Pi::currentSystem->m_spaceStations.size() > 0)
	{
		BuildCommsNavOptions();
	}

	Body * const navtarget = Pi::player->GetNavTarget();
	Body * const comtarget = Pi::player->GetCombatTarget();
	Gui::Button *button;
	int ypos = 0;
	int optnum = 1;
	if (!(navtarget || comtarget)) {
		m_commsOptions->Add(new Gui::Label("#0f0Ship Computer: No target selected"), 16, float(ypos));
	}
	if (navtarget) {
		m_commsOptions->Add(new Gui::Label("#0f0"+navtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		if (navtarget->IsType(Object::SPACESTATION)) {
			button = AddCommsOption("Request docking clearance", ypos, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&PlayerRequestDockingClearance), reinterpret_cast<SpaceStation*>(navtarget)));
			ypos += 32;

			if (Pi::player->m_equipment.Get(Equip::SLOT_AUTOPILOT) == Equip::AUTOPILOT) {
				button = AddCommsOption("Autopilot: Dock with space station", ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_dock), navtarget));
				ypos += 32;
			}

			Sint64 crime, fine;
			Polit::GetCrime(&crime, &fine);
			if (fine) {
				button = AddCommsOption(stringf(512, "Pay fine by remote transfer (%s)",
							format_money(fine).c_str()).c_str(), ypos, optnum++);
				button->onClick.connect(sigc::ptr_fun(&PlayerPayFine));
				ypos += 32;
			}
		}
		if (Pi::player->m_equipment.Get(Equip::SLOT_AUTOPILOT) == Equip::AUTOPILOT) {
			button = AddCommsOption("Autopilot: Fly to vicinity of " + navtarget->GetLabel(), ypos, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&autopilot_flyto), navtarget));
			ypos += 32;

			if (navtarget->IsType(Object::PLANET) || navtarget->IsType(Object::STAR)) {
				button = AddCommsOption("Autopilot: Enter low orbit around " + navtarget->GetLabel(), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 1.1));
				ypos += 32;

				button = AddCommsOption("Autopilot: Enter medium orbit around " + navtarget->GetLabel(), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 2.0));
				ypos += 32;

				button = AddCommsOption("Autopilot: Enter high orbit around " + navtarget->GetLabel(), ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_orbit), navtarget, 5.0));
				ypos += 32;
			}
		}

		const Equip::Type t = Pi::player->m_equipment.Get(Equip::SLOT_HYPERCLOUD);
		if ((t != Equip::NONE) && navtarget->IsType(Object::HYPERSPACECLOUD)) {
			HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(navtarget);
			if (!cloud->IsArrival()) {
				button = AddCommsOption("Hyperspace cloud analyzer: Set hyperspace target to follow this departure", ypos, optnum++);
				button->onClick.connect(sigc::bind(sigc::ptr_fun(player_target_hypercloud), cloud));
			}
		}
#if 0
		Frame *f = navtarget->GetFrame();
		SBody *b = f->GetSBodyFor();
		if (b) {
			SBodyPath path;
			Pi::currentSystem->GetPathOf(b, &path);
			std::string msg = "Set hyperspace target to " + navtarget->GetLabel();
			button = AddCommsOption(msg, ypos, optnum++);
			button->onClick.connect(sigc::bind(sigc::ptr_fun(&OnPlayerSetHyperspaceTargetTo), path));
			ypos += 32;
		}
#endif
	}
	if (comtarget) {
		m_commsOptions->Add(new Gui::Label("#f00"+comtarget->GetLabel()), 16, float(ypos));
		ypos += 32;
		button = AddCommsOption("Autopilot: Fly to vicinity of "+comtarget->GetLabel(), ypos, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_flyto), comtarget));
		ypos += 32;
        /*
		button = AddCommsOption("Autopilot: Attack "+comtarget->GetLabel(), ypos, optnum++);
		button->onClick.connect(sigc::bind(sigc::ptr_fun(autopilot_attack), comtarget));
		ypos += 32;
        */
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
			Pi::player->SetCombatTarget(target);
		}
	} else {
		if (Pi::player->GetNavTarget() == target) {
			if (reselectIsDeselect) Pi::player->SetNavTarget(0);
		} else {
			Pi::player->SetNavTarget(target);
		}
	}
}

Body* WorldView::PickBody(const double screenX, const double screenY) const
{
	Body *selected = 0;

	for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		Body *b = *i;
		if(b->IsOnscreen() && (b != Pi::player)) {
			const vector3d& _pos = b->GetProjectedPos();
			const double x1 = _pos.x - PICK_OBJECT_RECT_SIZE * 0.5;
			const double x2 = x1 + PICK_OBJECT_RECT_SIZE;
			const double y1 = _pos.y - PICK_OBJECT_RECT_SIZE * 0.5;
			const double y2 = y1 + PICK_OBJECT_RECT_SIZE;
			if(screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2) {
				selected = b;
				break;
			}
		}
	}

	return selected;
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

void WorldView::ProjectObjsToScreenPos(const Frame *cam_frame)
{
	Gui::Screen::EnterOrtho();		// To save matrices

	matrix4x4d cam_rot = cam_frame->GetTransform();
	cam_rot.ClearToRotOnly();
	
	{
		// Direction indicator
		vector3d vel = Pi::player->GetVelocityRelTo(Pi::player->GetFrame());
			// XXX ^ not the same as GetVelocity(), because it considers
			// the stasis velocity of a rotating frame

		vector3d vdir = vel * cam_rot;			// transform to camera space
		m_velocityIndicatorOnscreen = false;
		if (vdir.z < -1.0) {					// increase this maybe
			vector3d pos;
			if (Gui::Screen::Project(vdir, pos)) {
				m_velocityIndicatorPos[0] = int(pos.x);		// integers eh
				m_velocityIndicatorPos[1] = int(pos.y);
				m_velocityIndicatorOnscreen = true;
			}
		}
	}
	
	m_navVelocityIndicatorOnscreen = false;
	if (Body *navtarget = Pi::player->GetNavTarget()) {
		// nav target direction indicator
		vector3d vel = Pi::player->GetVelocityRelTo(navtarget);

		vector3d vdir = vel * cam_rot;			// transform to camera space
		if (vdir.z < -1.0) {					// increase this maybe
			vector3d pos;
			if (Gui::Screen::Project(vdir, pos)) {
				m_navVelocityIndicatorPos[0] = int(pos.x);		// integers eh
				m_navVelocityIndicatorPos[1] = int(pos.y);
				m_navVelocityIndicatorOnscreen = true;
			}
		}
	}

	// test code for mousedir
/*	vector3d mdir = Pi::player->GetMouseDir() * cam_rot;
	if (mdir.z < 0) {
		vector3d pos;
		if (Gui::Screen::Project(mdir, pos)) {
			m_velocityIndicatorPos[0] = (int)pos.x;
			m_velocityIndicatorPos[1] = (int)pos.y;
			m_velocityIndicatorOnscreen = true;
		}
	}
*/
	// Update object onscreen positions
	{
		m_bodyLabels->Clear();
		for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
			if ((GetCamType() != WorldView::CAM_EXTERNAL) && (*i == Pi::player)) continue;
			Body *b = *i;
			b->SetOnscreen(false);
			vector3d pos = b->GetInterpolatedPositionRelTo(cam_frame);
			if (pos.z < 0 && Gui::Screen::Project(pos, pos)) {
				b->SetProjectedPos(pos);
				b->SetOnscreen(true);
				// Ok here we are hiding the label of distant small objects.
				// If you are not a planet, star, space station or remote city
				// and you are > 1000km away then bugger off. :)
				if (b->IsType(Object::PLANET) || b->IsType(Object::STAR) || b->IsType(Object::SPACESTATION) ||
					Pi::player->GetPositionRelTo(b).LengthSqr() < 1000000.0*1000000.0) {

					m_bodyLabels->Add((*i)->GetLabel(), sigc::bind(sigc::mem_fun(this, &WorldView::SelectBody), *i, true), float(pos.x), float(pos.y));
				}
			}
		}
	}

	// update navtarget distance
	Body *navtarget = Pi::player->GetNavTarget();
	if (navtarget && navtarget->IsOnscreen())
	{
		double dist = Pi::player->GetPositionRelTo(navtarget).Length();
		m_targetDist->SetText(format_distance(dist).c_str());

		m_targetDist->Color(0.0f, 1.0f, 0.0f);

		vector3d lpos = navtarget->GetProjectedPos() + vector3d(-10,12,0);
		MoveChild(m_targetDist, float(lpos.x), float(lpos.y));

		m_targetDist->Show();
	}
	else
		m_targetDist->Hide();
	
	// update navtarget speed
	if (m_navVelocityIndicatorOnscreen) {
		double vel = Pi::player->GetVelocityRelTo(navtarget).Length();
		char buf[128];
		if (vel > 1000)
			snprintf(buf, sizeof(buf), "%.2f km/s", vel*0.001);
		else
			snprintf(buf, sizeof(buf), "%.0f m/s", vel);
		m_targetSpeed->SetText(buf);

		m_targetSpeed->Color(0.0f, 1.0f, 0.0f);

		MoveChild(m_targetSpeed, m_navVelocityIndicatorPos[0]-26, m_navVelocityIndicatorPos[1]+26);

		m_targetSpeed->Show();
	}
	else
		m_targetSpeed->Hide();

	// update combat HUD
	Ship *enemy = static_cast<Ship *>(Pi::player->GetCombatTarget());
	m_targLeadOnscreen = false;
	m_combatDist->Hide();
	m_combatSpeed->Hide();
	if (GetCamType() == CAM_FRONT && enemy && enemy->IsOnscreen())
	{
		vector3d targpos = enemy->GetInterpolatedPositionRelTo(cam_frame);	// transforms to object space?
		matrix4x4d prot = cam_frame->GetTransform(); prot[12] = prot[13] = prot[14] = 0.0;
		vector3d targvel = enemy->GetVelocityRelTo(Pi::player) * prot;

		int laser = Equip::types[Pi::player->m_equipment.Get(Equip::SLOT_LASER, 0)].tableIndex;
		double projspeed = Equip::lasers[laser].speed;
		vector3d leadpos = targpos + targvel*(targpos.Length()/projspeed);
		leadpos = targpos + targvel*(leadpos.Length()/projspeed); 	// second order approx
		double dist = targpos.Length();

		if (leadpos.z < 0.0 && dist < 100000 && Gui::Screen::Project(leadpos, m_targLeadPos))
			m_targLeadOnscreen = true;

		// now the text speed/distance
		// want to calculate closing velocity that you couldn't counter with retros

		double vel = targvel.z;				// position should be towards
		double raccel = Pi::player->GetShipType().linThrust[ShipType::THRUSTER_REVERSE]
			/ Pi::player->GetMass();

		double c = vel / sqrt(2.0 * raccel * dist);
		if (c > 1.0) c = 1.0; if (c < -1.0) c = -1.0;
		float r = float(0.2+(c+1.0)*0.4);
		float b = float(0.2+(1.0-c)*0.4);
		char buf[1024];
			
		m_combatDist->Color(r, 0.0f, b);
		sprintf(buf, "%.0fm", dist);
		m_combatDist->SetText(buf);
		vector3d lpos = enemy->GetProjectedPos() + vector3d(20,30,0);
		MoveChild(m_combatDist, float(lpos.x), float(lpos.y));
		m_combatDist->Show();

		m_combatSpeed->Color(r, 0.0f, b);
		sprintf(buf, "%0.fm/s", vel);
		m_combatSpeed->SetText(buf);
		lpos = enemy->GetProjectedPos() + vector3d(20,44,0);
		MoveChild(m_combatSpeed, float(lpos.x), float(lpos.y));
		m_combatSpeed->Show();
	}
	Gui::Screen::LeaveOrtho();		// To save matrices
}

void WorldView::Draw()
{
	View::Draw();

	// don't draw crosshairs etc in hyperspace
	if (Pi::player->GetFlightState() == Ship::HYPERSPACE) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glEnable(GL_BLEND);

	const float sz = HUD_CROSSHAIR_SIZE;
	// velocity indicator
	if (m_velocityIndicatorOnscreen) {
		const int *pos = m_velocityIndicatorPos;
		GLfloat vtx[16] = {
			pos[0]-sz, pos[1]-sz,
			pos[0]-0.5f*sz, pos[1]-0.5f*sz,
			pos[0]+sz, pos[1]-sz,
			pos[0]+0.5f*sz, pos[1]-0.5f*sz,
			pos[0]+sz, pos[1]+sz,
			pos[0]+0.5f*sz, pos[1]+0.5f*sz,
			pos[0]-sz, pos[1]+sz,
			pos[0]-0.5f*sz, pos[1]+0.5f*sz };
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINES, 0, 8);
	}

	// normal crosshairs
	if (GetCamType() == WorldView::CAM_FRONT) {
		float px = float(Gui::Screen::GetWidth())/2.0f;
		float py = float(Gui::Screen::GetHeight())/2.0f;
		GLfloat vtx[16] = {
			px-sz, py,
			px-0.5f*sz, py,
			px+sz, py,
			px+0.5f*sz, py,
			px, py-sz,
			px, py-0.5f*sz,
			px, py+sz,
			px, py+0.5f*sz };
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINES, 0, 8);
	} else if (GetCamType() == WorldView::CAM_REAR) {
		float px = float(Gui::Screen::GetWidth())/2.0f;
		float py = float(Gui::Screen::GetHeight())/2.0f;
		const float szH = 0.5*HUD_CROSSHAIR_SIZE;
		GLfloat vtx[16] = {
			px-szH, py,
			px-0.5f*szH, py,
			px+szH, py,
			px+0.5f*szH, py,
			px, py-szH,
			px, py-0.5f*szH,
			px, py+szH,
			px, py+0.5f*szH };
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINES, 0, 8);
	}

	// nav target velocity indicator
	if (m_navVelocityIndicatorOnscreen) {
		const int *pos = m_navVelocityIndicatorPos;
		glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
		GLfloat vtx[16] = {
			pos[0]-sz, pos[1]-sz,
			pos[0]-0.5f*sz, pos[1]-0.5f*sz,
			pos[0]+sz, pos[1]-sz,
			pos[0]+0.5f*sz, pos[1]-0.5f*sz,
			pos[0]+sz, pos[1]+sz,
			pos[0]+0.5f*sz, pos[1]+0.5f*sz,
			pos[0]-sz, pos[1]+sz,
			pos[0]-0.5f*sz, pos[1]+0.5f*sz };
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINES, 0, 8);
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	DrawTargetSquares();

	glDisable(GL_BLEND);
}

void WorldView::DrawTargetSquares()
{
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
	glLineWidth(2.0f);

	if(Pi::player->GetNavTarget()) {
		glColor3f(0.0f, 1.0f, 0.0f);
		DrawTargetSquare(Pi::player->GetNavTarget());
	}

	if(Pi::player->GetCombatTarget()) {
//		glColor3f(1.0f, 0.0f, 0.0f);
//		DrawTargetSquare(Pi::player->GetCombatTarget());

// ok, let's put the fancy stuff in here

		DrawCombatTargetIndicator(static_cast<Ship *>(Pi::player->GetCombatTarget()));
	}

	glPopAttrib();
}


void WorldView::DrawCombatTargetIndicator(const Ship* const target)
{
	if (!target->IsOnscreen()) return;
	vector3d pos1 = target->GetProjectedPos();
	vector3d pos2 = m_targLeadPos;
	vector3d dir = (pos2 - pos1); dir.z = 0.0;
	if (dir.Length() == 0.0 || !m_targLeadOnscreen) dir = vector3d(1,0,0);
	else dir = dir.Normalized();

	float x1 = float(pos1.x), y1 = float(pos1.y);
	float x2 = float(pos2.x), y2 = float(pos2.y);
	float xd = float(dir.x), yd = float(dir.y);

	glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
	GLfloat vtx[28] = {
		x1+10*xd, y1+10*yd,	x1+20*xd, y1+20*yd,		// target crosshairs
		x1-10*xd, y1-10*yd,	x1-20*xd, y1-20*yd,
		x1-10*yd, y1+10*xd,	x1-20*yd, y1+20*xd,
		x1+10*yd, y1-10*xd,	x1+20*yd, y1-20*xd,

		x2-10*xd, y2-10*yd,	x2+10*xd, y2+10*yd,		// lead crosshairs
		x2-10*yd, y2+10*xd,	x2+10*yd, y2-10*xd,

		x1+20*xd, y1+20*yd,	x2-10*xd, y2-10*yd,		// line between crosshairs
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glDrawArrays(GL_LINES, 0, 8);
	if (m_targLeadOnscreen) glDrawArrays(GL_LINES, 8, 6);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void WorldView::DrawTargetSquare(const Body* const target)
{
	if(target->IsOnscreen()) {
		const vector3d& _pos = target->GetProjectedPos();
		const float x1 = float(_pos.x - WorldView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float x2 = float(x1 + WorldView::PICK_OBJECT_RECT_SIZE);
		const float y1 = float(_pos.y - WorldView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float y2 = float(y1 + WorldView::PICK_OBJECT_RECT_SIZE);

		GLfloat vtx[8] = {
			x1, y1,
			x2, y1,
			x2, y2,
			x1, y2 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void WorldView::MouseButtonDown(int button, int x, int y)
{
	if (GetCamType() == CAM_EXTERNAL)
	{
		const float ft = Pi::GetFrameTime();
		if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN))
				m_externalViewDist += 400*ft;
		if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP))
				m_externalViewDist -= 400*ft;
	}
}
