#include "SystemView.h"
#include "Pi.h"
#include "SectorView.h"
#include "StarSystem.h"

SystemView::SystemView(): View()
{
	m_system = 0;
	SetTransparency(true);

	m_timePoint = new Gui::Label("");
	m_timePoint->SetColor(.7,.7,.7);
	Add(m_timePoint, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
	m_zoomInButton = new Gui::ImageButton("icons/zoom_in_f7.png");
	m_zoomInButton->SetShortcut(SDLK_F6, KMOD_NONE);
	m_zoomInButton->SetToolTip("Zoom in");
	m_rightButtonBar->Add(m_zoomInButton, 34, 2);
	
	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out_f8.png");
	m_zoomOutButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_zoomOutButton->SetToolTip("Zoom out");
	m_rightButtonBar->Add(m_zoomOutButton, 66, 2);

	Gui::ImageButton *b = new Gui::ImageButton("icons/sysview_accel_r3.png", "icons/sysview_accel_r3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -10000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 0, 0);
	
	b = new Gui::ImageButton("icons/sysview_accel_r2.png", "icons/sysview_accel_r2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -1000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 26, 0);
	
	b = new Gui::ImageButton("icons/sysview_accel_r1.png", "icons/sysview_accel_r1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), -100000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 45, 0);
	
	b = new Gui::ImageButton("icons/sysview_accel_f1.png", "icons/sysview_accel_f1_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 100000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 64, 0);
	
	b = new Gui::ImageButton("icons/sysview_accel_f2.png", "icons/sysview_accel_f2_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 1000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 83, 0);

	b = new Gui::ImageButton("icons/sysview_accel_f3.png", "icons/sysview_accel_f3_on.png");
	b->onPress.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 10000000.0));
	b->onRelease.connect(sigc::bind(sigc::mem_fun(this, &SystemView::OnClickAccel), 0.0));
	m_rightRegion2->Add(b, 102, 0);
	
	ResetViewpoint();
}

SystemView::~SystemView()
{
}

void SystemView::OnClickAccel(float step)
{
	m_timeStep = step;
}

void SystemView::ResetViewpoint()
{
	m_selectedObject = 0;
	m_rot_x = m_rot_z = 0;
	m_zoom = 1.0f/AU;
	m_timeStep = 1.0f;
	m_time = Pi::GetGameTime();
}

void SystemView::PutOrbit(StarSystem::SBody *b)
{
	glColor3f(0,1,0);
	glBegin(GL_LINE_LOOP);
	double inc = b->orbit.period/100.0;
	for (double t=0.0; t < b->orbit.period; t += inc) {
		vector3d pos = b->orbit.CartesianPosAtTime(t);
		pos = pos * m_zoom;
		glVertex3dv(&pos[0]);
	}
	glEnd();
}

void SystemView::OnClickObject(StarSystem::SBody *b, const Gui::MouseButtonEvent *ev)
{
	m_selectedObject = b;
}

void SystemView::PutLabel(StarSystem::SBody *b)
{
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	GLint viewport[4];

	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv (GL_VIEWPORT, viewport);

	Gui::Screen::EnterOrtho();
	
	vector3d pos;
	if (Gui::Screen::Project (0,0,0, modelMatrix, projMatrix, viewport, &pos[0], &pos[1], &pos[2])) {
		// libsigc++ is a beautiful thing
		Gui::Screen::PutClickableLabel(b->name, pos.x, pos.y,
				sigc::bind<0>(sigc::mem_fun(this, &SystemView::OnClickObject), b));
	}

	Gui::Screen::LeaveOrtho();
	glDisable(GL_LIGHTING);
}

// i don't know how to name it
#define ROUGH_SIZE_OF_TURD	10.0

void SystemView::PutBody(StarSystem::SBody *b)
{
	if (b->type == StarSystem::TYPE_STARPORT_SURFACE) return;
	if (b->type != StarSystem::TYPE_GRAVPOINT) {
		glPointSize(5);
		glColor3f(1,1,1);
		glBegin(GL_POINTS);
		glVertex3f(0,0,0);
		glEnd();

		PutLabel(b);
	}

	if (b->children.size()) for(std::vector<StarSystem::SBody*>::iterator kid = b->children.begin(); kid != b->children.end(); ++kid) {

		if ((*kid)->orbit.semiMajorAxis * m_zoom < ROUGH_SIZE_OF_TURD)
			PutOrbit(*kid);
		
		glPushMatrix();
		{
			// not using current time yet
			vector3d pos = (*kid)->orbit.CartesianPosAtTime(m_time);
			pos = pos * m_zoom;
			glTranslatef(pos.x, pos.y, pos.z);
			
			PutBody(*kid);
		}
		glPopMatrix();
	}
}

static const GLfloat fogDensity = 0.1;
static const GLfloat fogColor[4] = { 0,0,0,1.0 };

void SystemView::ViewingTransformTo(StarSystem::SBody *b)
{
	if (b->parent) {
		ViewingTransformTo(b->parent);
		vector3d pos = b->orbit.CartesianPosAtTime(m_time);
		pos = pos * m_zoom;
		glTranslatef(-pos.x, -pos.y, -pos.z);
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
	
	int sector_x, sector_y, system_idx;
	Pi::sectorView->GetSelectedSystem(&sector_x, &sector_y, &system_idx);
	if (m_system) {
		if (!m_system->IsSystem(sector_x, sector_y, system_idx)) {
			delete m_system;
			m_system = 0;
			ResetViewpoint();
		}
	}
	m_time += m_timeStep*Pi::GetFrameTime();
	std::string t = "Time point: "+format_date(m_time);
	m_timePoint->SetText(t);

	if (!m_system) m_system = new StarSystem(sector_x, sector_y, system_idx);

	glDisable(GL_LIGHTING);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glHint(GL_FOG_HINT, GL_NICEST);

	glTranslatef(0,0,-ROUGH_SIZE_OF_TURD);
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	
	if (m_selectedObject) ViewingTransformTo(m_selectedObject);

	if (m_system->rootBody) PutBody(m_system->rootBody);
	
	glEnable(GL_LIGHTING);
	glDisable(GL_FOG);
}

void SystemView::Update()
{
	const float ft = Pi::GetFrameTime();
	if (Pi::KeyState(SDLK_EQUALS) ||
	    m_zoomInButton->IsPressed()) m_zoom *= pow(4.0f, ft);
	if (Pi::KeyState(SDLK_MINUS) ||
	    m_zoomOutButton->IsPressed()) m_zoom *= pow(0.25f, ft);
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1]*20*ft;
		m_rot_z += motion[0]*20*ft;
	}
}
