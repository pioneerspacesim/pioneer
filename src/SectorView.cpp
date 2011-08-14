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
#include "ShipCpanel.h"

#define INNER_RADIUS (Sector::SIZE*1.5f)
#define OUTER_RADIUS (Sector::SIZE*3.0f)
		
SectorView::SectorView() :
	m_firstTime(true),
	m_selectionFollowsMovement(true),
	m_infoBoxVisible(true)
{
	SetTransparency(true);
	m_pos = m_posMovingTo = vector3f(0.0f);
	m_rotX = m_rotXMovingTo = -10.0f;
	m_rotZ = m_rotZMovingTo = 0;
	m_zoom = 2.0f;

	Gui::Screen::PushFont("OverlayFont");
	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(.7f,.7f,.7f,0.75f));
	Add(m_clickableLabels, 0, 0);
	Gui::Screen::PopFont();

	m_sectorLabel = new Gui::Label("");
	Add(m_sectorLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()*2-66);
	m_distanceLabel = new Gui::Label("");
	Add(m_distanceLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);
	
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
	
	m_infoBox = new Gui::VBox();
	m_infoBox->SetTransparency(false);
	m_infoBox->SetBgColor(1.0f, 1.0f, 1.0f, 0.05f);
	m_infoBox->SetSpacing(5.0f);
	Add(m_infoBox, 5, 5);

	Gui::VBox *systemBox = new Gui::VBox();
	Gui::HBox *hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoCurrentSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label("Current system"))->Color(1.0f, 1.0f, 1.0f));
	systemBox->PackEnd(hbox);
	m_currentSystemLabels.systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	m_currentSystemLabels.distance = 0;
	m_currentSystemLabels.starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_currentSystemLabels.shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	systemBox->PackEnd(m_currentSystemLabels.systemName);
	systemBox->PackEnd(m_currentSystemLabels.starType);
	systemBox->PackEnd(m_currentSystemLabels.shortDesc);
	m_infoBox->PackEnd(systemBox);

	systemBox = new Gui::VBox();
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoSelectedSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label("Selected system"))->Color(1.0f, 1.0f, 1.0f));
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_selectedSystemLabels.systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	m_selectedSystemLabels.distance = (new Gui::Label(""))->Color(1.0f, 0.0f, 0.0f);
	hbox->PackEnd(m_selectedSystemLabels.systemName);
	hbox->PackEnd(m_selectedSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_selectedSystemLabels.starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_selectedSystemLabels.shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	systemBox->PackEnd(m_selectedSystemLabels.starType);
	systemBox->PackEnd(m_selectedSystemLabels.shortDesc);
	m_infoBox->PackEnd(systemBox);

	systemBox = new Gui::VBox();
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoHyperspaceTarget));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label("Hyperspace target"))->Color(1.0f, 1.0f, 1.0f));
    m_hyperspaceLockLabel = (new Gui::Label(""))->Color(1.0f, 1.0f, 1.0f);
    hbox->PackEnd(m_hyperspaceLockLabel);
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_targetSystemLabels.systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	m_targetSystemLabels.distance = (new Gui::Label(""))->Color(1.0f, 0.0f, 0.0f);
	hbox->PackEnd(m_targetSystemLabels.systemName);
	hbox->PackEnd(m_targetSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_targetSystemLabels.starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_targetSystemLabels.shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	systemBox->PackEnd(m_targetSystemLabels.starType);
	systemBox->PackEnd(m_targetSystemLabels.shortDesc);
	m_infoBox->PackEnd(systemBox);

	m_onMouseButtonDown = 
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &SectorView::MouseButtonDown));
	
	FloatHyperspaceTarget();
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
	m_current.Serialize(wr);
	m_selected.Serialize(wr);
	m_hyperspaceTarget.Serialize(wr);
	wr.Float(m_pos.x);
	wr.Float(m_pos.y);
	wr.Float(m_pos.z);
	wr.Float(m_rotX);
	wr.Float(m_rotZ);
	wr.Bool(m_matchTargetToSelection);
	wr.Bool(m_selectionFollowsMovement);
}

void SectorView::Load(Serializer::Reader &rd)
{
	m_zoom = rd.Float();
	m_current = SystemPath::Unserialize(rd);
	m_selected = SystemPath::Unserialize(rd);
	m_hyperspaceTarget = SystemPath::Unserialize(rd);
	m_pos.x = m_posMovingTo.x = rd.Float();
	m_pos.y = m_posMovingTo.y = rd.Float();
	m_pos.z = m_posMovingTo.z = rd.Float();
	m_rotX = m_rotXMovingTo = rd.Float();
	m_rotZ = m_rotZMovingTo = rd.Float();
	m_matchTargetToSelection = rd.Bool();
	m_selectionFollowsMovement = rd.Bool();

	UpdateSystemLabels(m_currentSystemLabels, m_current);
	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);

	m_hyperspaceLockLabel->SetText(m_matchTargetToSelection ? "[following selection]" : "[locked]");

	m_firstTime = false;
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
	
	m_sectorLabel->SetText(stringf(128, Lang::SECTOR_X_Y_Z, int(floorf(m_pos.x)), int(floorf(m_pos.y)), int(floorf(m_pos.z))));

	vector3f dv = vector3f(floorf(m_pos.x)-m_current.sectorX, floorf(m_pos.y)-m_current.sectorY, floorf(m_pos.z)-m_current.sectorZ) * Sector::SIZE;
	m_distanceLabel->SetText(stringf(128, Lang::DISTANCE_LY, dv.Length()));

	glDisable(GL_LIGHTING);

	// units are lightyears, my friend
	glTranslatef(0, 0, -10-10*m_zoom);
	glRotatef(m_rotX, 1, 0, 0);
	glRotatef(m_rotZ, 0, 0, 1);
	glTranslatef(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	
	Sector* playerSec = GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
	vector3f playerPos(0.0f);
	if (m_selected.systemIndex < playerSec->m_systems.size())
	{
		playerPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ))
			+ playerSec->m_systems[m_current.systemIndex].p;
	}
	

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				glPushMatrix();
				glTranslatef(Sector::SIZE*sx, Sector::SIZE*sy, Sector::SIZE*sz);
				DrawSector(int(floorf(m_pos.x))+sx, int(floorf(m_pos.y))+sy, int(floorf(m_pos.z))+sz, playerPos);
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

	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);

	m_hyperspaceLockLabel->SetText("[locked]");
}

void SectorView::FloatHyperspaceTarget()
{
	m_matchTargetToSelection = true;
	m_hyperspaceLockLabel->SetText("[following selection]");
}

void SectorView::ResetHyperspaceTarget()
{
	SystemPath old = m_hyperspaceTarget;
	m_hyperspaceTarget = m_selected;
	FloatHyperspaceTarget();

	if (old != m_hyperspaceTarget) {
		onHyperspaceTargetChanged.emit();
		UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);
	}
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

void SectorView::SetSelectedSystem(const SystemPath &path)
{
    m_selected = path;

	if (m_matchTargetToSelection) {
		m_hyperspaceTarget = m_selected;
		onHyperspaceTargetChanged.emit();
		UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);
	}

	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
}

void SectorView::OnClickSystem(const SystemPath &path)
{
	if (m_selectionFollowsMovement)
		GotoSystem(path);
	else
		SetSelectedSystem(path);
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

void SectorView::UpdateSystemLabels(SystemLabels &labels, const SystemPath &path)
{
	Sector *sec = GetCached(path.sectorX, path.sectorY, path.sectorZ);
	Sector *playerSec = GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
	const float dist = Sector::DistanceBetween(sec, path.systemIndex, playerSec, m_current.systemIndex);

    if (labels.distance) {
		char buf[256];
	
		int fuelRequired;
		double dur;
		enum Ship::HyperjumpStatus jumpStatus;
		Pi::player->CanHyperspaceTo(&path, fuelRequired, dur, &jumpStatus);
	
		switch (jumpStatus) {
			case Ship::HYPERJUMP_OK:
				snprintf(buf, sizeof(buf), "[ %.2f ly | %dt | %.1f hrs ]", dist, fuelRequired, dur*0.0002778);
				labels.distance->Color(0.0f, 1.0f, 0.2f);
				break;
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				snprintf(buf, sizeof(buf), "[ %.2f ly | %dt ]", dist, fuelRequired);
				labels.distance->Color(1.0f, 1.0f, 0.0f);
				break;
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				snprintf(buf, sizeof(buf), "[ %.2f ly ]", dist);
				labels.distance->Color(1.0f, 0.0f, 0.0f);
				break;
			default:
				buf[0] = '\0';
				break;
		}
		labels.distance->SetText(buf);
	}

	StarSystem *sys = StarSystem::GetCached(path);

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
	labels.starType->SetText(desc);

	labels.systemName->SetText(sys->GetName());
	labels.shortDesc->SetText(sys->GetShortDescription());

	sys->Release();

	if (m_infoBoxVisible)
		m_infoBox->ShowAll();
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

		const vector3f sysAbsPos = Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + (*i).p;
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
		glRotatef(-m_rotZ, 0, 0, 1);
		glRotatef(-m_rotX, 1, 0, 0);
		glScalef((StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]),
			(StarSystem::starScale[(*i).starType[0]]));
		glCallList(m_gluDiskDlist);
		glScalef(2,2,2);

		// player location indicator
		if (current == m_current) {
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
		if (current == m_hyperspaceTarget && m_hyperspaceTarget != m_selected && m_hyperspaceTarget != m_current) {
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
		SystemPath m_current = Pi::currentSystem->GetPath();

		UpdateSystemLabels(m_currentSystemLabels, m_current);

		WarpToSystem(m_current);
		OnClickSystem(m_current);

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

	// space "locks" (or unlocks) the hyperspace target to the selected system
	if (keysym->sym == SDLK_SPACE) {
		if ((m_matchTargetToSelection || m_hyperspaceTarget != m_selected) && !m_selected.IsSameSystem(m_current))
			SetHyperspaceTarget(m_selected);
		else
			ResetHyperspaceTarget();
	}

	// toggle the info box
	if (keysym->sym == SDLK_TAB) {
		m_infoBoxVisible = !m_infoBoxVisible;
		if (m_infoBoxVisible)
			m_infoBox->ShowAll();
		else
			m_infoBox->HideAll();
	}

	// toggle selection mode
	if (keysym->sym == SDLK_RETURN) {
		m_selectionFollowsMovement = !m_selectionFollowsMovement;
		if (m_selectionFollowsMovement)
			Pi::cpan->MsgLog()->Message("", "Enabled automatic system selection");
		else
			Pi::cpan->MsgLog()->Message("", "Disabled automatic system selection");
	}

	// fast move selection to current player system or hyperspace target
	if (Pi::KeyState(SDLK_c) || Pi::KeyState(SDLK_h)) {
		if (Pi::KeyState(SDLK_c))
			GotoSystem(m_current);
		else
			GotoSystem(m_hyperspaceTarget);

		if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
			while (m_rotZ < -180.0f) m_rotZ += 360.0f;
			while (m_rotZ > 180.0f)  m_rotZ -= 360.0f;
			m_rotXMovingTo = -10.0f;
			m_rotZMovingTo = 0;
			m_zoom = 2.0f;
		}
	}
}

void SectorView::Update()
{
	SystemPath last_current = m_current;
	m_current = Pi::currentSystem->GetPath();
	if (last_current != m_current)
		UpdateSystemLabels(m_currentSystemLabels, m_current);

	const float frameTime = Pi::GetFrameTime();

	matrix4x4f rot = matrix4x4f::Identity();
	rot.RotateX(DEG2RAD(-m_rotX));
	rot.RotateZ(DEG2RAD(-m_rotZ));

	float moveSpeed = 1.0;
	if (Pi::KeyState(SDLK_LSHIFT)) moveSpeed = 100.0;
	if (Pi::KeyState(SDLK_RSHIFT)) moveSpeed = 10.0;
	
	float move = moveSpeed*frameTime;
	if (Pi::KeyState(SDLK_LEFT) || Pi::KeyState(SDLK_RIGHT))
		m_posMovingTo += vector3f(Pi::KeyState(SDLK_LEFT) ? -move : move, 0,0) * rot;
	if (Pi::KeyState(SDLK_UP) || Pi::KeyState(SDLK_DOWN))
		m_posMovingTo += vector3f(0, Pi::KeyState(SDLK_DOWN) ? -move : move, 0) * rot;
	if (Pi::KeyState(SDLK_PAGEUP) || Pi::KeyState(SDLK_PAGEDOWN))
		m_posMovingTo += vector3f(0,0, Pi::KeyState(SDLK_PAGEUP) ? -move : move) * rot;

	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(0.5f, frameTime);
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(2.0f, frameTime);
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(0.5f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(2.0f, frameTime);
	m_zoom = Clamp(m_zoom, 0.1f, 5.0f);
	
	if (Pi::KeyState(SDLK_a) || Pi::KeyState(SDLK_d))
		m_rotZMovingTo += (Pi::KeyState(SDLK_a) ? -0.5f : 0.5f) * moveSpeed;
	if (Pi::KeyState(SDLK_w) || Pi::KeyState(SDLK_s))
		m_rotXMovingTo += (Pi::KeyState(SDLK_w) ? -0.5f : 0.5f) * moveSpeed;

	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);

		m_rotXMovingTo += 0.2f*float(motion[1]);
		m_rotZMovingTo += 0.2f*float(motion[0]);
	}

	m_rotXMovingTo = Clamp(m_rotXMovingTo, -170.0f, -10.0f);

	{
		vector3f diffPos = m_posMovingTo - m_pos;
		vector3f travelPos = diffPos * 10.0f*frameTime;
		if (travelPos.Length() > diffPos.Length()) m_pos = m_posMovingTo;
		else m_pos = m_pos + travelPos;

		float diffX = m_rotXMovingTo - m_rotX;
		float travelX = diffX * 10.0f*frameTime;
		if (fabs(travelX) > fabs(diffX)) m_rotX = m_rotXMovingTo;
		else m_rotX = m_rotX + travelX;

		float diffZ = m_rotZMovingTo - m_rotZ;
		float travelZ = diffZ * 10.0f*frameTime;
		if (fabs(travelZ) > fabs(diffZ)) m_rotZ = m_rotZMovingTo;
		else m_rotZ = m_rotZ + travelZ;
	}

	if (m_selectionFollowsMovement) {
		SystemPath new_selected = SystemPath(int(floor(m_pos.x)), int(floor(m_pos.y)), int(floor(m_pos.z)), 0);

		Sector* ps = GetCached(new_selected.sectorX, new_selected.sectorY, new_selected.sectorZ);
		if (ps->m_systems.size()) {
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
					new_selected.systemIndex = i;
				}
			}

			if (m_selected != new_selected)
				SetSelectedSystem(new_selected);
		}
	}

	ShrinkCache();
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

	std::map<SystemPath,Sector*>::iterator i = m_sectorCache.find(loc);
	if (i != m_sectorCache.end())
		return (*i).second;

	s = new Sector(sectorX, sectorY, sectorZ);
	m_sectorCache.insert( std::pair<SystemPath,Sector*>(loc, s) );

	return s;
}

void SectorView::ShrinkCache()
{
	// we're going to use these to determine if our sectors are within the range that we'll ever render
	const int xmin = floorf(m_pos.x)-DRAW_RAD;
	const int xmax = ceilf(m_pos.x)+DRAW_RAD;
	const int ymin = floorf(m_pos.y)-DRAW_RAD;
	const int ymax = ceilf(m_pos.y)+DRAW_RAD;
	const int zmin = floorf(m_pos.z)-DRAW_RAD;
	const int zmax = ceilf(m_pos.z)+DRAW_RAD;

	// XXX don't clear the current/selected/target sectors

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
