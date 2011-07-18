#include "libs.h"
#include "gui/Gui.h"
#include "Pi.h"
#include "SectorView.h"
#include "Sector.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "StarSystem.h"
#include "GalacticView.h"
		
SectorView::SectorView() :
	m_firstTime(true)
{
	SetTransparency(true);
	m_pos = m_posMovingTo = vector3f(0.5f);
	m_rot_x = m_rot_z = 0;
	m_zoom = 1.2;

	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(.7f,.7f,.7f,1.0f));
	Add(m_clickableLabels, 0, 0);

	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in.png");
	m_zoomInButton->SetToolTip("Zoom in");
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out.png");
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
	m_selected.Serialize(wr);
	wr.Float(m_pos.x);
	wr.Float(m_pos.y);
	wr.Float(m_pos.z);
	wr.Float(m_rot_x);
	wr.Float(m_rot_z);
}

void SectorView::Load(Serializer::Reader &rd)
{
	m_zoom = rd.Float();
	m_selected = SystemPath::Unserialize(rd);
	m_pos.x = m_posMovingTo.x = rd.Float();
	m_pos.y = m_posMovingTo.y = rd.Float();
	m_pos.z = m_posMovingTo.z = rd.Float();
	m_rot_x = rd.Float();
	m_rot_z = rd.Float();
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
	snprintf(buf, sizeof(buf), "Sector: %d,%d", m_selected.sectorX, m_selected.sectorY);
	m_infoLabel->SetText(buf);

	// units are lightyears, my friend
	glTranslatef(0, 0, -10-10*m_zoom);
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	glTranslatef(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);
	glDisable(GL_LIGHTING);
	/*glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glHint(GL_FOG_HINT, GL_NICEST);
*/
	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				glPushMatrix();
				glTranslatef(sx*Sector::SIZE, sy*Sector::SIZE, sz*Sector::SIZE);
				DrawSector(m_selected.sectorX+sx, m_selected.sectorY+sy, m_selected.sectorZ+sz);
				glPopMatrix();
			}
		}
	}

	glDisable(GL_FOG);
	glEnable(GL_LIGHTING);
}

void SectorView::GotoSystem(const SystemPath &path)
{
	Sector* ps = GetCached(path.sectorX, path.sectorY, path.sectorZ);
	const vector3f &p = ps->m_systems[path.systemIndex].p;
	m_posMovingTo.x = path.sectorX + p.x/Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y/Sector::SIZE;
}

void SectorView::WarpToSystem(const SystemPath &path)
{
	GotoSystem(path);
	m_pos = m_posMovingTo;
}

void SectorView::OnClickSystem(const SystemPath &path)
{
	GotoSystem(path);
}

void SectorView::PutClickableLabel(std::string &text, const SystemPath &path)
{
	Gui::Screen::EnterOrtho();
	vector3d pos;
	if (Gui::Screen::Project(vector3d(0.0), pos)) {
		m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), path), pos.x, pos.y);
	}
	Gui::Screen::LeaveOrtho();
}

void SectorView::DrawSector(int sx, int sy, int sz)
{
	SystemPath playerLoc = Pi::currentSystem->GetPath();
	Sector* ps = GetCached(sx, sy, sz);
	glColor3f(0,.8,0);
	glBegin(GL_LINE_LOOP);
		glVertex3f(0, 0, 0);
		glVertex3f(0, Sector::SIZE, 0);
		glVertex3f(Sector::SIZE, Sector::SIZE, 0);
		glVertex3f(Sector::SIZE, 0, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex3f(0, 0, Sector::SIZE);
		glVertex3f(0, Sector::SIZE, Sector::SIZE);
		glVertex3f(Sector::SIZE, Sector::SIZE, Sector::SIZE);
		glVertex3f(Sector::SIZE, 0, Sector::SIZE);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, Sector::SIZE);

		glVertex3f(0, Sector::SIZE, 0);
		glVertex3f(0, Sector::SIZE, Sector::SIZE);

		glVertex3f(Sector::SIZE, Sector::SIZE, 0);
		glVertex3f(Sector::SIZE, Sector::SIZE, Sector::SIZE);

		glVertex3f(Sector::SIZE, 0, 0);
		glVertex3f(Sector::SIZE, 0, Sector::SIZE);
	glEnd();
	
	if (!(sx || sy)) glColor3f(1,1,0);
	Uint32 num=0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i) {
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
		glScalef((StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]));
		glCallList(m_gluDiskDlist);
		glScalef(2,2,2);

		// only do this once we've pretty much stopped moving.
		vector3f diff = vector3f(
				fabs(m_posMovingTo.x - m_pos.x),
				fabs(m_posMovingTo.y - m_pos.y),
				fabs(m_posMovingTo.z - m_pos.z));
		// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
		if( !(*i).IsSetInhabited() && diff.x < 0.001f && diff.y < 0.001f ) {
			StarSystem* pSS = StarSystem::GetCached(SystemPath(sx, sy, num));
			if( !pSS->m_unexplored && pSS->m_spaceStations.size()>0 ) 
			{
				(*i).SetInhabited(true);
			}
			else
			{
				(*i).SetInhabited(false);
			}
			pSS->DecRefCount();
		}
		// Pulse populated stars
		if( (*i).IsSetInhabited() && (*i).IsInhabited() )
		{
			// precise to the rendered frame (better than PHYSICS_HZ granularity)
			double preciseTime = Pi::GetGameTime() + Pi::GetGameTickAlpha()*Pi::GetTimeStep();
			float radius = 1.5f+(0.5*sin(5.0*(preciseTime+double(num))));

			// I-IS-ALIVE indicator
			glPushMatrix();
			{
				glDepthRange(0.3,1.0);
				glColor3f(0.8f,0.0f,0.0f);
				glScalef(radius,radius,radius);
				glCallList(m_gluDiskDlist);
			}
			glPopMatrix();
		}

		// player location indicator
		if ((sx == playerLoc.sectorX) && (sy == playerLoc.sectorY) && (num == playerLoc.systemIndex)) {
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
		if ((sx == m_selected.sectorX) && (sy == m_selected.sectorY) && (sz == m_selected.sectorZ) &&
		    (num == m_selected.systemIndex)) {
			glDepthRange(0.1,1.0);
			glColor3f(0,0.8,0);
			glScalef(2,2,2);
			glCallList(m_gluDiskDlist);
		}
		glDepthRange(0,1);
		glPopMatrix();
		glColor3f(.7,.7,.7);
		PutClickableLabel((*i).name, SystemPath(sx, sy, num));
		glDisable(GL_LIGHTING);

		glPopMatrix();
		num++;
	}
}

void SectorView::OnSwitchTo() {
	if (m_firstTime) {
		WarpToSystem(Pi::currentSystem->GetPath());
		m_firstTime = false;
	}
	Update();
}

void SectorView::Update()
{
	const float frameTime = Pi::GetFrameTime();

	SystemPath playerLoc = Pi::currentSystem->GetPath();

	if (Pi::KeyState(SDLK_c)) {
		GotoSystem(playerLoc);
		if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
			m_rot_x = m_rot_z = 0;
			m_zoom = 1.2;
		}
	}

	float moveSpeed = 1.0;
	if (Pi::KeyState(SDLK_LSHIFT)) moveSpeed = 100.0;
	if (Pi::KeyState(SDLK_RSHIFT)) moveSpeed = 10.0;
	
	if (Pi::KeyState(SDLK_LEFT)) m_posMovingTo.x -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_RIGHT)) m_posMovingTo.x += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_UP)) m_posMovingTo.y += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_DOWN)) m_posMovingTo.y -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(0.5f, frameTime);
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(2.0f, frameTime);
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(0.5f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(2.0f, frameTime);
	m_zoom = Clamp(m_zoom, 0.1f, 5.0f);
	
	// when zooming to a clicked on spot
	{
		vector3f diff = m_posMovingTo - m_pos;
		m_pos += diff * 10.0f*frameTime;
	}
	
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1];
		m_rot_z += motion[0];
	}

	SystemPath last_selected = m_selected;
	
	m_selected.sectorX = int(floor(m_pos.x));
	m_selected.sectorY = int(floor(m_pos.y));
	m_selected.sectorZ = int(floor(m_pos.z));

	Sector* ps = GetCached(m_selected.sectorX, m_selected.sectorY, m_selected.sectorZ);
	float px = FFRAC(m_pos.x)*Sector::SIZE;
	float py = FFRAC(m_pos.y)*Sector::SIZE;

	float min_dist = FLT_MAX;
	for (unsigned int i=0; i<ps->m_systems.size(); i++) {
		Sector::System *ss = &ps->m_systems[i];
		float dx = px - ss->p.x;
		float dy = py - ss->p.y;
		float dist = sqrtf(dx*dx + dy*dy);
		if (dist < min_dist) {
			min_dist = dist;
			m_selected.systemIndex = i;
		}
	}
	
	if (last_selected != m_selected) {
		Sector sec(m_selected.sectorX, m_selected.sectorY);
		Sector psec(playerLoc.sectorX, playerLoc.sectorY);
		const float dist = Sector::DistanceBetween(&sec, m_selected.systemIndex, &psec, playerLoc.systemIndex);

		char buf[256];
		int fuelRequired;
		double dur;
		enum Ship::HyperjumpStatus jumpStatus;
		Pi::player->CanHyperspaceTo(&m_selected, fuelRequired, dur, &jumpStatus);
		switch (jumpStatus) {
			case Ship::HYPERJUMP_OK:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (fuel required: %dt | time loss: %.1fhrs)", dist, fuelRequired, dur*0.0002778);
				Pi::player->SetHyperspaceTarget(&m_selected);
				m_distance->Color(0.0f, 1.0f, 0.2f);
				break;
			case Ship::HYPERJUMP_CURRENT_SYSTEM:
				snprintf(buf, sizeof(buf), "Current system");
				Pi::player->ClearHyperspaceTarget();
				m_distance->Color(0.0f, 1.0f, 1.0f);
				break;
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (insufficient fuel, required: %dt)", dist, fuelRequired);
				Pi::player->ClearHyperspaceTarget();
				m_distance->Color(1.0f, 1.0f, 0.0f);
				break;
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				snprintf(buf, sizeof(buf), "Dist. %.2f light years (out of range)", dist);
				Pi::player->ClearHyperspaceTarget();
				m_distance->Color(1.0f, 0.0f, 0.0f);
				break;
			case Ship::HYPERJUMP_NO_DRIVE:
				snprintf(buf, sizeof(buf), "You cannot perform a hyperjump because you do not have a functioning hyperdrive");
				Pi::player->ClearHyperspaceTarget();
				m_distance->Color(1.0f, 0.6f, 1.0f);
				break;
		}

		StarSystem *sys = StarSystem::GetCached(m_selected);

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

		m_systemName->SetText(sys->GetName());
		m_distance->SetText(buf);
		m_starType->SetText(desc);
		m_shortDesc->SetText(sys->GetShortDescription());

		sys->Release();

		// Think we'll only need to do this when our location has changed.
		ShrinkCache();
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

Sector* SectorView::GetCached(int sectorX, int sectorY, int sectorZ)
{
	const SystemPath loc(sectorX, sectorY, 0);

	Sector *s = 0;

	for (std::map<SystemPath,Sector*>::iterator i = m_sectorCache.begin(); i != m_sectorCache.end(); i++) {
		if ((*i).first == loc)
			s = (*i).second;
	}

	if (!s) {
		s = new Sector(sectorX, sectorY);
		m_sectorCache.insert( std::pair<SystemPath,Sector*>(loc, s) );
	}

	return s;
}

void SectorView::ShrinkCache()
{
	// we're going to use these to determine if our sectors are within the range that we'll ever render
	const int xmin = m_selected.sectorX-DRAW_RAD;
	const int xmax = m_selected.sectorX+DRAW_RAD;
	const int ymin = m_selected.sectorY-DRAW_RAD;
	const int ymax = m_selected.sectorY+DRAW_RAD;

	std::map<SystemPath,Sector*>::iterator iter = m_sectorCache.begin();
	while (iter != m_sectorCache.end())	{
		Sector *s = (*iter).second;
		//check_point_in_box
		if (s && !s->WithinBox( xmin, xmax, ymin, ymax )) {
			delete s;
			m_sectorCache.erase( iter++ ); 
		} else {
			iter++;
		}
	}
}
