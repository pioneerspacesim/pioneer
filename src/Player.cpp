#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "Space.h"
#include "Gui.h"
#include "WorldView.h"
#include "SpaceStationView.h"
#include "Serializer.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseCMov[0] = m_mouseCMov[1] = 0;
	m_flightControlState = CONTROL_MANUAL;
	UpdateMass();
}

Player::~Player()
{
	assert(this == Pi::player);
	Pi::player = 0;
}

void Player::Save()
{
	using namespace Serializer::Write;
	Ship::Save();
	wr_int(static_cast<int>(m_flightControlState));
	wr_float(m_setSpeed);
}

void Player::Load()
{
	using namespace Serializer::Read;
	Ship::Load();
	m_flightControlState = static_cast<FlightControlState>(rd_int());
	m_setSpeed = rd_float();
}

void Player::SetFlightControlState(enum FlightControlState s)
{
	m_flightControlState = s;
	if (m_flightControlState == CONTROL_AUTOPILOT) {
		Body *target = GetNavTarget();
		AIClearInstructions();
		if (target && target->IsType(Object::SHIP)) {
			AIInstruct(Ship::DO_KILL, target);
		} else if (target) {
			AIInstruct(Ship::DO_KILL, target);
		}
	} else if (m_flightControlState == CONTROL_FIXSPEED) {
		AIClearInstructions();
		m_setSpeed = GetVelocity().Length();
	} else {
		AIClearInstructions();
	}
}

void Player::Render(const Frame *camFrame)
{
	if (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL) {
		Ship::Render(camFrame);
	} else {
		glPushMatrix();
		// could only rotate, since transform is zero (camFrame is at player origin)
		RenderLaserfire();
		glPopMatrix();
	}
}

void Player::SetDockedWith(SpaceStation *s, int port)
{
	Ship::SetDockedWith(s, port);
	if (s) {
		Pi::SetView(Pi::spaceStationView);
	}
}

void Player::TimeStepUpdate(const float timeStep)
{
	ClearThrusterState();
	polledControlsThisTurn = false;
	if (Pi::GetView() == Pi::worldView) PollControls();

	if (GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_MANUAL:
			// when world view not selected
			if (!polledControlsThisTurn) {
				const float time_accel = Pi::GetTimeAccel();
				const float ta2 = time_accel*time_accel;
				// still must apply rotation damping
				vector3d damping = CalcRotDamping();
				damping *= 1.0f/ta2;
				SetAngThrusterState(0, -damping.x);
				SetAngThrusterState(1, -damping.y);
				SetAngThrusterState(2, -damping.z);
			}
			break;
		case CONTROL_FIXSPEED:
			AIAccelToModelRelativeVelocity(vector3d(0,0,-m_setSpeed));
			break;
		case CONTROL_AUTOPILOT:
			break;
		}
	} else {
		m_flightControlState = CONTROL_MANUAL;
		AIClearInstructions();
	}
	Ship::TimeStepUpdate(timeStep);
}

#define MOUSE_CTRL_AREA		10.0f
#define MOUSE_RESTITUTION	0.75f

void Player::PollControls()
{
	int mouseMotion[2];
	const float frameTime = Pi::GetFrameTime();
	float time_accel = Pi::GetTimeAccel();
	float ta2 = time_accel*time_accel;

	polledControlsThisTurn = true;

	if (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) Pi::worldView->m_externalViewRotX -= 45*frameTime;
		if (Pi::KeyState(SDLK_DOWN)) Pi::worldView->m_externalViewRotX += 45*frameTime;
		if (Pi::KeyState(SDLK_LEFT)) Pi::worldView->m_externalViewRotY -= 45*frameTime;
		if (Pi::KeyState(SDLK_RIGHT)) Pi::worldView->m_externalViewRotY += 45*frameTime;
		if (Pi::KeyState(SDLK_EQUALS)) Pi::worldView->m_externalViewDist -= 400*frameTime;
		if (Pi::KeyState(SDLK_MINUS)) Pi::worldView->m_externalViewDist += 400*frameTime;
		Pi::worldView->m_externalViewDist = MAX(50, Pi::worldView->m_externalViewDist);

		// when landed don't let external view look from below
		if (GetFlightState() == LANDED) Pi::worldView->m_externalViewRotX = CLAMP(Pi::worldView->m_externalViewRotX, -170.0, -10);
	}

	if ((time_accel == 0) || GetDockedWith() ||
	    (GetFlightState() != FLYING)) {
		return;
	}

	/* if flying */
	{
		ClearThrusterState();
		
		vector3f angThrust(0.0f);

		if (Pi::MouseButtonState(3)) {
			Pi::GetMouseMotion(mouseMotion);
			m_mouseCMov[0] += mouseMotion[0];
			m_mouseCMov[1] += mouseMotion[1];
			m_mouseCMov[0] = CLAMP(m_mouseCMov[0]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			m_mouseCMov[1] = CLAMP(m_mouseCMov[1]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			angThrust.y = -m_mouseCMov[0] / MOUSE_CTRL_AREA;
			angThrust.x = m_mouseCMov[1] / MOUSE_CTRL_AREA;
		}
		
		if (m_flightControlState == CONTROL_FIXSPEED) {
			if (Pi::KeyState(SDLK_RETURN)) m_setSpeed += MAX(m_setSpeed*0.05, 1.0);
			if (Pi::KeyState(SDLK_RSHIFT)) m_setSpeed -= MAX(m_setSpeed*0.05, 1.0);
		}
		if (Pi::KeyState(SDLK_w)) SetThrusterState(ShipType::THRUSTER_REAR, 1.0f);
		if (Pi::KeyState(SDLK_s)) SetThrusterState(ShipType::THRUSTER_FRONT, 1.0f);
		if (Pi::KeyState(SDLK_2)) SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
		if (Pi::KeyState(SDLK_x)) SetThrusterState(ShipType::THRUSTER_BOTTOM, 1.0f);
		if (Pi::KeyState(SDLK_a)) SetThrusterState(ShipType::THRUSTER_LEFT, 1.0f);
		if (Pi::KeyState(SDLK_d)) SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0f);

		if (Pi::KeyState(SDLK_SPACE) || (Pi::MouseButtonState(1) && Pi::MouseButtonState(3))) {
			if (Pi::worldView->GetCamType() == WorldView::CAM_REAR) SetGunState(1,1);
			else SetGunState(0,1);
		} else {
			SetGunState(0,0);
			SetGunState(1,0);
		}
		
		if (Pi::worldView->GetCamType() != WorldView::CAM_EXTERNAL) {
			if (Pi::KeyState(SDLK_LEFT)) angThrust.y += 1;
			if (Pi::KeyState(SDLK_RIGHT)) angThrust.y += -1;
			if (Pi::KeyState(SDLK_UP)) angThrust.x += -1;
			if (Pi::KeyState(SDLK_DOWN)) angThrust.x += 1;
		}
		if (Pi::KeyState(SDLK_q)) angThrust.z += 1;
		if (Pi::KeyState(SDLK_e)) angThrust.z -= 1;
		// rotation damping.
		vector3d damping = time_accel*CalcRotDamping();

		angThrust.x -= damping.x;
		angThrust.y -= damping.y;
		angThrust.z -= damping.z;

		// dividing by time step so controls don't go totally mental when
		// used at 10x accel
		angThrust *= 1.0f/ta2;
		SetAngThrusterState(0, angThrust.x);
		SetAngThrusterState(1, angThrust.y);
		SetAngThrusterState(2, angThrust.z);
	}
}

#define HUD_CROSSHAIR_SIZE	24.0f

void Player::DrawHUD(const Frame *cam_frame)
{
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	GLint viewport[4];

	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv (GL_VIEWPORT, viewport);

	Gui::Screen::EnterOrtho();
	glColor3f(.7,.7,.7);

	// Object labels
	{
		for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
			if ((Pi::worldView->GetCamType() != WorldView::CAM_EXTERNAL) && (*i == this)) continue;
			Body *b = *i;
			vector3d _pos = b->GetPositionRelTo(cam_frame);

			if (_pos.z < 0
				&& Gui::Screen::Project (_pos.x,_pos.y,_pos.z, modelMatrix, projMatrix, viewport, &_pos.x, &_pos.y, &_pos.z)) {
				b->SetProjectedPos(_pos);
				b->SetOnscreen(true);
				if (Pi::worldView->GetShowLabels()) Gui::Screen::RenderLabel(b->GetLabel(), _pos.x, _pos.y);
			}
			else
				b->SetOnscreen(false);
		}
	}

	DrawTargetSquares();

	// Direction indicator
	const float sz = HUD_CROSSHAIR_SIZE;
	const vector3d vel = GetVelocity();
	vector3d loc_v = cam_frame->GetOrientation().InverseOf() * vel;
	if (loc_v.z < 0) {
		GLdouble pos[3];
		if (Gui::Screen::Project (loc_v[0],loc_v[1],loc_v[2], modelMatrix, projMatrix, viewport, &pos[0], &pos[1], &pos[2])) {
			glBegin(GL_LINES);
			glVertex2f(pos[0]-sz, pos[1]-sz);
			glVertex2f(pos[0]-0.5*sz, pos[1]-0.5*sz);
			
			glVertex2f(pos[0]+sz, pos[1]-sz);
			glVertex2f(pos[0]+0.5*sz, pos[1]-0.5*sz);
			
			glVertex2f(pos[0]+sz, pos[1]+sz);
			glVertex2f(pos[0]+0.5*sz, pos[1]+0.5*sz);
			
			glVertex2f(pos[0]-sz, pos[1]+sz);
			glVertex2f(pos[0]-0.5*sz, pos[1]+0.5*sz);
			glEnd();
		}
	}

	// normal crosshairs
	if (Pi::worldView->GetCamType() == WorldView::CAM_FRONT) {
		float px = Gui::Screen::GetWidth()/2.0;
		float py = Gui::Screen::GetHeight()/2.0;
		glBegin(GL_LINES);
		glVertex2f(px-sz, py);
		glVertex2f(px-0.5*sz, py);
		
		glVertex2f(px+sz, py);
		glVertex2f(px+0.5*sz, py);
		
		glVertex2f(px, py-sz);
		glVertex2f(px, py-0.5*sz);
		
		glVertex2f(px, py+sz);
		glVertex2f(px, py+0.5*sz);
		glEnd();
	} else if (Pi::worldView->GetCamType() == WorldView::CAM_REAR) {
		float px = Gui::Screen::GetWidth()/2.0;
		float py = Gui::Screen::GetHeight()/2.0;
		const float sz = 0.5*HUD_CROSSHAIR_SIZE;
		glBegin(GL_LINES);
		glVertex2f(px-sz, py);
		glVertex2f(px-0.5*sz, py);
		
		glVertex2f(px+sz, py);
		glVertex2f(px+0.5*sz, py);
		
		glVertex2f(px, py-sz);
		glVertex2f(px, py-0.5*sz);
		
		glVertex2f(px, py+sz);
		glVertex2f(px, py+0.5*sz);
		glEnd();
	}
	
	if (Pi::showDebugInfo) {
		char buf[1024];
		vector3d pos = GetPosition();
		vector3d abs_pos = GetPositionRelTo(Space::GetRootFrame());
		const char *rel_to = (GetFrame() ? GetFrame()->GetLabel() : "System");
		snprintf(buf, sizeof(buf), "Pos: %.1f,%.1f,%.1f\n"
			"AbsPos: %.1f,%.1f,%.1f (%.3f AU)\n"
			"Rel-to: %s (%.0f km)",
			pos.x, pos.y, pos.z,
			abs_pos.x, abs_pos.y, abs_pos.z, abs_pos.Length()/AU,
			rel_to, pos.Length()/1000);

		glPushMatrix();
		glTranslatef(2, Gui::Screen::GetFontHeight(), 0);
		Gui::Screen::RenderString(buf);
		glPopMatrix();
	}

	{
		double _vel = sqrt(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]);
		char buf[128];
		if (_vel > 1000) {
			snprintf(buf,sizeof(buf), "Velocity: %.2f km/s", _vel*0.001);
		} else {
			snprintf(buf,sizeof(buf), "Velocity: %.0f m/s", _vel);
		}
		glPushMatrix();
		glTranslatef(2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66, 0);
		Gui::Screen::RenderString(buf);
		glPopMatrix();
	}
	
	if (m_flightControlState == CONTROL_FIXSPEED) {
		char buf[128];
		if (m_setSpeed > 1000) {
			snprintf(buf,sizeof(buf), "Set speed: %.2f km/s", m_setSpeed*0.001);
		} else {
			snprintf(buf,sizeof(buf), "Set speed: %.0f m/s", m_setSpeed);
		}
		glPushMatrix();
		glTranslatef(200, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66, 0);
		Gui::Screen::RenderString(buf);
		glPopMatrix();
	}

	// altitude
	if (GetFrame()->m_astroBody) {
		//(GetFrame()->m_sbody->GetSuperType() == SUPERTYPE_ROCKY_PLANET)) {
		double radius = GetFrame()->m_astroBody->GetRadius();
		double altitude = GetPosition().Length() - radius;
		if (altitude < 0) altitude = 0;
		char buf[128];
		snprintf(buf, sizeof(buf), "Altitude: %.0f m", altitude);
		glPushMatrix();
		glTranslatef(400, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66, 0);
		Gui::Screen::RenderString(buf);
		glPopMatrix();
	}

	Gui::Screen::LeaveOrtho();
}

void Player::DrawTargetSquares()
{
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
	glLineWidth(2.0f);

	if(GetNavTarget()) {
		glColor3f(0.0f, 1.0f, 0.0f);
		DrawTargetSquare(GetNavTarget());
	}

	if(GetCombatTarget()) {
		glColor3f(1.0f, 0.0f, 0.0f);
		DrawTargetSquare(GetNavTarget());
	}

	glPopAttrib();
}

void Player::DrawTargetSquare(const Body* const target)
{
	if(target->IsOnscreen()) {
		glColor3f(0.0f, 1.0f, 0.0f);

		const vector3d& _pos = target->GetProjectedPos();
		const float x1 = _pos.x - WorldView::PICK_OBJECT_RECT_SIZE * 0.5f;
		const float x2 = x1 + WorldView::PICK_OBJECT_RECT_SIZE;
		const float y1 = _pos.y - WorldView::PICK_OBJECT_RECT_SIZE * 0.5f;
		const float y2 = y1 + WorldView::PICK_OBJECT_RECT_SIZE;

		glBegin(GL_LINE_STRIP);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
		glEnd();
	}
}
