#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "Space.h"
#include "Gui.h"
#include "WorldView.h"
#include "SpaceStationView.h"

#define DEG_2_RAD	0.0174532925

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_external_view_rotx = m_external_view_roty = 0;
	m_external_view_dist = 200;
	m_mouseCMov[0] = m_mouseCMov[1] = 0;
	m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS1);
	UpdateMass();
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

#define MOUSE_CTRL_AREA		10.0f
#define MOUSE_RESTITUTION	0.01f

void Player::PollControls()
{
	int mouseMotion[2];
	float time_accel = Pi::GetTimeAccel();
	float ta2 = time_accel*time_accel;

	SetAngThrusterState(0, 0.0f);
	SetAngThrusterState(1, 0.0f);
	SetAngThrusterState(2, 0.0f);

	if ((time_accel == 0) || GetDockedWith()) {
		return;
	}

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
	
	ClearThrusterState();
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
		const dReal *_av = dBodyGetAngularVel(m_body);
		vector3d angVel(_av[0], _av[1], _av[2]);
		matrix4x4d rot;
		GetRotMatrix(rot);
		angVel = rot.InverseOf() * angVel;

		angVel *= 0.6;
		angThrust.x -= angVel.x;
		angThrust.y -= angVel.y;
		angThrust.z -= angVel.z;

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
	if (Pi::GetCamType() == Pi::CAM_EXTERNAL) {
		if (Pi::KeyState(SDLK_UP)) m_external_view_rotx -= 1;
		if (Pi::KeyState(SDLK_DOWN)) m_external_view_rotx += 1;
		if (Pi::KeyState(SDLK_LEFT)) m_external_view_roty -= 1;
		if (Pi::KeyState(SDLK_RIGHT)) m_external_view_roty += 1;
		if (Pi::KeyState(SDLK_EQUALS)) m_external_view_dist -= 10;
		if (Pi::KeyState(SDLK_MINUS)) m_external_view_dist += 10;
		m_external_view_dist = MAX(50, m_external_view_dist);
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

	const dReal *vel = dBodyGetLinearVel(m_body);

	const matrix4x4d &rot = Pi::world_view->viewingRotation;
	vector3d loc_v = rot * vector3d(vel[0], vel[1], vel[2]);

	Gui::Screen::EnterOrtho();
	glColor3f(.7,.7,.7);

	{
		for(std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
			if ((Pi::GetCamType() != Pi::CAM_EXTERNAL) && (*i == this)) continue;
			Body *b = *i;
			vector3d _pos = b->GetPositionRelTo(cam_frame);
			vector3d cam_coord = rot*_pos;

			//printf("%s: %.1f,%.1f,%.1f\n", b->GetLabel().c_str(), _pos.x, _pos.y, _pos.z);

			if (cam_coord.z < 0
				&& Gui::Screen::Project (_pos.x,_pos.y,_pos.z, modelMatrix, projMatrix, viewport, &_pos.x, &_pos.y, &_pos.z)) {
				b->SetProjectedPos(_pos);
				b->SetOnscreen(true);
				Gui::Screen::RenderLabel(b->GetLabel(), _pos.x, _pos.y);
			}
			else
				b->SetOnscreen(false);
		}
	}

	DrawTargetSquare();

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

void Player::DrawTargetSquare()
{
	if(GetTarget() && GetTarget()->IsOnscreen()) {
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
		glColor3f(0.0f, 1.0f, 0.0f);
		glLineWidth(2.0f);

		const vector3d& _pos = GetTarget()->GetProjectedPos();
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

		glPopAttrib();
	}
}
