#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "Space.h"
#include "Gui.h"
#include "WorldView.h"
#include "SpaceStationView.h"

#define DEG_2_RAD	0.0174532925

Player::Player(): Ship()
{
	m_external_view_rotx = m_external_view_roty = 0;
	m_external_view_dist = 200;
}

void Player::Render(const Frame *camFrame)
{
	if (Pi::GetCamType() == Pi::CAM_EXTERNAL) {
		Ship::Render(camFrame);
	} else {
		glPushMatrix();
		// could only rotate, since transform is zero (camFrame is at player origin)
		TransformToModelCoords(camFrame);
		RenderLaserfire();
		glPopMatrix();
	}
}

void Player::SetDockedWith(SpaceStation *s)
{
	Ship::SetDockedWith(s);
	if (s) {
		Pi::SetView(Pi::spaceStationView);
	}
}

vector3d Player::GetExternalViewTranslation()
{
	vector3d p = vector3d(0, 0, m_external_view_dist);
	p = matrix4x4d::RotateXMatrix(-DEG_2_RAD*m_external_view_rotx) * p;
	p = matrix4x4d::RotateYMatrix(-DEG_2_RAD*m_external_view_roty) * p;
	matrix4x4d m;
	GetRotMatrix(m);
	p = m*p;
//	printf("%f,%f,%f\n", p.x, p.y, p.z);
	return p;
}

void Player::ApplyExternalViewRotation()
{
//	glTranslatef(0, 0, m_external_view_dist);
	glRotatef(-m_external_view_rotx, 1, 0, 0);
	glRotatef(-m_external_view_roty, 0, 1, 0);
}

#define MOUSE_ACCEL	400

void Player::AITurn()
{
	int mouseMotion[2];
	float time_step = Pi::GetTimeStep();
	float ts2 = time_step*time_step;

	if (time_step == 0) return;
	if (GetDockedWith()) return;

	Pi::GetMouseMotion(mouseMotion);
	float mx, my;
	mx = -mouseMotion[0]*time_step*MOUSE_ACCEL;
	my = mouseMotion[1]*time_step*MOUSE_ACCEL;
	
	ClearThrusterState();
	if (Pi::KeyState(SDLK_w)) SetThrusterState(ShipType::THRUSTER_REAR, 1.0f);
	if (Pi::KeyState(SDLK_s)) SetThrusterState(ShipType::THRUSTER_FRONT, 1.0f);
	if (Pi::KeyState(SDLK_2)) SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
	if (Pi::KeyState(SDLK_x)) SetThrusterState(ShipType::THRUSTER_BOTTOM, 1.0f);
	if (Pi::KeyState(SDLK_a)) SetThrusterState(ShipType::THRUSTER_LEFT, 1.0f);
	if (Pi::KeyState(SDLK_d)) SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0f);

	if (Pi::KeyState(SDLK_SPACE)) SetGunState(0,1);
	else SetGunState(0,0);
	
	// no torques at huge time accels -- ODE hates it
	if (time_step <= 10) {
		// dividing by time step so controls don't go totally mental when
		// used at 10x accel
		mx /= ts2;
		my /= ts2;
		if (Pi::MouseButtonState(3) && (mouseMotion[0] || mouseMotion[1])) {
			SetAngThrusterState(1, mx);
			SetAngThrusterState(0, my);
		} else if (Pi::GetCamType() != Pi::CAM_EXTERNAL) {
			float tq = 100/ts2;
			float ax = 0;
			float ay = 0;
			if (Pi::KeyState(SDLK_LEFT)) ay += 1;
			if (Pi::KeyState(SDLK_RIGHT)) ay += -1;
			if (Pi::KeyState(SDLK_UP)) ax += -1;
			if (Pi::KeyState(SDLK_DOWN)) ax += 1;
			SetAngThrusterState(2, 0);
			SetAngThrusterState(1, ay);
			SetAngThrusterState(0, ax);
		}

		// rotation damping.
		vector3d angDrag = GetAngularMomentum() * time_step;
		dBodyAddTorque(m_body, -angDrag.x, -angDrag.y, -angDrag.z);
	}
	if (time_step > 10) {
		dBodySetAngularVel(m_body, 0, 0, 0);
		SetAngThrusterState(0, 0.0f);
		SetAngThrusterState(1, 0.0f);
		SetAngThrusterState(2, 0.0f);
	}
	if (Pi::GetCamType() == Pi::CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) m_external_view_rotx -= 1;
		if (Pi::KeyState(SDLK_DOWN)) m_external_view_rotx += 1;
		if (Pi::KeyState(SDLK_LEFT)) m_external_view_roty -= 1;
		if (Pi::KeyState(SDLK_RIGHT)) m_external_view_roty += 1;
		if (Pi::KeyState(SDLK_EQUALS)) m_external_view_dist -= 10;
		if (Pi::KeyState(SDLK_MINUS)) m_external_view_dist += 10;
		m_external_view_dist = MAX(50, m_external_view_dist);
	}
	Ship::AITurn();
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

	const dReal *vel = dBodyGetLinearVel(m_body);

	const matrix4x4d &rot = Pi::world_view->viewingRotation;
	vector3d loc_v = rot * vector3d(vel[0], vel[1], vel[2]);

	Gui::Screen::EnterOrtho();
	glColor3f(.7,.7,.7);

	{
		for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
			Body *b = *i;
			if (b == this) continue;
			vector3d _pos = b->GetPositionRelTo(cam_frame);
			vector3d cam_coord = rot*_pos;

			//printf("%s: %.1f,%.1f,%.1f\n", b->GetLabel().c_str(), _pos.x, _pos.y, _pos.z);

			if (cam_coord.z < 0) if (Gui::Screen::Project (_pos.x,_pos.y,_pos.z, modelMatrix, projMatrix, viewport, &_pos.x, &_pos.y, &_pos.z)) {
				Gui::Screen::RenderLabel(b->GetLabel(), _pos.x, _pos.y);
			}
		}
	}

	GLdouble pos[3];

	const float sz = HUD_CROSSHAIR_SIZE;
	// if velocity vector is in front of us. draw indicator
	if (loc_v.z < 0) {
		if (Gui::Screen::Project (vel[0],vel[1],vel[2], modelMatrix, projMatrix, viewport, &pos[0], &pos[1], &pos[2])) {
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

	if (Pi::GetCamType() == Pi::CAM_FRONT) {
		// normal crosshairs
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
	
	{
		char buf[1024];
		glPushMatrix();
		glTranslatef(0,440,0);
		vector3d pos = GetPosition();
		vector3d abs_pos = GetPositionRelTo(Space::GetRootFrame());
		const char *rel_to = (GetFrame() ? GetFrame()->GetLabel() : "System");
		snprintf(buf, sizeof(buf), "Pos: %.1f,%.1f,%.1f\n"
			"AbsPos: %.1f,%.1f,%.1f\n"
			"Rel-to: %s",
			pos.x, pos.y, pos.z,
			abs_pos.x, abs_pos.y, abs_pos.z,
			rel_to);
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

	Gui::Screen::LeaveOrtho();
}

