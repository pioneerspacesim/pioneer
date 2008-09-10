#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "Space.h"
#include "Gui.h"
#include "WorldView.h"
#include "SpaceStationView.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_external_view_rotx = m_external_view_roty = 0;
	m_external_view_dist = 200;
	m_mouseCMov[0] = m_mouseCMov[1] = 0;
	m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS1);
	UpdateMass();
}

Player::~Player()
{
	assert(this == Pi::player);
	Pi::player = 0;
}

void Player::Render(const Frame *camFrame)
{
	if (Pi::GetCamType() == Pi::CAM_EXTERNAL) {
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

vector3d Player::GetExternalViewTranslation()
{
	vector3d p = vector3d(0, 0, m_external_view_dist);
	p = matrix4x4d::RotateXMatrix(-DEG2RAD(m_external_view_rotx)) * p;
	p = matrix4x4d::RotateYMatrix(-DEG2RAD(m_external_view_roty)) * p;
	matrix4x4d m;
	GetRotMatrix(m);
	p = m*p;
//	printf("%f,%f,%f\n", p.x, p.y, p.z);
	return p;
}

void Player::ApplyExternalViewRotation(matrix4x4d &m)
{
	m = matrix4x4d::RotateXMatrix(-DEG2RAD(m_external_view_rotx)) * m;
	m = matrix4x4d::RotateYMatrix(-DEG2RAD(m_external_view_roty)) * m;
}

void Player::TimeStepUpdate(const float timeStep)
{
	if (GetFlightState() == Ship::FLYING) {
		// when world view not selected
		if (!polledControlsThisTurn) {
			const float time_accel = Pi::GetTimeAccel();
			const float ta2 = time_accel*time_accel;
			ClearThrusterState();
			// still must apply rotation damping
			vector3d damping = CalcRotDamping();
			damping *= 1.0f/ta2;
			SetAngThrusterState(0, -damping.x);
			SetAngThrusterState(1, -damping.y);
			SetAngThrusterState(2, -damping.z);
		}
	}
	polledControlsThisTurn = false;
	Ship::TimeStepUpdate(timeStep);
}

#define MOUSE_CTRL_AREA		10.0f
#define MOUSE_RESTITUTION	0.01f

void Player::PollControls()
{
	int mouseMotion[2];
	const float frameTime = Pi::GetFrameTime();
	float time_accel = Pi::GetTimeAccel();
	float ta2 = time_accel*time_accel;

	polledControlsThisTurn = true;

	if (Pi::GetCamType() == Pi::CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) m_external_view_rotx -= 45*frameTime;
		if (Pi::KeyState(SDLK_DOWN)) m_external_view_rotx += 45*frameTime;
		if (Pi::KeyState(SDLK_LEFT)) m_external_view_roty -= 45*frameTime;
		if (Pi::KeyState(SDLK_RIGHT)) m_external_view_roty += 45*frameTime;
		if (Pi::KeyState(SDLK_EQUALS)) m_external_view_dist -= 400*frameTime;
		if (Pi::KeyState(SDLK_MINUS)) m_external_view_dist += 400*frameTime;
		m_external_view_dist = MAX(50, m_external_view_dist);

		// when landed don't let external view look from below
		if (GetFlightState() == LANDED) m_external_view_rotx = CLAMP(m_external_view_rotx, -170.0, -10);
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
			float restitution = powf(MOUSE_RESTITUTION, Pi::GetTimeStep());
			Pi::GetMouseMotion(mouseMotion);
			m_mouseCMov[0] += mouseMotion[0];
			m_mouseCMov[1] += mouseMotion[1];
			m_mouseCMov[0] = CLAMP(m_mouseCMov[0]*restitution, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			m_mouseCMov[1] = CLAMP(m_mouseCMov[1]*restitution, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			angThrust.y = -m_mouseCMov[0] / MOUSE_CTRL_AREA;
			angThrust.x = m_mouseCMov[1] / MOUSE_CTRL_AREA;
		}
		
		if (Pi::KeyState(SDLK_w)) SetThrusterState(ShipType::THRUSTER_REAR, 1.0f);
		if (Pi::KeyState(SDLK_s)) SetThrusterState(ShipType::THRUSTER_FRONT, 1.0f);
		if (Pi::KeyState(SDLK_2)) SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
		if (Pi::KeyState(SDLK_x)) SetThrusterState(ShipType::THRUSTER_BOTTOM, 1.0f);
		if (Pi::KeyState(SDLK_a)) SetThrusterState(ShipType::THRUSTER_LEFT, 1.0f);
		if (Pi::KeyState(SDLK_d)) SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0f);

		if (Pi::KeyState(SDLK_SPACE) || (Pi::MouseButtonState(1) && Pi::MouseButtonState(3))) SetGunState(0,1);
		else SetGunState(0,0);
		
		// no torques at huge time accels -- ODE hates it
		if (time_accel <= 10) {
			if (Pi::GetCamType() != Pi::CAM_EXTERNAL) {
				if (Pi::KeyState(SDLK_LEFT)) angThrust.y += 1;
				if (Pi::KeyState(SDLK_RIGHT)) angThrust.y += -1;
				if (Pi::KeyState(SDLK_UP)) angThrust.x += -1;
				if (Pi::KeyState(SDLK_DOWN)) angThrust.x += 1;
			}
			// rotation damping.
			vector3d damping = CalcRotDamping();

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
		if (time_accel > 10) {
			dBodySetAngularVel(m_body, 0, 0, 0);
		}
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
			if ((Pi::GetCamType() != Pi::CAM_EXTERNAL) && (*i == this)) continue;
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
	const dReal *vel = dBodyGetLinearVel(m_body);
	vector3d loc_v = cam_frame->GetOrientation().InverseOf() * vector3d(vel[0], vel[1], vel[2]);
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
	if (Pi::GetCamType() == Pi::CAM_FRONT) {
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
	}
	
	if (Pi::showDebugInfo) {
		char buf[1024];
		glPushMatrix();
		glTranslatef(0,440,0);
		vector3d pos = GetPosition();
		vector3d abs_pos = GetPositionRelTo(Space::GetRootFrame());
		const char *rel_to = (GetFrame() ? GetFrame()->GetLabel() : "System");
		snprintf(buf, sizeof(buf), "Pos: %.1f,%.1f,%.1f\n"
			"AbsPos: %.1f,%.1f,%.1f (%.3f AU)\n"
			"Rel-to: %s (%.0f km)",
			pos.x, pos.y, pos.z,
			abs_pos.x, abs_pos.y, abs_pos.z, abs_pos.Length()/AU,
			rel_to, pos.Length()/1000);
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
		glTranslatef(2, 66, 0);
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
		glTranslatef(400, 66, 0);
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
