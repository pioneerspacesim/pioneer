#include "libs.h"
#include "Gui.h"
#include "Pi.h"
#include "SectorView.h"
#include "Sector.h"
#include "SystemInfoView.h"
		
SectorView::SectorView(): GenericSystemView()
{
	SetTransparency(true);
	m_px = m_py = 0.5;
	m_rot_x = m_rot_z = 0;
	m_secx = m_secy = 0;
	m_selected = -1;
	m_zoom = 1;

	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, 2);
	
	Gui::ImageButton *ib = new Gui::ImageButton("icons/sectorview_f6_systeminfo.png");
	ib->onClick.connect(sigc::mem_fun(this, &SectorView::OnClickSystemInfo));
	ib->SetShortcut(SDLK_F6, KMOD_NONE);
	m_rightButtonBar->Add(ib, 2, 2);
	
	m_zoomInButton = new Gui::ImageButton("icons/zoom_in_f7.png");
	m_zoomInButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_rightButtonBar->Add(m_zoomInButton, 34, 2);
	
	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out_f8.png");
	m_zoomOutButton->SetShortcut(SDLK_F8, KMOD_NONE);
	m_rightButtonBar->Add(m_zoomOutButton, 66, 2);

	GLUquadricObj *qobj = gluNewQuadric();
	
/*	m_gluSphereDlist = glGenLists(1);
	glNewList(m_gluSphereDlist, GL_COMPILE);
	gluDisk(qobj, 0.0, 0.2, 32, 1);
	glEndList();
*/	
	m_gluDiskDlist = glGenLists(1);
	glNewList(m_gluDiskDlist, GL_COMPILE);
	gluDisk(qobj, 0.0, 0.2, 20, 1);
	glEndList();
	
	gluDeleteQuadric(qobj);
}

SectorView::~SectorView()
{
	glDeleteLists(m_gluDiskDlist, 1);
}

void SectorView::OnClickSystemInfo()
{
	Pi::SetView(Pi::system_info_view);
}

bool SectorView::GetSelectedSystem(int *sector_x, int *sector_y, int *system_idx)
{
	*sector_x = m_secx;
	*sector_y = m_secy;
	*system_idx = m_selected;
	return m_selected != -1;
}

#define SEC_SIZE	8
#define DRAW_RAD	2

#define FFRAC(_x)	((_x)-floor(_x))

void SectorView::Draw3D()
{
	GenericSystemView::Draw3D();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50, Pi::GetScrAspect(), 1.0, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	char buf[80];
	snprintf(buf, sizeof(buf), "Sector: %d,%d", m_secx, m_secy);
	m_infoLabel->SetText(buf);

	// units are lightyears, my friend
	glTranslatef(0, 0, -10-10*m_zoom);
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	glTranslatef(-FFRAC(m_px)*SEC_SIZE, -FFRAC(m_py)*SEC_SIZE, 0);
	glDisable(GL_LIGHTING);

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			glPushMatrix();
			glTranslatef(sx*SEC_SIZE, sy*SEC_SIZE, 0);
			DrawSector(m_secx+sx, m_secy+sy);
			glPopMatrix();
		}
	}

	glEnable(GL_LIGHTING);
}
	
void SectorView::PutText(std::string &text)
{
	// highly optimal..
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	GLint viewport[4];

	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv (GL_VIEWPORT, viewport);

	Gui::Screen::EnterOrtho();
	vector3d _pos;
	if (Gui::Screen::Project (0,0,0, modelMatrix, projMatrix, viewport, &_pos.x, &_pos.y, &_pos.z)) {
		Gui::Screen::RenderLabel(text, _pos.x, _pos.y);
	}
	Gui::Screen::LeaveOrtho();
	glDisable(GL_LIGHTING);
}

void SectorView::DrawSector(int sx, int sy)
{
	Sector s = Sector(sx, sy);
	glColor3f(0,.8,0);
	glBegin(GL_LINE_LOOP);
		glVertex3f(0, 0, 0);
		glVertex3f(0, SEC_SIZE, 0);
		glVertex3f(SEC_SIZE, SEC_SIZE, 0);
		glVertex3f(SEC_SIZE, 0, 0);
	glEnd();
	
	if (!(sx || sy)) glColor3f(1,1,0);
	int num=0;
	for (std::vector<Sector::System>::iterator i = s.m_systems.begin(); i != s.m_systems.end(); ++i) {
		glColor3fv(StarSystem::starColors[(*i).primaryStarClass]);
		glPushMatrix();
		glTranslatef((*i).p.x*SEC_SIZE, (*i).p.y*SEC_SIZE, 0);
		glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, (*i).p.z);
		glEnd();
		glTranslatef(0, 0, (*i).p.z);
		
		glPushMatrix();
		glRotatef(-m_rot_z, 0, 0, 1);
		glRotatef(-m_rot_x, 1, 0, 0);
		glCallList(m_gluDiskDlist);
		// selected indicator
		if ((sx == m_secx) && (sy == m_secy) && (num == m_selected)) {
			glColor3f(0,0.8,0);
			glScalef(2,2,2);
			glCallList(m_gluDiskDlist);
		}
		glPopMatrix();
		glColor3f(.7,.7,.7);
		PutText((*i).name);

		glPopMatrix();
		num++;
	}
}

void SectorView::Update()
{
	if (Pi::KeyState(SDLK_LEFT)) m_px -= 0.01;
	if (Pi::KeyState(SDLK_RIGHT)) m_px += 0.01;
	if (Pi::KeyState(SDLK_UP)) m_py += 0.01;
	if (Pi::KeyState(SDLK_DOWN)) m_py -= 0.01;
	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= 0.99;
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= 1.01;
	if (m_zoomInButton->IsPressed()) m_zoom *= 0.99;
	if (m_zoomOutButton->IsPressed()) m_zoom *= 1.01;
	m_zoom = CLAMP(m_zoom, 0.1, 5.0);
	
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1];
		m_rot_z += motion[0];
	}
	
	m_secx = (int)floor(m_px);
	m_secy = (int)floor(m_py);

	Sector s = Sector(m_secx, m_secy);
	float px = FFRAC(m_px);
	float py = FFRAC(m_py);

	m_selected = -1;
	float min_dist = FLT_MAX;
	for (unsigned int i=0; i<s.m_systems.size(); i++) {
		Sector::System *ss = &s.m_systems[i];
		float dx = px - ss->p.x;
		float dy = py - ss->p.y;
		float dist = sqrtf(dx*dx + dy*dy);
		if (dist < min_dist) {
			min_dist = dist;
			m_selected = i;
		}
	}
}


