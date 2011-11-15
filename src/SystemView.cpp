#include "SystemView.h"
#include "Pi.h"
#include "SectorView.h"
#include "StarSystem.h"
#include "Lang.h"
#include "StringF.h"
#include "Space.h"
#include "Player.h"
#include "FloatComparison.h"

const double SystemView::PICK_OBJECT_RECT_SIZE = 12.0;

SystemView::SystemView()
{
	SetTransparency(true);

	Gui::Screen::PushFont("OverlayFont");
	m_objectLabels = new Gui::LabelSet();
	Add(m_objectLabels, 0, 0);
	Gui::Screen::PopFont();

	m_timePoint = (new Gui::Label(""))->Color(0.7f, 0.7f, 0.7f);
	Add(m_timePoint, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
	m_infoLabel = (new Gui::Label(""))->Color(0.7f, 0.7f, 0.7f);
	Add(m_infoLabel, 2, 0);
	
	m_infoText = (new Gui::Label(""))->Color(0.7f, 0.7f, 0.7f);
	Add(m_infoText, 200, 0);
	
	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	Add(m_zoomOutButton, 732, 5);

	Gui::ImageButton *b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_r3.png", PIONEER_DATA_DIR "/icons/sysview_accel_r3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -10000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 0, 0);
	
	b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_r2.png", PIONEER_DATA_DIR "/icons/sysview_accel_r2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -1000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 26, 0);
	
	b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_r1.png", PIONEER_DATA_DIR "/icons/sysview_accel_r1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -100000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 45, 0);
	
	b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_f1.png", PIONEER_DATA_DIR "/icons/sysview_accel_f1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 100000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 64, 0);
	
	b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_f2.png", PIONEER_DATA_DIR "/icons/sysview_accel_f2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 1000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 83, 0);

	b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/sysview_accel_f3.png", PIONEER_DATA_DIR "/icons/sysview_accel_f3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 10000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 102, 0);

	m_onMouseButtonDown = 
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &SystemView::MouseButtonDown));
	
	ResetViewpoint();
}

SystemView::~SystemView()
{
	m_onMouseButtonDown.disconnect();
}

void SystemView::OnClickAccel(float step)
{
	m_timeStep = step;
}

void SystemView::ResetViewpoint()
{
	m_selectedObject = 0;
	m_rot_z = 0;
	m_rot_x = 50;
	m_zoom = 1.0f/AU;
	m_timeStep = 1.0f;
	m_time = Pi::GetGameTime();
}

void SystemView::PutOrbit(SBody *b, vector3d offset)
{
	glColor3f(0,1,0);
	glBegin(GL_LINE_LOOP);
	for (double t=0.0; t<1.0; t += 0.01) {
		vector3d pos = b->orbit.EvenSpacedPosAtTime(t);
		pos = offset + pos * double(m_zoom);
		glVertex3dv(&pos[0]);
	}
	glEnd();
}

void SystemView::OnClickObject(SBody *b)
{
	m_selectedObject = b;
	std::string desc;
	std::string data;

	desc += std::string(Lang::NAME);
	desc += ":\n";
	data += b->name+"\n";
	
	desc += std::string(Lang::DAY_LENGTH);
	desc += std::string(Lang::ROTATIONAL_PERIOD);
	desc += ":\n";
	data += stringf(Lang::N_DAYS, formatarg("days", b->rotationPeriod.ToFloat())) + "\n";
	
	desc += std::string(Lang::RADIUS);
	desc += ":\n";
	data += format_distance(b->GetRadius())+"\n";

	if (b->parent) {
		desc += std::string(Lang::SEMI_MAJOR_AXIS);
	desc += ":\n";
		data += format_distance(b->orbit.semiMajorAxis)+"\n";

		desc += std::string(Lang::ORBITAL_PERIOD);
	desc += ":\n";
		data += stringf(Lang::N_DAYS, formatarg("days", b->orbit.period / (24*60*60))) + "\n";
	}
	m_infoLabel->SetText(desc);
	m_infoText->SetText(data);

	if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
		SystemPath path = m_system->GetPathOf(b);
		if (Pi::currentSystem->GetPath() == m_system->GetPath()) {
			Body* body = Space::FindBodyForPath(&path);
			if (body != 0)
				Pi::player->SetNavTarget(body);
		}
	}
}

void SystemView::PutLabel(SBody *b, vector3d offset)
{
	Gui::Screen::EnterOrtho();

	vector3d pos;
	if (Gui::Screen::Project(offset, pos)) {
		// libsigc++ is a beautiful thing
		m_objectLabels->Add(b->name, sigc::bind(sigc::mem_fun(this, &SystemView::OnClickObject), b), pos.x, pos.y);
	}

	Gui::Screen::LeaveOrtho();
	glDisable(GL_LIGHTING);
}

// i don't know how to name it
#define ROUGH_SIZE_OF_TURD	10.0

matrix4x4f s_invRot;

void SystemView::PutBody(SBody *b, vector3d offset)
{
	if (b->type == SBody::TYPE_STARPORT_SURFACE) return;
	if (b->type != SBody::TYPE_GRAVPOINT) {
		glGetFloatv (GL_MODELVIEW_MATRIX, &s_invRot[0]);
		s_invRot[12] = s_invRot[13] = s_invRot[14] = 0;
		s_invRot = s_invRot.InverseOf();

		glColor3f(1,1,1);
		glBegin(GL_TRIANGLE_FAN);
		double radius = b->GetRadius() * m_zoom;
		const vector3f offsetf(offset);
		for (float ang=0; ang<2.0f*M_PI; ang+=M_PI*0.05f) {
			vector3f p = offsetf + s_invRot * vector3f(radius*sin(ang), -radius*cos(ang), 0);
			glVertex3fv(&p.x);
		}
		glEnd();

		PutLabel(b, offset);
	}

	if (b->children.size()) for(std::vector<SBody*>::iterator kid = b->children.begin(); kid != b->children.end(); ++kid) {

		if (float_is_zero_general((*kid)->orbit.semiMajorAxis)) continue;
		if ((*kid)->orbit.semiMajorAxis * m_zoom < ROUGH_SIZE_OF_TURD) {
			PutOrbit(*kid, offset);
		}

		// not using current time yet
		vector3d pos = (*kid)->orbit.OrbitalPosAtTime(m_time);
		pos *= double(m_zoom);
		//glTranslatef(pos.x, pos.y, pos.z);

		PutBody(*kid, offset + pos);
	}
}

void SystemView::PutSelectionBox(const SBody *b, const vector3d &rootPos, const Color &col)
{
	// surface starports just show the planet as being selected,
	// because SystemView doesn't render terrains anyway
	if (b->type == SBody::TYPE_STARPORT_SURFACE)
		b = b->parent;
	assert(b);

	vector3d pos = rootPos;
	// while (b->parent), not while (b) because the root SBody is defined to be at (0,0,0)
	while (b->parent) {
		pos += b->orbit.OrbitalPosAtTime(m_time) * double(m_zoom);
		b = b->parent;
	}

	PutSelectionBox(pos, col);
}

void SystemView::PutSelectionBox(const vector3d &worldPos, const Color &col)
{
	Gui::Screen::EnterOrtho();

	vector3d screenPos;
	if (Gui::Screen::Project(worldPos, screenPos)) {
		// XXX copied from WorldView::DrawTargetSquare -- these should be unified
		const float x1 = float(screenPos.x - SystemView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float x2 = float(x1 + SystemView::PICK_OBJECT_RECT_SIZE);
		const float y1 = float(screenPos.y - SystemView::PICK_OBJECT_RECT_SIZE * 0.5);
		const float y2 = float(y1 + SystemView::PICK_OBJECT_RECT_SIZE);

		const GLfloat vtx[8] = {
			x1, y1,
			x2, y1,
			x2, y2,
			x1, y2
		};
		glColor4f(col.r, col.g, col.b, col.a);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	Gui::Screen::LeaveOrtho();
}

static const GLfloat fogDensity = 0.1;
static const GLfloat fogColor[4] = { 0,0,0,1.0 };

void SystemView::GetTransformTo(SBody *b, vector3d &pos)
{
	if (b->parent) {
		GetTransformTo(b->parent, pos);
		pos -= double(m_zoom) * b->orbit.OrbitalPosAtTime(m_time);
	}
}

void SystemView::Draw3D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50, Pi::GetScrAspect(), 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	SystemPath path = Pi::sectorView->GetSelectedSystem();
	if (m_system) {
		if (!m_system->GetPath().IsSameSystem(path)) {
			m_system.Reset();
			ResetViewpoint();
		}
	}
	m_time += m_timeStep*Pi::GetFrameTime();
	std::string t = Lang::TIME_POINT+format_date(m_time);
	m_timePoint->SetText(t);

	if (!m_system) m_system = StarSystem::GetCached(path);

	glDisable(GL_LIGHTING);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glHint(GL_FOG_HINT, GL_NICEST);

	glTranslatef(0,0,-ROUGH_SIZE_OF_TURD);
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	
	vector3d pos(0,0,0);
	if (m_selectedObject) GetTransformTo(m_selectedObject, pos);

	m_objectLabels->Clear();
	if (m_system->m_unexplored)
		m_infoLabel->SetText(Lang::UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW);
	else if (m_system->rootBody) {
		PutBody(m_system->rootBody, pos);
		if (Pi::currentSystem == m_system) {
			const Body *navTarget = Pi::player->GetNavTarget();
			const SBody *navTargetSBody = navTarget ? navTarget->GetSBody() : 0;
			if (navTargetSBody)
				PutSelectionBox(navTargetSBody, pos, Color(0.0, 1.0, 0.0, 1.0));
		}
	}

	glEnable(GL_LIGHTING);
	glDisable(GL_FOG);
}

void SystemView::Update()
{
	const float ft = Pi::GetFrameTime();
	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
		if (Pi::KeyState(SDLK_EQUALS) ||
			m_zoomInButton->IsPressed()) 
				m_zoom *= pow(4.0f, ft);
		if (Pi::KeyState(SDLK_MINUS) ||
			m_zoomOutButton->IsPressed()) 
				m_zoom *= pow(0.25f, ft);
	}
	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1]*20*ft;
		m_rot_z += motion[0]*20*ft;
	}
}

void SystemView::MouseButtonDown(int button, int x, int y)
{
	const float ft = Pi::GetFrameTime();
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN)) 
			m_zoom *= pow(0.25f, ft);
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP)) 
			m_zoom *= pow(4.0f, ft);
}
