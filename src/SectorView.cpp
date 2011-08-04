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
#include "Lang.h"

#define INNER_RADIUS (Sector::SIZE*1.5f)
#define OUTER_RADIUS (Sector::SIZE*3.0f)
		
SectorView::SectorView() :
	m_firstTime(true),
	m_matchTargetToSelection(true)
{
	SetTransparency(true);
	m_pos = m_posMovingTo = vector3f(0.0f);
	m_rot_x = -45.0f;
	m_rot_z = 0;
	m_zoom = 2.0f;

	Gui::Screen::PushFont("OverlayFont");
	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(.7f,.7f,.7f,0.75f));
	Add(m_clickableLabels, 0, 0);
	Gui::Screen::PopFont();

	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	Add(m_zoomOutButton, 732, 5);

	Add(new Gui::Label("Search:"), 650, 500);
	m_searchBox = new Gui::TextEntry();
	m_searchBox->onValueChanged.connect(sigc::mem_fun(this, &SectorView::OnSearchBoxValueChanged));
	Add(m_searchBox, 700, 500);

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
	if (m_onKeyPressConnection.connected()) m_onKeyPressConnection.disconnect();
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

void SectorView::OnSearchBoxValueChanged()
{
	const std::string search = m_searchBox->GetText();
	
	int results = 0;
	const SystemPath *lastResult = 0;

	for (std::map<SystemPath,Sector*>::iterator i = m_sectorCache.begin(); i != m_sectorCache.end(); i++) {
		
	       for (std::vector<Sector::System>::iterator j = (*i).second->m_systems.begin(); j != (*i).second->m_systems.end(); j++) {
			if ((*j).name.find(search) != std::string::npos) {
				results++;
				lastResult = &(*i).first;
			}
	       }
	}
	if (results == 1) GotoSystem(lastResult);
}

#define DRAW_RAD	3

#define FFRAC(_x)	((_x)-floor(_x))

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
	snprintf(buf, sizeof(buf), Lang::SECTOR_X_Y_Z, m_selected.sectorX, m_selected.sectorY, m_selected.sectorZ);
	m_infoLabel->SetText(buf);

	// units are lightyears, my friend
	glTranslatef(0, 0, -10-10*m_zoom);
	glDisable(GL_LIGHTING);
	{
		// draw a circle around the outer view radius	
		glColor3f(0,0,1);
		glBegin(GL_LINE_LOOP);
		for (float theta=0; theta < 2*M_PI; theta += 0.05*M_PI) {
			glVertex3f(OUTER_RADIUS*sin(theta), OUTER_RADIUS*cos(theta), 0);
		}
		glEnd();
	}
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	glTranslatef(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	
	SystemPath playerLoc = m_selected;
	Sector* playerSec = GetCached(playerLoc.sectorX, playerLoc.sectorY, playerLoc.sectorZ);
	vector3f playerPos(0.0f);
	if (m_selected.systemIndex < playerSec->m_systems.size())
	{
		playerPos = Sector::SIZE * vector3f((float)playerLoc.sectorX, (float)playerLoc.sectorY, (float)playerLoc.sectorZ)
			+ playerSec->m_systems[playerLoc.systemIndex].p;
	}
	

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				glPushMatrix();
				glTranslatef(Sector::SIZE*sx, Sector::SIZE*sy, Sector::SIZE*sz);
				DrawSector(m_selected.sectorX+sx, m_selected.sectorY+sy, m_selected.sectorZ+sz, playerPos);
				glPopMatrix();
			}
		}
	}

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

void SectorView::SetHyperspaceTarget(const SystemPath &path)
{
	m_hyperspaceTarget = path;
	m_matchTargetToSelection = false;
	onHyperspaceTargetChanged.emit();
}

void SectorView::FloatHyperspaceTarget()
{
	m_matchTargetToSelection = true;
}

void SectorView::ResetHyperspaceTarget()
{
	SystemPath old = m_hyperspaceTarget;
	m_hyperspaceTarget = m_selected;
	m_matchTargetToSelection = true;

	if (old != m_hyperspaceTarget)
		onHyperspaceTargetChanged.emit();
}

void SectorView::GotoSystem(const SystemPath &path)
{
	Sector* ps = GetCached(path.sectorX, path.sectorY, path.sectorZ);
	const vector3f &p = ps->m_systems[path.systemIndex].p;
	m_posMovingTo.x = path.sectorX + p.x/Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y/Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z/Sector::SIZE;
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

void SectorView::PutClickableLabel(const std::string &text, const Color &labelCol, const SystemPath &path)
{
	Gui::Screen::EnterOrtho();
	vector3d pos;
	if (Gui::Screen::Project(vector3d(0.0), pos)) {
		m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), path), pos.x, pos.y, labelCol);
	}
	Gui::Screen::LeaveOrtho();
}

static void _draw_arrow(const vector3f &direction)
{
	// ^^^^ !sol
	const float headRadius = 0.25f;
	glBegin(GL_LINE_STRIP);
		glVertex3f(direction.x, direction.y, direction.z);
		glVertex3f(0, 0, 0);
	glEnd();
	glDisable(GL_CULL_FACE);
	const vector3f axis1 = direction.Cross(vector3f(0,1.0f,0)).Normalized();
	const vector3f axis2 = direction.Cross(axis1).Normalized();
	vector3f p;
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(direction.x, direction.y, direction.z);
		for (float f=2*M_PI; f>0; f-=0.6) {
			p = 0.8f*direction + headRadius*sin(f)*axis1 + headRadius*cos(f)*axis2;
			glVertex3fv(&p.x);
		}
		p = 0.8f*direction + headRadius*axis2;
		glVertex3fv(&p.x);
	glEnd();
	glEnable(GL_CULL_FACE);
}

void SectorView::DrawSector(int sx, int sy, int sz, const vector3f &playerAbsPos)
{
	SystemPath playerLoc = Pi::currentSystem->GetPath();
	Sector* ps = GetCached(sx, sy, sz);

	int cz = floor(m_pos.z+0.5f);

	if (cz == sz) {
		glColor3f(0,0.2f,0);
		glBegin(GL_LINE_LOOP);
			glVertex3f(0, 0, 0);
			glVertex3f(0, Sector::SIZE, 0);
			glVertex3f(Sector::SIZE, Sector::SIZE, 0);
			glVertex3f(Sector::SIZE, 0, 0);
		glEnd();
	}

	if (!(sx || sy)) glColor3f(1,1,0);
	Uint32 num=0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i, ++num) {
		SystemPath current = SystemPath(sx, sy, sz, num);

		const vector3f sysAbsPos = Sector::SIZE*vector3f((float)sx, (float)sy, (float)sz) + (*i).p;
		const vector3f toPlayer = playerAbsPos - sysAbsPos;
		const vector3f toCentreOfView = m_pos*Sector::SIZE - sysAbsPos;
		const float distanceFade = (OUTER_RADIUS - Clamp(toCentreOfView.Length()-0.5f*INNER_RADIUS, 0.0f, OUTER_RADIUS)) / OUTER_RADIUS;

		if (toCentreOfView.Length() > OUTER_RADIUS) continue;

		if (isqrt(1 + sx*sx + sy*sy + sz*sz) <= 90) {
			// only do this once we've pretty much stopped moving.
			vector3f diff = vector3f(
					fabs(m_posMovingTo.x - m_pos.x),
					fabs(m_posMovingTo.y - m_pos.y),
					fabs(m_posMovingTo.z - m_pos.z));
			// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
			if( !(*i).IsSetInhabited() && diff.x < 0.001f && diff.y < 0.001f && diff.z < 0.001f ) {
				StarSystem* pSS = StarSystem::GetCached(current);
				if( (!pSS->m_unexplored) && (pSS->m_spaceStations.size()>0) ) 
				{
					(*i).SetInhabited(true);
				}
				else
				{
					(*i).SetInhabited(false);
				}
				pSS->Release();
			}
		}
		
		glPushMatrix();
		glTranslatef((*i).p.x, (*i).p.y, (*i).p.z);

		const float* col = StarSystem::starColors[(*i).starType[0]];
		/* only do coloured depth indicator lines linked to current system
		   if inhabited, and quite near */
		if ((toPlayer.Length() <= INNER_RADIUS) &&
		    ((*i).IsSetInhabited() && (*i).IsInhabited()) ) {
			glColor4f(col[0], col[1], col[2], 0.5f);
			glBegin(GL_LINE_STRIP);
				glVertex3f(toPlayer.x, toPlayer.y, toPlayer.z);
				glVertex3f(0, 0, toPlayer.z);
				glVertex3f(0, 0, 0);
			glEnd();
		} else {
			glColor4f(distanceFade, distanceFade, distanceFade, 0.2f);
			glBegin(GL_LINES);
				float z = -(*i).p.z;
				if (sz <= cz)
					z = z+abs(cz-sz)*Sector::SIZE;
				else
					z = z-abs(cz-sz)*Sector::SIZE;

				glVertex3f(0, 0, z);
				glVertex3f(0, 0, 0);

				glVertex3f(-0.1f, -0.1f, z);
				glVertex3f(0.1f, 0.1f, z);
				glVertex3f(-0.1f, 0.1f, z);
				glVertex3f(0.1f, -0.1f, z);
			glEnd();
		}

		if (current == m_selected && current != SystemPath(0,0,0,0)) {
			glColor4f(0, 0.8f, 0, 1.0f);
			_draw_arrow(-3.0f*sysAbsPos.Normalized());
		}

		// draw star blob itself
		glColor4f(col[0], col[1], col[2], 1.0f);
		glPushMatrix();
		glRotatef(-m_rot_z, 0, 0, 1);
		glRotatef(-m_rot_x, 1, 0, 0);
		glScalef((StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]));
		glCallList(m_gluDiskDlist);
		glScalef(2,2,2);

		// player location indicator
		if (current == playerLoc) {
			glPushMatrix();
			glDepthRange(0.2,1.0);
			glColor3f(0,0,0.8);
			glScalef(3,3,3);
			glCallList(m_gluDiskDlist);
			glPopMatrix();
		}
		// selected indicator
		if (current == m_selected) {
			glPushMatrix();
			glDepthRange(0.1,1.0);
			glColor3f(0,0.8,0);
			glScalef(2,2,2);
			glCallList(m_gluDiskDlist);
			glPopMatrix();
		}
		// hyperspace target indicator (if different from selection)
		if (current == m_hyperspaceTarget && m_hyperspaceTarget != m_selected && m_hyperspaceTarget != playerLoc) {
			glPushMatrix();
			glDepthRange(0.1,1.0);
			glColor3f(0.3,0.3,0.3);
			glScalef(2,2,2);
			glCallList(m_gluDiskDlist);
			glPopMatrix();
		}
		glDepthRange(0,1);
		glPopMatrix();
		Color labelColor(0.5f,0.5f,0.5f,0.75f);
		if ((*i).IsSetInhabited() && (*i).IsInhabited()) {
			labelColor.b = labelColor.g = 1.0f;
		}
		labelColor *= distanceFade;

		PutClickableLabel((*i).name, labelColor, SystemPath(sx, sy, sz, num));
		glDisable(GL_LIGHTING);

		glPopMatrix();
	}
}

void SectorView::OnSwitchTo() {
	if (m_firstTime) {
		WarpToSystem(Pi::currentSystem->GetPath());
		m_firstTime = false;
	}
	
	if (!m_onKeyPressConnection.connected())
		m_onKeyPressConnection =
			Pi::onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnKeyPress));

	Update();
}

void SectorView::OnKeyPress(SDL_keysym *keysym)
{
	if (Pi::GetView() != this) {
		m_onKeyPressConnection.disconnect();
		return;
	}

	SystemPath playerLoc = Pi::currentSystem->GetPath();

	// space "locks" (or unlocks) the hyperspace target to the selected system
	if (keysym->sym == SDLK_SPACE) {
		if ((m_matchTargetToSelection || m_hyperspaceTarget != m_selected) && !m_selected.IsSameSystem(playerLoc))
			SetHyperspaceTarget(m_selected);
		else
			ResetHyperspaceTarget();
	}

	// fast move selection to current player system or hyperspace target
	if (Pi::KeyState(SDLK_c) || Pi::KeyState(SDLK_h)) {
		if (Pi::KeyState(SDLK_c))
			GotoSystem(playerLoc);
		else
			GotoSystem(m_hyperspaceTarget);

		if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
			m_rot_x = -45.0f;
			m_rot_z = 0;
			m_zoom = 2.0f;
		}
	}

}

void SectorView::Update()
{
	const float frameTime = Pi::GetFrameTime();

	float moveSpeed = 1.0;
	if (Pi::KeyState(SDLK_LSHIFT)) moveSpeed = 100.0;
	if (Pi::KeyState(SDLK_RSHIFT)) moveSpeed = 10.0;
	
	if (Pi::KeyState(SDLK_LEFT)) m_posMovingTo.x -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_RIGHT)) m_posMovingTo.x += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_UP)) m_posMovingTo.y += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_DOWN)) m_posMovingTo.y -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_PAGEUP)) m_posMovingTo.z += moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_PAGEDOWN)) m_posMovingTo.z -= moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(0.5f, frameTime);
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(2.0f, frameTime);
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(0.5f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(2.0f, frameTime);
	m_zoom = Clamp(m_zoom, 0.1f, 5.0f);
	
	// when zooming to a clicked on spot
	{
		vector3f diff = m_posMovingTo - m_pos;
		vector3f travel = diff * 10.0f*frameTime;
		if (travel.Length() > diff.Length()) m_pos = m_posMovingTo;
		else m_pos = m_pos + travel;
	}
	
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += 0.2f*(float)motion[1];
		m_rot_z += 0.2f*(float)motion[0];
	}
	// clamp x rotation because without it, getting lost is easier
	m_rot_x = Clamp(m_rot_x, -170.0f, -10.0f);

	SystemPath last_selected = m_selected;
	
	m_selected = SystemPath(int(floor(m_pos.x)), int(floor(m_pos.y)), int(floor(m_pos.z)), 0);

	Sector* ps = GetCached(m_selected.sectorX, m_selected.sectorY, m_selected.sectorZ);
	float px = FFRAC(m_pos.x)*Sector::SIZE;
	float py = FFRAC(m_pos.y)*Sector::SIZE;
	float pz = FFRAC(m_pos.z)*Sector::SIZE;

	float min_dist = FLT_MAX;
	for (unsigned int i=0; i<ps->m_systems.size(); i++) {
		Sector::System *ss = &ps->m_systems[i];
		float dx = px - ss->p.x;
		float dy = py - ss->p.y;
		float dz = pz - ss->p.z;
		float dist = sqrtf(dx*dx + dy*dy + dz*dz);
		if (dist < min_dist) {
			min_dist = dist;
			m_selected.systemIndex = i;
		}
	}

	if (last_selected != m_selected) {
		SystemPath playerLoc = Pi::currentSystem->GetPath();

		Sector sec(m_selected.sectorX, m_selected.sectorY, m_selected.sectorZ);
		Sector psec(playerLoc.sectorX, playerLoc.sectorY, playerLoc.sectorZ);
		if (m_selected.systemIndex < sec.m_systems.size()) {
			if (m_matchTargetToSelection) {
				m_hyperspaceTarget = m_selected;
				onHyperspaceTargetChanged.emit();
			}
	
			const float dist = Sector::DistanceBetween(&sec, m_selected.systemIndex, &psec, playerLoc.systemIndex);

			char buf[256];
			int fuelRequired;
			double dur;
			enum Ship::HyperjumpStatus jumpStatus;
			Pi::player->CanHyperspaceTo(&m_selected, fuelRequired, dur, &jumpStatus);
			switch (jumpStatus) {
				case Ship::HYPERJUMP_OK:
					snprintf(buf, sizeof(buf), "Dist. %.2f light years (fuel required: %dt | time loss: %.1fhrs)", dist, fuelRequired, dur*0.0002778);
					m_distance->Color(0.0f, 1.0f, 0.2f);
					break;
				case Ship::HYPERJUMP_CURRENT_SYSTEM:
					snprintf(buf, sizeof(buf), "Current system");
					m_distance->Color(0.0f, 1.0f, 1.0f);
					break;
				case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
					snprintf(buf, sizeof(buf), "Dist. %.2f light years (insufficient fuel, required: %dt)", dist, fuelRequired);
					m_distance->Color(1.0f, 1.0f, 0.0f);
					break;
				case Ship::HYPERJUMP_OUT_OF_RANGE:
					snprintf(buf, sizeof(buf), "Dist. %.2f light years (out of range)", dist);
					m_distance->Color(1.0f, 0.0f, 0.0f);
					break;
				case Ship::HYPERJUMP_NO_DRIVE:
					snprintf(buf, sizeof(buf), "You cannot perform a hyperjump because you do not have a functioning hyperdrive");
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
	const SystemPath loc(sectorX, sectorY, sectorZ, 0);

	Sector *s = 0;

	for (std::map<SystemPath,Sector*>::iterator i = m_sectorCache.begin(); i != m_sectorCache.end(); i++) {
		if ((*i).first == loc)
			s = (*i).second;
	}

	if (!s) {
		s = new Sector(sectorX, sectorY, sectorZ);
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
	const int zmin = m_selected.sectorZ-DRAW_RAD;
	const int zmax = m_selected.sectorZ+DRAW_RAD;

	std::map<SystemPath,Sector*>::iterator iter = m_sectorCache.begin();
	while (iter != m_sectorCache.end())	{
		Sector *s = (*iter).second;
		//check_point_in_box
		if (s && !s->WithinBox( xmin, xmax, ymin, ymax, zmin, zmax )) {
			delete s;
			m_sectorCache.erase( iter++ ); 
		} else {
			iter++;
		}
	}
}
