#include "libs.h"
#include "Gui.h"
#include "Pi.h"
#include "SectorView.h"
#include "Sector.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "StarSystem.h"
#include "GalacticView.h"
		
SectorView::SectorView()
{
	SetTransparency(true);
	m_lastShownLoc = SysLoc(9999,9999,9999);
	m_px = m_py = m_pxMovingTo = m_pyMovingTo = 0.5;
	m_rot_x = m_rot_z = 0;
	m_secx = m_secy = 0;
	m_selected = -1;
	m_zoom = 1.2;

	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(.7f,.7f,.7f,1.0f));
	Add(m_clickableLabels, 0, 0);

	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in_f7.png");
	//m_zoomInButton->SetShortcut(SDLK_F6, KMOD_NONE);
	m_zoomInButton->SetToolTip("Zoom in");
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out_f8.png");
	//m_zoomOutButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_zoomOutButton->SetToolTip("Zoom out");
	Add(m_zoomOutButton, 732, 5);

	m_gluDiskDlist = glGenLists(1);
	glNewList(m_gluDiskDlist, GL_COMPILE);
	gluDisk(Pi::gluQuadric, 0.0, 0.2, 20, 1);
	glEndList();
	
	Gui::Fixed *infoBar = new Gui::Fixed(Gui::Screen::GetWidth(), 60);
	infoBar->SetTransparency(false);
	infoBar->SetBgColor(0.0f, 0.0f, 1.0f, 0.25f);
	Add(infoBar, 0, 0);

	m_systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	infoBar->Add(m_systemName, 15, 4);
	
	m_distance = (new Gui::Label(""))->Color(1.0f, 0.0f, 0.0f);
	infoBar->Add(m_distance, 300, 4);

	m_starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	infoBar->Add(m_starType, 15, 20);

	m_shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	infoBar->Add(m_shortDesc, 15, 38);

	m_onMouseButtonDown = 
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &SectorView::MouseButtonDown));
}

SectorView::~SectorView()
{
	glDeleteLists(m_gluDiskDlist, 1);
	m_onMouseButtonDown.disconnect();
}

void SectorView::Save(Serializer::Writer &wr)
{
	wr.Float(m_zoom);
	wr.Int32(m_secx);
	wr.Int32(m_secy);
	wr.Int32(m_selected);
	wr.Float(m_px);
	wr.Float(m_py);
	wr.Float(m_rot_x);
	wr.Float(m_rot_z);
}

void SectorView::Load(Serializer::Reader &rd)
{
	m_zoom = rd.Float();
	m_secx = rd.Int32();
	m_secy = rd.Int32();
	m_selected = rd.Int32();
	m_px = m_pxMovingTo = rd.Float();
	m_py = m_pyMovingTo = rd.Float();
	m_rot_x = rd.Float();
	m_rot_z = rd.Float();
}

bool SectorView::GetSelectedSystem(int *sector_x, int *sector_y, int *system_idx)
{
	*sector_x = m_secx;
	*sector_y = m_secy;
	*system_idx = m_selected;
	return m_selected != -1;
}

#define DRAW_RAD	2

#define FFRAC(_x)	((_x)-floor(_x))
static const GLfloat fogDensity = 0.03;
static const GLfloat fogColor[4] = { 0,0,0,1.0 };

void SectorView::Draw3D()
{
	m_clickableLabels->Clear();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, Pi::GetScrAspect(), 1.0, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	char buf[80];
	snprintf(buf, sizeof(buf), "Sector: %d,%d", m_secx, m_secy);
	m_infoLabel->SetText(buf);

	// units are lightyears, my friend
	glTranslatef(0, 0, -10-10*m_zoom);
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	glTranslatef(-FFRAC(m_px)*Sector::SIZE, -FFRAC(m_py)*Sector::SIZE, 0);
	glDisable(GL_LIGHTING);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glHint(GL_FOG_HINT, GL_NICEST);

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			glPushMatrix();
			glTranslatef(sx*Sector::SIZE, sy*Sector::SIZE, 0);
			DrawSector(m_secx+sx, m_secy+sy);
			glPopMatrix();
		}
	}

	glDisable(GL_FOG);
	glEnable(GL_LIGHTING);
}
	
void SectorView::GotoSystem(int sector_x, int sector_y, int system_idx)
{
	Sector s = Sector(sector_x, sector_y);
	const vector3f &p = s.m_systems[system_idx].p;
	m_pxMovingTo = sector_x + p.x/Sector::SIZE;
	m_pyMovingTo = sector_y + p.y/Sector::SIZE;
}

void SectorView::OnClickSystem(int sx, int sy, int sys_idx)
{
	GotoSystem(sx, sy, sys_idx);
}

void SectorView::PutClickableLabel(std::string &text, int sx, int sy, int sys_idx)
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
		m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), sx, sy, sys_idx), _pos.x, _pos.y);
	}
	Gui::Screen::LeaveOrtho();
}

void SectorView::DrawSector(int sx, int sy)
{
	int playerLocSecX, playerLocSecY, playerLocSysIdx;
	Pi::currentSystem->GetPos(&playerLocSecX, &playerLocSecY, &playerLocSysIdx);
	Sector s = Sector(sx, sy);
	glColor3f(0,.8,0);
	glBegin(GL_LINE_LOOP);
		glVertex3f(0, 0, 0);
		glVertex3f(0, Sector::SIZE, 0);
		glVertex3f(Sector::SIZE, Sector::SIZE, 0);
		glVertex3f(Sector::SIZE, 0, 0);
	glEnd();
	
	if (!(sx || sy)) glColor3f(1,1,0);
	int num=0;
	for (std::vector<Sector::System>::iterator i = s.m_systems.begin(); i != s.m_systems.end(); ++i) {
		glColor3fv(StarSystem::starColors[(*i).starType[0]]);
		glPushMatrix();
		glTranslatef((*i).p.x, (*i).p.y, 0);
		glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, (*i).p.z);
		glEnd();
		glTranslatef(0, 0, (*i).p.z);
		
		glPushMatrix();
		glRotatef(-m_rot_z, 0, 0, 1);
		glRotatef(-m_rot_x, 1, 0, 0);
		glScalef(.5,.5,.5);
		glCallList(m_gluDiskDlist);
		glScalef(2,2,2);
		// player location indicator
		if ((sx == playerLocSecX) && (sy == playerLocSecY) && (num == playerLocSysIdx)) {
			const shipstats_t *stats = Pi::player->CalcStats();
			glColor3f(0,0,1);
			glBegin(GL_LINE_LOOP);
			// draw a lovely circle around our beloved player
			for (float theta=0; theta < 2*M_PI; theta += 0.05*M_PI) {
				glVertex3f(stats->hyperspace_range*sin(theta), stats->hyperspace_range*cos(theta), 0);
			}
			glEnd();

			glPushMatrix();
			glDepthRange(0.2,1.0);
			glColor3f(0,0,0.8);
			glScalef(3,3,3);
			glCallList(m_gluDiskDlist);
			glPopMatrix();
		}
		// selected indicator
		if ((sx == m_secx) && (sy == m_secy) && (num == m_selected)) {
			glDepthRange(0.1,1.0);
			glColor3f(0,0.8,0);
			glScalef(2,2,2);
			glCallList(m_gluDiskDlist);
		}
		glDepthRange(0,1);
		glPopMatrix();
		glColor3f(.7,.7,.7);
		PutClickableLabel((*i).name, sx, sy, num);
		glDisable(GL_LIGHTING);

		glPopMatrix();
		num++;
	}
}

void SectorView::OnSwitchTo() {
	m_lastShownLoc = SysLoc(9999,9999,9999);
	Update();
}

void SectorView::Update()
{
	const float frameTime = Pi::GetFrameTime();

	int playerLocSecX, playerLocSecY, playerLocSysIdx;
	Pi::currentSystem->GetPos(&playerLocSecX, &playerLocSecY, &playerLocSysIdx);

	if (Pi::KeyState(SDLK_c)) {
		GotoSystem(playerLocSecX, playerLocSecY, playerLocSysIdx);
		if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
			m_rot_x = m_rot_z = 0;
			m_zoom = 1.2;
		}
	}

	float moveSpeed = 1.0;
	if (Pi::KeyState(SDLK_LSHIFT)) moveSpeed = 100.0;
	
	if (Pi::KeyState(SDLK_LEFT)) m_pxMovingTo -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_RIGHT)) m_pxMovingTo += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_UP)) m_pyMovingTo += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_DOWN)) m_pyMovingTo -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(0.5f, frameTime);
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(2.0f, frameTime);
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(0.5f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(2.0f, frameTime);
	m_zoom = Clamp(m_zoom, 0.1f, 5.0f);
	
	// when zooming to a clicked on spot
	{
		float diffx = m_pxMovingTo - m_px;
		float diffy = m_pyMovingTo - m_py;
		m_px += diffx * 10.0*frameTime;
		m_py += diffy * 10.0*frameTime;
	}
	
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1];
		m_rot_z += motion[0];
	}
	
	m_secx = (int)floor(m_px);
	m_secy = (int)floor(m_py);

	Sector s = Sector(m_secx, m_secy);
	float px = FFRAC(m_px)*Sector::SIZE;
	float py = FFRAC(m_py)*Sector::SIZE;

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
	
	StarSystem *sys = Pi::GetSelectedSystem();
	if (!sys) return;

	if (sys->GetLocation() != m_lastShownLoc) {
		Sector sec(m_secx, m_secy);
		Sector psec(playerLocSecX, playerLocSecY);
		const float dist = Sector::DistanceBetween(&sec, m_selected, &psec, playerLocSysIdx);
		char buf[256];
		SBodyPath sbody_path(m_secx, m_secy, m_selected);
		int fuelRequired;
		double dur;
		enum Ship::HyperjumpStatus jumpStatus;
		Pi::player->CanHyperspaceTo(&sbody_path, fuelRequired, dur, &jumpStatus);
		switch (jumpStatus) {
			case Ship::HYPERJUMP_OK:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (fuel required: %dt)", dist, fuelRequired);
				Pi::player->SetHyperspaceTarget(&sbody_path);
				break;
			case Ship::HYPERJUMP_CURRENT_SYSTEM:
				snprintf(buf, sizeof(buf), "Current system");
				Pi::player->ClearHyperspaceTarget();
				break;
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (insufficient fuel, required: %dt)", dist, fuelRequired);
				Pi::player->ClearHyperspaceTarget();
				break;
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (out of range)", dist);
				Pi::player->ClearHyperspaceTarget();
				break;
			case Ship::HYPERJUMP_NO_DRIVE:
				snprintf(buf, sizeof(buf), "You cannot perform a hyperjump because you do not have a functioning hyperdrive");
				Pi::player->ClearHyperspaceTarget();
				break;
		}

		std::string desc;
		if (sys->GetNumStars() == 4) {
			desc = "Quadruple system. ";
		} else if (sys->GetNumStars() == 3) {
			desc = "Triple system. ";
		} else if (sys->GetNumStars() == 2) {
			desc = "Binary system. ";
		} else {
			desc = sys->rootBody->GetAstroDescription();
		}

		m_systemName->SetText(sec.m_systems[m_selected].name);
		m_distance->SetText(buf);
		m_starType->SetText(desc);
		m_shortDesc->SetText(sys->GetShortDescription());

		m_lastShownLoc = sys->GetLocation();
	}
}

void SectorView::MouseButtonDown(int button, int x, int y)
{
	const float ft = Pi::GetFrameTime();
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN)) 
			m_zoom *= pow(2.0f, ft);
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP)) 
			m_zoom *= pow(0.5f, ft);
}

