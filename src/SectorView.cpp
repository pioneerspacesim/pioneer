// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "gui/Gui.h"
#include "Pi.h"
#include "SectorView.h"
#include "galaxy/Sector.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "galaxy/StarSystem.h"
#include "GalacticView.h"
#include "Lang.h"
#include "StringF.h"
#include "ShipCpanel.h"
#include "Game.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include <algorithm>
#include <sstream>

using namespace Graphics;

#define INNER_RADIUS (Sector::SIZE*1.5f)
#define OUTER_RADIUS (Sector::SIZE*3.0f)
static const float ZOOM_SPEED = 15;
static const float WHEEL_SENSITIVITY = .03f;		// Should be a variable in user settings.

SectorView::SectorView()
{
	InitDefaults();

	m_rotX = m_rotXMovingTo = m_rotXDefault;
	m_rotZ = m_rotZMovingTo = m_rotZDefault;
	m_zoom = m_zoomMovingTo = m_zoomDefault;

	m_inSystem = true;

	m_current = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	assert(!m_current.IsSectorPath());
	m_current = m_current.SystemOnly();

	m_selected = m_hyperspaceTarget = m_current;

	GotoSystem(m_current);
	m_pos = m_posMovingTo;

	m_matchTargetToSelection = true;
	m_selectionFollowsMovement = true;
	m_infoBoxVisible = true;

	InitObject();
}

SectorView::SectorView(Serializer::Reader &rd)
{
	InitDefaults();

	m_pos.x = m_posMovingTo.x = rd.Float();
	m_pos.y = m_posMovingTo.y = rd.Float();
	m_pos.z = m_posMovingTo.z = rd.Float();
	m_rotX = m_rotXMovingTo = rd.Float();
	m_rotZ = m_rotZMovingTo = rd.Float();
	m_zoom = m_zoomMovingTo = rd.Float();
	m_inSystem = rd.Bool();
	m_current = SystemPath::Unserialize(rd);
	m_selected = SystemPath::Unserialize(rd);
	m_hyperspaceTarget = SystemPath::Unserialize(rd);
	m_matchTargetToSelection = rd.Bool();
	m_selectionFollowsMovement = rd.Bool();
	m_infoBoxVisible = rd.Bool();

	InitObject();
}

void SectorView::InitDefaults()
{
	m_rotXDefault = Pi::config->Float("SectorViewXRotation");
	m_rotZDefault = Pi::config->Float("SectorViewZRotation");
	m_zoomDefault = Pi::config->Float("SectorViewZoom");
	m_rotXDefault = Clamp(m_rotXDefault, -170.0f, -10.0f);
	m_zoomDefault = Clamp(m_zoomDefault, 0.1f, 5.0f);
	m_previousSearch = "";
}

void SectorView::InitObject()
{
	SetTransparency(true);

	Gui::Screen::PushFont("OverlayFont");
	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(.7f,.7f,.7f,0.75f));
	Add(m_clickableLabels, 0, 0);
	Gui::Screen::PopFont();

	m_sectorLabel = new Gui::Label("");
	Add(m_sectorLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()*2-66);
	m_distanceLabel = new Gui::Label("");
	Add(m_distanceLabel, 2, Gui::Screen::GetHeight()-Gui::Screen::GetFontHeight()-66);

	m_zoomInButton = new Gui::ImageButton("icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	m_zoomInButton->SetRenderDimensions(30, 22);
	Add(m_zoomInButton, 700, 5);

	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	m_zoomOutButton->SetRenderDimensions(30, 22);
	Add(m_zoomOutButton, 732, 5);

	Add(new Gui::Label(Lang::SEARCH), 650, 500);
	m_searchBox = new Gui::TextEntry();
	m_searchBox->onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnSearchBoxKeyPress));
	Add(m_searchBox, 700, 500);

	m_disk.Reset(new Graphics::Drawables::Disk(Pi::renderer, Color::WHITE, 0.2f));

	m_infoBox = new Gui::VBox();
	m_infoBox->SetTransparency(false);
	m_infoBox->SetBgColor(0.05f, 0.05f, 0.12f, 0.5f);
	m_infoBox->SetSpacing(5.0f);
	Add(m_infoBox, 5, 5);

	Gui::VBox *systemBox = new Gui::VBox();
	Gui::HBox *hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoCurrentSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label(Lang::CURRENT_SYSTEM))->Color(1.0f, 1.0f, 1.0f));
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_currentSystemLabels.systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	m_currentSystemLabels.distance = (new Gui::Label(""))->Color(1.0f, 0.0f, 0.0f);
	hbox->PackEnd(m_currentSystemLabels.systemName);
	hbox->PackEnd(m_currentSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_currentSystemLabels.starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_currentSystemLabels.shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	systemBox->PackEnd(m_currentSystemLabels.starType);
	systemBox->PackEnd(m_currentSystemLabels.shortDesc);
	m_infoBox->PackEnd(systemBox);

	systemBox = new Gui::VBox();
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoSelectedSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label(Lang::SELECTED_SYSTEM))->Color(1.0f, 1.0f, 1.0f));
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
	hbox->PackEnd((new Gui::Label(Lang::HYPERSPACE_TARGET))->Color(1.0f, 1.0f, 1.0f));
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

	UpdateSystemLabels(m_currentSystemLabels, m_current);
	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);

	UpdateHyperspaceLockLabel();
}

SectorView::~SectorView()
{
	m_onMouseButtonDown.disconnect();
	if (m_onKeyPressConnection.connected()) m_onKeyPressConnection.disconnect();
}

void SectorView::Save(Serializer::Writer &wr)
{
	wr.Float(m_pos.x);
	wr.Float(m_pos.y);
	wr.Float(m_pos.z);
	wr.Float(m_rotX);
	wr.Float(m_rotZ);
	wr.Float(m_zoom);
	wr.Bool(m_inSystem);
	m_current.Serialize(wr);
	m_selected.Serialize(wr);
	m_hyperspaceTarget.Serialize(wr);
	wr.Bool(m_matchTargetToSelection);
	wr.Bool(m_selectionFollowsMovement);
	wr.Bool(m_infoBoxVisible);
}

void SectorView::OnSearchBoxKeyPress(const SDL_keysym *keysym)
{
	//remember the last search text, hotkey: up
	if (m_searchBox->GetText().empty() && keysym->sym == SDLK_UP && !m_previousSearch.empty())
		m_searchBox->SetText(m_previousSearch);

	if (keysym->sym != SDLK_KP_ENTER && keysym->sym != SDLK_RETURN)
		return;

	std::string search = m_searchBox->GetText();
	if (!search.size())
		return;

	m_previousSearch = search;

	//Try to detect if user entered a sector address, comma or space separated, strip parentheses
	//system index is unreliable, so it is not supported
	try {
		GotoSector(SystemPath::Parse(search.c_str()));
		return;
	} catch (SystemPath::ParseFailure) {}

	bool gotMatch = false, gotStartMatch = false;
	SystemPath bestMatch;
	const std::string *bestMatchName = 0;

	for (std::map<SystemPath,Sector*>::iterator i = m_sectorCache.begin(); i != m_sectorCache.end(); ++i)

		for (unsigned int systemIndex = 0; systemIndex < (*i).second->m_systems.size(); systemIndex++) {
			const Sector::System *ss = &((*i).second->m_systems[systemIndex]);

			// compare with the start of the current system
			if (strncasecmp(search.c_str(), ss->name.c_str(), search.size()) == 0) {

				// matched, see if they're the same size
				if (search.size() == ss->name.size()) {

					// exact match, take it and go
					SystemPath path = (*i).first;
					path.systemIndex = systemIndex;
					Pi::cpan->MsgLog()->Message("", stringf(Lang::EXACT_MATCH_X, formatarg("system", ss->name)));
					GotoSystem(path);
					return;
				}

				// partial match at start of name
				if (!gotMatch || !gotStartMatch || bestMatchName->size() > ss->name.size()) {

					// don't already have one or its shorter than the previous
					// one, take it
					bestMatch = (*i).first;
					bestMatch.systemIndex = systemIndex;
					bestMatchName = &(ss->name);
					gotMatch = gotStartMatch = true;
				}

				continue;
			}

			// look for the search term somewhere within the current system
			if (pi_strcasestr(ss->name.c_str(), search.c_str())) {

				// found it
				if (!gotMatch || !gotStartMatch || bestMatchName->size() > ss->name.size()) {

					// best we've found so far, take it
					bestMatch = (*i).first;
					bestMatch.systemIndex = systemIndex;
					bestMatchName = &(ss->name);
					gotMatch = true;
				}
			}
		}

	if (gotMatch) {
		Pi::cpan->MsgLog()->Message("", stringf(Lang::NOT_FOUND_BEST_MATCH_X, formatarg("system", *bestMatchName)));
		GotoSystem(bestMatch);
	}

	else
		Pi::cpan->MsgLog()->Message("", Lang::NOT_FOUND);
}


#define DRAW_RAD	3

#define FFRAC(_x)	((_x)-floor(_x))

void SectorView::Draw3D()
{
	m_clickableLabels->Clear();

	m_renderer->SetPerspectiveProjection(40.f, Pi::GetScrAspect(), 1.f, 100.f);

	matrix4x4f modelview = matrix4x4f::Identity();
	m_renderer->ClearScreen();

	m_sectorLabel->SetText(stringf(Lang::SECTOR_X_Y_Z,
		formatarg("x", int(floorf(m_pos.x))),
		formatarg("y", int(floorf(m_pos.y))),
		formatarg("z", int(floorf(m_pos.z)))));

	if (m_inSystem) {
		vector3f dv = vector3f(floorf(m_pos.x)-m_current.sectorX, floorf(m_pos.y)-m_current.sectorY, floorf(m_pos.z)-m_current.sectorZ) * Sector::SIZE;
		m_distanceLabel->SetText(stringf(Lang::DISTANCE_LY, formatarg("distance", dv.Length())));
	}
	else {
		m_distanceLabel->SetText("");
	}

	// units are lightyears, my friend
	modelview.Translate(0.f, 0.f, -10.f-10.f*m_zoom);
	modelview.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	modelview.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);
	modelview.Translate(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);
	m_renderer->SetTransform(modelview);

	m_renderer->SetBlendMode(BLEND_ALPHA);

	Sector *playerSec = GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
	vector3f playerPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) + playerSec->m_systems[m_current.systemIndex].p;

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				DrawSector(int(floorf(m_pos.x))+sx, int(floorf(m_pos.y))+sy, int(floorf(m_pos.z))+sz, playerPos,
					modelview * matrix4x4f::Translation(Sector::SIZE*sx, Sector::SIZE*sy, Sector::SIZE*sz));
			}
		}
	}

	m_renderer->SetBlendMode(BLEND_SOLID);
}

void SectorView::SetHyperspaceTarget(const SystemPath &path)
{
	m_hyperspaceTarget = path;
	m_matchTargetToSelection = false;
	onHyperspaceTargetChanged.emit();

	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);

	UpdateHyperspaceLockLabel();
}

void SectorView::FloatHyperspaceTarget()
{
	m_matchTargetToSelection = true;
	UpdateHyperspaceLockLabel();
}

void SectorView::UpdateHyperspaceLockLabel()
{
	m_hyperspaceLockLabel->SetText(stringf("[%0]", m_matchTargetToSelection ? std::string(Lang::FOLLOWING_SELECTION) : std::string(Lang::LOCKED)));
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

void SectorView::GotoSector(const SystemPath &path)
{
	m_posMovingTo = vector3f(path.sectorX, path.sectorY, path.sectorZ);
}

void SectorView::GotoSystem(const SystemPath &path)
{
	Sector* ps = GetCached(path.sectorX, path.sectorY, path.sectorZ);
	const vector3f &p = ps->m_systems[path.systemIndex].p;
	m_posMovingTo.x = path.sectorX + p.x/Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y/Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z/Sector::SIZE;
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

	char format[256];

	if (m_inSystem) {
		const float dist = Sector::DistanceBetween(sec, path.systemIndex, playerSec, m_current.systemIndex);

		int fuelRequired;
		double dur;
		enum Ship::HyperjumpStatus jumpStatus
			= Pi::player->GetHyperspaceDetails(&path, fuelRequired, dur);
		const double DaysNeeded = dur*(1.0 / (24*60*60));
		const double HoursNeeded = (DaysNeeded - floor(DaysNeeded))*24;

		switch (jumpStatus) {
			case Ship::HYPERJUMP_OK:
				snprintf(format, sizeof(format), "[ %s | %s | %s, %s ]", Lang::NUMBER_LY, Lang::NUMBER_TONNES, Lang::NUMBER_DAYS, Lang::NUMBER_HOURS);
				labels.distance->SetText(stringf(format,
					formatarg("distance", dist), formatarg("mass", fuelRequired), formatarg("days", floor(DaysNeeded)), formatarg("hours", HoursNeeded)));
				labels.distance->Color(0.0f, 1.0f, 0.2f);
				m_jumpLine.SetColor(Color(0.f, 1.f, 0.2f, 1.f));
				break;
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				snprintf(format, sizeof(format), "[ %s | %s ]", Lang::NUMBER_LY, Lang::NUMBER_TONNES);
				labels.distance->SetText(stringf(format,
					formatarg("distance", dist), formatarg("mass", fuelRequired)));
				labels.distance->Color(1.0f, 1.0f, 0.0f);
				m_jumpLine.SetColor(Color(1.f, 1.f, 0.f, 1.f));
				break;
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				snprintf(format, sizeof(format), "[ %s ]", Lang::NUMBER_LY);
				labels.distance->SetText(stringf(format,
					formatarg("distance", dist)));
				labels.distance->Color(1.0f, 0.0f, 0.0f);
				m_jumpLine.SetColor(Color(1.f, 0.f, 0.f, 1.f));
				break;
			default:
				labels.distance->SetText("");
				break;
		}
	}

	else if (path.IsSameSystem(Pi::player->GetHyperspaceDest())) {
		snprintf(format, sizeof(format), "[ %s ]", Lang::IN_TRANSIT);
		labels.distance->SetText(format);
		labels.distance->Color(0.4f, 0.4f, 1.0f);
	}

	else
		labels.distance->SetText("");

	RefCountedPtr<StarSystem> sys = StarSystem::GetCached(path);

	std::string desc;
	if (sys->GetNumStars() == 4) {
		desc = Lang::QUADRUPLE_SYSTEM;
	} else if (sys->GetNumStars() == 3) {
		desc = Lang::TRIPLE_SYSTEM;
	} else if (sys->GetNumStars() == 2) {
		desc = Lang::BINARY_SYSTEM;
	} else {
		desc = sys->rootBody->GetAstroDescription();
	}
	labels.starType->SetText(desc);

	labels.systemName->SetText(sys->GetName());
	labels.shortDesc->SetText(sys->GetShortDescription());

	if (m_infoBoxVisible)
		m_infoBox->ShowAll();
}

void SectorView::DrawSector(int sx, int sy, int sz, const vector3f &playerAbsPos,const matrix4x4f &trans)
{
	m_renderer->SetTransform(trans);
	Sector* ps = GetCached(sx, sy, sz);

	int cz = int(floor(m_pos.z+0.5f));

	if (cz == sz) {
		const Color darkgreen(0.f, 0.2f, 0.f, 1.f);
		const vector3f vts[] = {
			vector3f(0.f, 0.f, 0.f),
			vector3f(0.f, Sector::SIZE, 0.f),
			vector3f(Sector::SIZE, Sector::SIZE, 0.f),
			vector3f(Sector::SIZE, 0.f, 0.f)
		};

		m_renderer->DrawLines(4, vts, darkgreen, LINE_LOOP);
	}

	Uint32 num=0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i, ++num) {
		SystemPath current = SystemPath(sx, sy, sz, num);

		const vector3f sysAbsPos = Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + (*i).p;
		const vector3f toCentreOfView = m_pos*Sector::SIZE - sysAbsPos;

		if (toCentreOfView.Length() > OUTER_RADIUS) continue;

		// don't worry about looking for inhabited systems if they're
		// unexplored (same calculation as in StarSystem.cpp)
		if (isqrt(1 + sx*sx + sy*sy + sz*sz) <= 90) {

			// only do this once we've pretty much stopped moving.
			vector3f diff = vector3f(
					fabs(m_posMovingTo.x - m_pos.x),
					fabs(m_posMovingTo.y - m_pos.y),
					fabs(m_posMovingTo.z - m_pos.z));
			// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
			if( !(*i).IsSetInhabited() && diff.x < 0.001f && diff.y < 0.001f && diff.z < 0.001f ) {
				RefCountedPtr<StarSystem> pSS = StarSystem::GetCached(current);
				if( (!pSS->m_unexplored) && (pSS->m_spaceStations.size()>0) )
				{
					(*i).SetInhabited(true);
				}
				else
				{
					(*i).SetInhabited(false);
				}
				(*i).factionColour = pSS->GetFactionColour();
			}
		}

		matrix4x4f systrans = trans * matrix4x4f::Translation((*i).p.x, (*i).p.y, (*i).p.z);
		m_renderer->SetTransform(systrans);

		glDisable(GL_LIGHTING);

		// draw system "leg"
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glBegin(GL_LINE_STRIP);
			float z = -(*i).p.z;
			if (sz <= cz)
				z = z+abs(cz-sz)*Sector::SIZE;
			else
				z = z-abs(cz-sz)*Sector::SIZE;

			glVertex3f(0, 0, z);
			glColor4f(0.2f, 0.2f, 0.2f, 0.2f);
			glVertex3f(0, 0, z * 0.5);
			glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
			glVertex3f(0, 0, 0);
		glEnd();

		//cross at other end
		glBegin(GL_LINES);
			glVertex3f(-0.1f, -0.1f, z);
			glVertex3f(0.1f, 0.1f, z);
			glVertex3f(-0.1f, 0.1f, z);
			glVertex3f(0.1f, -0.1f, z);
		glEnd();

		if (current == m_selected) {
			m_jumpLine.SetStart(vector3f(0.f, 0.f, 0.f));
			m_jumpLine.SetEnd(playerAbsPos - sysAbsPos);
			m_jumpLine.Draw(m_renderer);
		}

		// draw star blob itself
		systrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
		systrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
		systrans.Scale((StarSystem::starScale[(*i).starType[0]]));
		m_renderer->SetTransform(systrans);

		float *col = StarSystem::starColors[(*i).starType[0]];
		m_disk->SetColor(Color(col[0], col[1], col[2]));
		m_disk->Draw(m_renderer);

		// player location indicator
		if (m_inSystem && current == m_current) {
			glDepthRange(0.2,1.0);
			m_disk->SetColor(Color(0.f, 0.f, 0.8f));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(3.f));
			m_disk->Draw(m_renderer);
		}
		// selected indicator
		if (current == m_selected) {
			glDepthRange(0.1,1.0);
			m_disk->SetColor(Color(0.f, 0.8f, 0.f));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}
		// hyperspace target indicator (if different from selection)
		if (current == m_hyperspaceTarget && m_hyperspaceTarget != m_selected && (!m_inSystem || m_hyperspaceTarget != m_current)) {
			glDepthRange(0.1,1.0);
			m_disk->SetColor(Color(0.3f));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}

		glDepthRange(0,1);

		Color labelColor(0.8f,0.8f,0.8f,0.5f);
		if ((*i).IsSetInhabited() && (*i).IsInhabited()) {
			labelColor = (*i).factionColour;
			labelColor.a = 0.5f;
		}

		if (m_inSystem) {
			float dist = Sector::DistanceBetween( ps, num, GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ), m_current.systemIndex);
			if (dist <= m_playerHyperspaceRange)
				labelColor.a = 1.0f;
		}

		PutClickableLabel((*i).name, labelColor, current);
	}
}

void SectorView::OnSwitchTo() {
	if (!m_onKeyPressConnection.connected())
		m_onKeyPressConnection =
			Pi::onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnKeyPressed));

	Update();

	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);
}

void SectorView::OnKeyPressed(SDL_keysym *keysym)
{
	if (Pi::GetView() != this) {
		m_onKeyPressConnection.disconnect();
		return;
	}

	// XXX ugly hack checking for Lua console here
	if (Pi::IsConsoleActive())
		return;

	// ignore keypresses if they're typing
	if (m_searchBox->IsFocused()) {
		// but if they press enter then we want future keys
		if (keysym->sym == SDLK_KP_ENTER || keysym->sym == SDLK_RETURN)
			m_searchBox->Unfocus();
		return;
	}

	// '/' focuses the search box
	if (keysym->sym == SDLK_KP_DIVIDE || keysym->sym == SDLK_SLASH) {
		m_searchBox->SetText("");
		m_searchBox->GrabFocus();
		return;
	}

	// space "locks" (or unlocks) the hyperspace target to the selected system
	if (keysym->sym == SDLK_SPACE) {
		if ((m_matchTargetToSelection || m_hyperspaceTarget != m_selected) && !m_selected.IsSameSystem(m_current))
			SetHyperspaceTarget(m_selected);
		else
			ResetHyperspaceTarget();
		return;
	}

	// toggle the info box
	if (keysym->sym == SDLK_TAB) {
		m_infoBoxVisible = !m_infoBoxVisible;
		if (m_infoBoxVisible)
			m_infoBox->ShowAll();
		else
			m_infoBox->HideAll();
		return;
	}

	// toggle selection mode
		if (keysym->sym == SDLK_KP_ENTER || keysym->sym == SDLK_RETURN) {
		m_selectionFollowsMovement = !m_selectionFollowsMovement;
		if (m_selectionFollowsMovement)
			Pi::cpan->MsgLog()->Message("", Lang::ENABLED_AUTOMATIC_SYSTEM_SELECTION);
		else
			Pi::cpan->MsgLog()->Message("", Lang::DISABLED_AUTOMATIC_SYSTEM_SELECTION);
		return;
	}

	// fast move selection to current player system or hyperspace target
	if (keysym->sym == SDLK_c || keysym->sym == SDLK_g || keysym->sym == SDLK_h) {
		if (keysym->sym == SDLK_c)
			GotoSystem(m_current);
		else if (keysym->sym == SDLK_g)
			GotoSystem(m_selected);
		else
			GotoSystem(m_hyperspaceTarget);

		if (Pi::KeyState(SDLK_LSHIFT) || Pi::KeyState(SDLK_RSHIFT)) {
			while (m_rotZ < -180.0f) m_rotZ += 360.0f;
			while (m_rotZ > 180.0f)  m_rotZ -= 360.0f;
			m_rotXMovingTo = m_rotXDefault;
			m_rotZMovingTo = m_rotZDefault;
			m_zoomMovingTo = m_zoomDefault;
		}
		return;
	}

	// reset rotation and zoom
	if (keysym->sym == SDLK_r) {
		while (m_rotZ < -180.0f) m_rotZ += 360.0f;
		while (m_rotZ > 180.0f)  m_rotZ -= 360.0f;
		m_rotXMovingTo = m_rotXDefault;
		m_rotZMovingTo = m_rotZDefault;
		m_zoomMovingTo = m_zoomDefault;
		return;
	}
}

void SectorView::Update()
{
	SystemPath last_current = m_current;
	bool last_inSystem = m_inSystem;

	if (Pi::game->IsNormalSpace()) {
		m_inSystem = true;
		m_current = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	}
	else {
		m_inSystem = false;
		m_current = Pi::player->GetHyperspaceDest();
	}

	if (last_inSystem != m_inSystem || last_current != m_current) {
		UpdateSystemLabels(m_currentSystemLabels, m_current);
		UpdateSystemLabels(m_selectedSystemLabels, m_selected);
		UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);
	}

	const float frameTime = Pi::GetFrameTime();

	matrix4x4f rot = matrix4x4f::Identity();
	rot.RotateX(DEG2RAD(-m_rotX));
	rot.RotateZ(DEG2RAD(-m_rotZ));

	// don't check raw keypresses if the search box is active
	// XXX ugly hack checking for Lua console here
	if (!m_searchBox->IsFocused() && !Pi::IsConsoleActive()) {
		const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
		float move = moveSpeed*frameTime;
		if (Pi::KeyState(SDLK_LEFT) || Pi::KeyState(SDLK_RIGHT))
			m_posMovingTo += vector3f(Pi::KeyState(SDLK_LEFT) ? -move : move, 0,0) * rot;
		if (Pi::KeyState(SDLK_UP) || Pi::KeyState(SDLK_DOWN))
			m_posMovingTo += vector3f(0, Pi::KeyState(SDLK_DOWN) ? -move : move, 0) * rot;
		if (Pi::KeyState(SDLK_PAGEUP) || Pi::KeyState(SDLK_PAGEDOWN))
			m_posMovingTo += vector3f(0,0, Pi::KeyState(SDLK_PAGEUP) ? -move : move) * rot;

		if (Pi::KeyState(SDLK_EQUALS)) m_zoomMovingTo -= move;
		if (Pi::KeyState(SDLK_MINUS)) m_zoomMovingTo += move;
		if (m_zoomInButton->IsPressed()) m_zoomMovingTo -= move;
		if (m_zoomOutButton->IsPressed()) m_zoomMovingTo += move;
		m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, 5.0f);

		if (Pi::KeyState(SDLK_a) || Pi::KeyState(SDLK_d))
			m_rotZMovingTo += (Pi::KeyState(SDLK_a) ? -0.5f : 0.5f) * moveSpeed;
		if (Pi::KeyState(SDLK_w) || Pi::KeyState(SDLK_s))
			m_rotXMovingTo += (Pi::KeyState(SDLK_w) ? -0.5f : 0.5f) * moveSpeed;
	}

	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
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

		float diffZoom = m_zoomMovingTo - m_zoom;
		float travelZoom = diffZoom * ZOOM_SPEED*frameTime;
		if (fabs(travelZoom) > fabs(diffZoom)) m_zoom = m_zoomMovingTo;
		else m_zoom = m_zoom + travelZoom;
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

	m_playerHyperspaceRange = Pi::player->GetStats().hyperspace_range;
}

void SectorView::ShowAll()
{
	View::ShowAll();
	if (!m_infoBoxVisible)
		m_infoBox->HideAll();
}

void SectorView::MouseButtonDown(int button, int x, int y)
{
	if (this == Pi::GetView()) {
		if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN))
			m_zoomMovingTo += ZOOM_SPEED * WHEEL_SENSITIVITY * Pi::GetMoveSpeedShiftModifier();
		else if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP))
			m_zoomMovingTo -= ZOOM_SPEED * WHEEL_SENSITIVITY * Pi::GetMoveSpeedShiftModifier();
	}
}

Sector* SectorView::GetCached(int sectorX, int sectorY, int sectorZ)
{
	const SystemPath loc(sectorX, sectorY, sectorZ);

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
	const int xmin = int(floorf(m_pos.x))-DRAW_RAD;
	const int xmax = int(ceilf(m_pos.x))+DRAW_RAD;
	const int ymin = int(floorf(m_pos.y))-DRAW_RAD;
	const int ymax = int(ceilf(m_pos.y))+DRAW_RAD;
	const int zmin = int(floorf(m_pos.z))-DRAW_RAD;
	const int zmax = int(ceilf(m_pos.z))+DRAW_RAD;

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
