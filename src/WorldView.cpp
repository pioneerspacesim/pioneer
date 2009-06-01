#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"
#include "SpaceStation.h"
#include "ShipCpanel.h"
#include "Serializer.h"
#include "StarSystem.h"

const float WorldView::PICK_OBJECT_RECT_SIZE = 20.0f;

#define BG_STAR_MAX	5000

WorldView::WorldView(): View()
{
	float size[2];
	GetSize(size);
	
	m_labelsOn = true;
	m_camType = CAM_FRONT;
	SetTransparency(true);
	m_externalViewRotX = m_externalViewRotY = 0;
	m_externalViewDist = 200;
	
	m_commsOptions = new Fixed(size[0], size[1]/2);
	m_commsOptions->SetTransparency(true);
	Add(m_commsOptions, 10, 200);

	Gui::MultiStateImageButton *wheels_button = new Gui::MultiStateImageButton();
	wheels_button->SetShortcut(SDLK_F6, KMOD_NONE);
	wheels_button->AddState(0, "icons/wheels_up.png", "Wheels up");
	wheels_button->AddState(1, "icons/wheels_down.png", "Wheels down");
	wheels_button->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeWheelsState));
	m_rightButtonBar->Add(wheels_button, 34, 2);

	Gui::MultiStateImageButton *labels_button = new Gui::MultiStateImageButton();
	labels_button->SetShortcut(SDLK_F8, KMOD_NONE);
	labels_button->AddState(1, "icons/labels_on.png", "Object labels on");
	labels_button->AddState(0, "icons/labels_off.png", "Object labels off");
	labels_button->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeLabelsState));
	m_rightButtonBar->Add(labels_button, 98, 2);

	m_hyperspaceButton = new Gui::ImageButton("icons/hyperspace_f8.png");
	m_hyperspaceButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_hyperspaceButton->SetToolTip("Hyperspace Jump");
	m_hyperspaceButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickHyperspace));
	m_rightButtonBar->Add(m_hyperspaceButton, 66, 2);

	m_launchButton = new Gui::ImageButton("icons/blastoff.png");
	m_launchButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_launchButton->SetToolTip("Takeoff");
	m_launchButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnClickBlastoff));
	m_rightButtonBar->Add(m_launchButton, 2, 2);

	m_flightControlButton = new Gui::MultiStateImageButton();
	m_flightControlButton->SetShortcut(SDLK_F5, KMOD_NONE);
	m_flightControlButton->AddState(Player::CONTROL_MANUAL, "icons/manual_control.png", "Manual control");
	m_flightControlButton->AddState(Player::CONTROL_FIXSPEED, "icons/manual_control.png", "Computer speed control");
	m_flightControlButton->AddState(Player::CONTROL_AUTOPILOT, "icons/autopilot.png", "Autopilot on");
	m_flightControlButton->onClick.connect(sigc::mem_fun(this, &WorldView::OnChangeFlightState));
	m_rightButtonBar->Add(m_flightControlButton, 2, 2);
	
	m_flightStatus = new Gui::Label("");
	m_flightStatus->SetColor(1,.7,0);
	m_rightRegion2->Add(m_flightStatus, 10, 0);

	m_hyperTargetLabel = new Gui::Label("");
	m_hyperTargetLabel->SetColor(1,.7,0);
	m_rightRegion1->Add(m_hyperTargetLabel, 10, 0);
	
	Pi::onPlayerChangeHyperspaceTarget.connect(sigc::mem_fun(this, &WorldView::OnChangeHyperspaceTarget));
	
	m_bgstarsDlist = glGenLists(1);

	glNewList(m_bgstarsDlist, GL_COMPILE);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPointSize(1.0);
	glBegin(GL_POINTS);
	for (int i=0; i<BG_STAR_MAX; i++) {
		float col = 0.05f+(float)Pi::rng.NDouble(3);
		col = CLAMP(col, 0, 1);
		glColor3f(col, col, col);
		glVertex3f( (float)(1000-Pi::rng.Double(2000.0)),
			(float)(1000-Pi::rng.Double(2000.0)),
			(float)(1000-Pi::rng.Double(2000.0)) );
	}
	glEnd();
	glPopAttrib();

	glEndList();
}

void WorldView::Save()
{
	using namespace Serializer::Write;
	wr_float(m_externalViewRotX);
	wr_float(m_externalViewRotY);
	wr_float(m_externalViewDist);
	wr_int((int)m_camType);
}

void WorldView::Load()
{
	using namespace Serializer::Read;
	m_externalViewRotX = rd_float();
	m_externalViewRotY = rd_float();
	m_externalViewDist = rd_float();
	m_camType = (CamType)rd_int();
}

void WorldView::SetCamType(enum CamType c)
{
	m_camType = c;
}

vector3d WorldView::GetExternalViewTranslation()
{
	vector3d p = vector3d(0, 0, m_externalViewDist);
	p = matrix4x4d::RotateXMatrix(-DEG2RAD(m_externalViewRotX)) * p;
	p = matrix4x4d::RotateYMatrix(-DEG2RAD(m_externalViewRotY)) * p;
	matrix4x4d m;
	Pi::player->GetRotMatrix(m);
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
	if (!Pi::player->SetWheelState(b->GetState()!=0)) {
		b->StatePrev();
	}
}

void WorldView::OnChangeFlightState(Gui::MultiStateImageButton *b)
{
	Pi::player->SetFlightControlState(static_cast<Player::FlightControlState>(b->GetState()));
}

void WorldView::OnChangeLabelsState(Gui::MultiStateImageButton *b)
{
	m_labelsOn = b->GetState()!=0;
}

void WorldView::OnClickBlastoff()
{
	Pi::player->Blastoff();
}

void WorldView::OnClickHyperspace()
{
	const SBodyPath *path = Pi::player->GetHyperspaceTarget();
	Pi::HyperspaceTo(path);
}

void WorldView::DrawBgStars()
{
	glCallList(m_bgstarsDlist);
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
		vector3d lpos = (m * vector3d(0,0,0)).Normalized();
		float lightPos[4];
		lightPos[0] = (float)lpos.x;
		lightPos[1] = (float)lpos.y;
		lightPos[2] = (float)lpos.z;
		lightPos[3] = 0;
		
		const float *col = StarSystem::starRealColors[body->type];
		float lightCol[4] = { col[0], col[1], col[2], 0 };
		float ambCol[4] = { col[0]*0.1f, col[1]*0.1f, col[2]*0.1f, 0 };

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

void WorldView::Draw3D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// why the hell do i give these functions such big names..
	float fracH = WORLDVIEW_ZNEAR / Pi::GetScrAspect();
	glFrustum(-WORLDVIEW_ZNEAR, WORLDVIEW_ZNEAR, -fracH, fracH, WORLDVIEW_ZNEAR, WORLDVIEW_ZFAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(Pi::player) {
		// make temporary camera frame at player
		Frame cam_frame(Pi::player->GetFrame(), "", Frame::TEMP_VIEWING);

		matrix4x4d camRot = matrix4x4d::Identity();

		if (m_camType == CAM_FRONT) {
			cam_frame.SetPosition(Pi::player->GetPosition());
		} else if (m_camType == CAM_REAR) {
			camRot.RotateY(M_PI);
		//	glRotatef(180.0f, 0, 1, 0);
			cam_frame.SetPosition(Pi::player->GetPosition());
		} else /* CAM_EXTERNAL */ {
			cam_frame.SetPosition(Pi::player->GetPosition() + GetExternalViewTranslation());
			ApplyExternalViewRotation(camRot);
		}

		{
			matrix4x4d prot;
			Pi::player->GetRotMatrix(prot);
			camRot = prot * camRot;
		}
		cam_frame.SetOrientation(camRot);
		
		matrix4x4d trans2bg;
		Frame::GetFrameTransform(Space::rootFrame, &cam_frame, trans2bg);
		trans2bg.ClearToRotOnly();
		glPushMatrix();
		glMultMatrixd(&trans2bg[0]);
		DrawBgStars();
		glPopMatrix();

		int l=0;
		position_system_lights(&cam_frame, Space::rootFrame, l);
		Space::Render(&cam_frame);
		Pi::player->DrawHUD(&cam_frame);

		Pi::player->GetFrame()->RemoveChild(&cam_frame);
	}
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
}

void WorldView::Update()
{
	const float frameTime = Pi::GetFrameTime();

	if (GetCamType() == CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) m_externalViewRotX -= 45*frameTime;
		if (Pi::KeyState(SDLK_DOWN)) m_externalViewRotX += 45*frameTime;
		if (Pi::KeyState(SDLK_LEFT)) m_externalViewRotY -= 45*frameTime;
		if (Pi::KeyState(SDLK_RIGHT)) m_externalViewRotY += 45*frameTime;
		if (Pi::KeyState(SDLK_EQUALS)) m_externalViewDist -= 400*frameTime;
		if (Pi::KeyState(SDLK_MINUS)) m_externalViewDist += 400*frameTime;
		m_externalViewDist = MAX(50, m_externalViewDist);

		// when landed don't let external view look from below
		if (Pi::player->GetFlightState() == Ship::LANDED) m_externalViewRotX = CLAMP(m_externalViewRotX, -170.0f, -10.0f);
	}

	if (!Pi::player->GetDockedWith()) {
		m_hyperspaceButton->Show();
	} else {
		m_hyperspaceButton->Hide();
	}

	Body *target = Pi::player->GetNavTarget();
	if (target) {
		m_commsOptions->ShowAll();
	} else {
		//m_commsOptions->HideAll();
	}

	if (Pi::player->GetFlightState() == Ship::LANDED) {
		m_flightStatus->SetText("Landed");
		m_launchButton->Show();
		m_flightControlButton->Hide();
	} else {
		Player::FlightControlState fstate = Pi::player->GetFlightControlState();
		switch (fstate) {
			case Player::CONTROL_MANUAL:
				m_flightStatus->SetText("Manual Control"); break;
			case Player::CONTROL_FIXSPEED:
				m_flightStatus->SetText("Speed Control"); break;
			case Player::CONTROL_AUTOPILOT:
				m_flightStatus->SetText("Autopilot"); break;
		}
		m_launchButton->Hide();
		m_flightControlButton->Show();
	}
}

Gui::Button *WorldView::AddCommsOption(std::string msg, int ypos)
{
	Gui::Label *l = new Gui::Label(msg);
	m_commsOptions->Add(l, 50, (float)ypos);

	Gui::TransparentButton *b = new Gui::TransparentButton();
	m_commsOptions->Add(b, 16, (float)ypos);
	return b;
}

static void PlayerRequestDockingClearance(SpaceStation *s)
{
	s->GetDockingClearance(Pi::player);
	Pi::cpan->SetTemporaryMessage(s, "Docking clearance granted.");
}

static void OnPlayerSetHyperspaceTargetTo(SBodyPath path)
{
	Pi::player->SetHyperspaceTarget(&path);
}
	
void WorldView::OnChangeHyperspaceTarget()
{
	const SBodyPath *path = Pi::player->GetHyperspaceTarget();
	StarSystem *system = new StarSystem(path->sectorX, path->sectorY, path->systemIdx);
	SBody *b = system->GetBodyByPath(path);
	Pi::cpan->SetTemporaryMessage(0, std::string("Set hyperspace destination to ")+b->name);
	std::string msg = "To: "+b->name;
	m_hyperTargetLabel->SetText(msg);
	delete system;
		
	int fuelReqd;
	if (Pi::player->CanHyperspaceTo(path, fuelReqd)) m_hyperspaceButton->Show();
	else m_hyperspaceButton->Hide();
}

void WorldView::UpdateCommsOptions()
{
	Body * const navtarget = Pi::player->GetNavTarget();
	m_commsOptions->DeleteAllChildren();
	
	int ypos = 0;
	if (navtarget) {
		m_commsOptions->Add(new Gui::Label(navtarget->GetLabel()), 16, (float)ypos);
		ypos += 32;
		if (navtarget->IsType(Object::SPACESTATION)) {
			Gui::Button *b = AddCommsOption("Request docking clearance", ypos);
			b->onClick.connect(sigc::bind(sigc::ptr_fun(&PlayerRequestDockingClearance), (SpaceStation*)navtarget));
			ypos += 32;
		} else {
			std::string msg = "Do something to "+navtarget->GetLabel();
			Gui::Button *b = AddCommsOption(msg, ypos);
			ypos += 32;
		}
		Frame *f = navtarget->GetFrame();
		SBody *b = f->GetSBodyFor();
		if (b) {
			SBodyPath path;
			Pi::currentSystem->GetPathOf(b, &path);
			std::string msg = "Set hyperspace target to " + navtarget->GetLabel();
			Gui::Button *b = AddCommsOption(msg, ypos);
			b->onClick.connect(sigc::bind(sigc::ptr_fun(&OnPlayerSetHyperspaceTargetTo), path));
			ypos += 32;
		}
	}
}

bool WorldView::OnMouseDown(Gui::MouseButtonEvent *e)
{
	// if continuing to propagate mouse event, see if target is clicked on
	if (Container::OnMouseDown(e)) {
		if(1 == e->button && !Pi::MouseButtonState(3)) {
			// Left click in view when RMB not pressed => Select target.
			Body* const target = PickBody(e->screenX, e->screenY);
			
			if (!target) {

			} else if (target->IsType(Object::SHIP)) {
				if (Pi::player->GetCombatTarget() == target)
					Pi::player->SetCombatTarget(0);
				else
					Pi::player->SetCombatTarget(target);
			} else {
				if (Pi::player->GetNavTarget() == target)
					Pi::player->SetNavTarget(0);
				else
					Pi::player->SetNavTarget(target);
			}
		}
	}
	return true;
}

Body* WorldView::PickBody(const float screenX, const float screenY) const
{
	Body *selected = 0;

	for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		Body *b = *i;
		if(b->IsOnscreen()) {
			const vector3d& _pos = b->GetProjectedPos();
			const float x1 = (float)_pos.x - PICK_OBJECT_RECT_SIZE * 0.5f;
			const float x2 = x1 + PICK_OBJECT_RECT_SIZE;
			const float y1 = (float)_pos.y - PICK_OBJECT_RECT_SIZE * 0.5f;
			const float y2 = y1 + PICK_OBJECT_RECT_SIZE;
			if(screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2) {
				selected = b;
				break;
			}			
		}
	}

	return selected;
}
