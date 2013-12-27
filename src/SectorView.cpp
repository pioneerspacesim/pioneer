// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Factions.h"
#include "GalacticView.h"
#include "Game.h"
#include "Lang.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "SectorCache.h"
#include "Serializer.h"
#include "ShipCpanel.h"
#include "StringF.h"
#include "SystemInfoView.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "gui/Gui.h"
#include <algorithm>
#include <sstream>
#include <SDL_stdinc.h>

using namespace Graphics;

static const int DRAW_RAD = 5;
#define INNER_RADIUS (Sector::SIZE*1.5f)
#define OUTER_RADIUS (Sector::SIZE*float(DRAW_RAD))
static const float FAR_THRESHOLD = 7.5f;
static const float FAR_LIMIT     = 36.f;
static const float FAR_MAX       = 46.f;

enum DetailSelection {
	DETAILBOX_NONE    = 0
,	DETAILBOX_INFO    = 1
,	DETAILBOX_FACTION = 2
};

static const float ZOOM_SPEED = 15;
static const float WHEEL_SENSITIVITY = .03f;		// Should be a variable in user settings.

SectorView::SectorView()
{
	InitDefaults();

	m_rotX = m_rotXMovingTo = m_rotXDefault;
	m_rotZ = m_rotZMovingTo = m_rotZDefault;
	m_zoom = m_zoomMovingTo = m_zoomDefault;

	// XXX I have no idea if this is correct,
	// I just copied it from the one other place m_zoomClamped is set
	m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);

	m_inSystem = true;

	m_current = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	assert(!m_current.IsSectorPath());
	m_current = m_current.SystemOnly();

	m_selected = m_hyperspaceTarget = m_current;

	GotoSystem(m_current);
	m_pos = m_posMovingTo;
	SectorCache::SetPosition(m_pos);

	m_matchTargetToSelection   = true;
	m_selectionFollowsMovement = true;
	m_detailBoxVisible         = DETAILBOX_INFO;
	m_toggledFaction           = false;

	InitObject();
}

SectorView::SectorView(Serializer::Reader &rd)
{
	InitDefaults();

	m_pos.x = m_posMovingTo.x = rd.Float();
	m_pos.y = m_posMovingTo.y = rd.Float();
	m_pos.z = m_posMovingTo.z = rd.Float();
	SectorCache::SetPosition(m_pos);
	m_rotX = m_rotXMovingTo = rd.Float();
	m_rotZ = m_rotZMovingTo = rd.Float();
	m_zoom = m_zoomMovingTo = rd.Float();
	// XXX I have no idea if this is correct,
	// I just copied it from the one other place m_zoomClamped is set
	m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);
	m_inSystem = rd.Bool();
	m_current = SystemPath::Unserialize(rd);
	m_selected = SystemPath::Unserialize(rd);
	m_hyperspaceTarget = SystemPath::Unserialize(rd);
	m_matchTargetToSelection = rd.Bool();
	m_selectionFollowsMovement = rd.Bool();
	m_detailBoxVisible = rd.Byte();

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

	m_secPosFar = vector3f(INT_MAX, INT_MAX, INT_MAX);
	m_radiusFar = 0;
}

void SectorView::InitObject()
{
	SetTransparency(true);

	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_secLineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));

	Gui::Screen::PushFont("OverlayFont");
	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(178,178,178,191));
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

	m_zoomLevelLabel = (new Gui::Label(""))->Color(69, 219, 235);
	Add(m_zoomLevelLabel, 640, 5);

	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	m_zoomOutButton->SetRenderDimensions(30, 22);
	Add(m_zoomOutButton, 732, 5);

	Add(new Gui::Label(Lang::SEARCH), 650, 500);
	m_searchBox = new Gui::TextEntry();
	m_searchBox->onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnSearchBoxKeyPress));
	Add(m_searchBox, 700, 500);

	m_disk.reset(new Graphics::Drawables::Disk(Pi::renderer, Color::WHITE, 0.2f));

	m_infoBox = new Gui::VBox();
	m_infoBox->SetTransparency(false);
	m_infoBox->SetBgColor(Color(16,16,32,128));
	m_infoBox->SetSpacing(10.0f);
	Add(m_infoBox, 5, 5);

	// 1. holds info about current, selected, targeted systems
	Gui::VBox *locationsBox = new Gui::VBox();
	locationsBox->SetSpacing(5.f);
	// 1.1 current system
	Gui::VBox *systemBox = new Gui::VBox();
	Gui::HBox *hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoCurrentSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label(Lang::CURRENT_SYSTEM))->Color(255, 255, 255));
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_currentSystemLabels.systemName = (new Gui::Label(""))->Color(255, 255, 0);
	m_currentSystemLabels.distance = (new Gui::Label(""))->Color(255, 0, 0);
	hbox->PackEnd(m_currentSystemLabels.systemName);
	hbox->PackEnd(m_currentSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_currentSystemLabels.starType = (new Gui::Label(""))->Color(255, 0, 255);
	m_currentSystemLabels.shortDesc = (new Gui::Label(""))->Color(255, 0, 255);
	systemBox->PackEnd(m_currentSystemLabels.starType);
	systemBox->PackEnd(m_currentSystemLabels.shortDesc);
	locationsBox->PackEnd(systemBox);
	// 1.2 selected system
	systemBox = new Gui::VBox();
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoSelectedSystem));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label(Lang::SELECTED_SYSTEM))->Color(255, 255, 255));
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_selectedSystemLabels.systemName = (new Gui::Label(""))->Color(255, 255, 0);
	m_selectedSystemLabels.distance = (new Gui::Label(""))->Color(255, 0, 0);
	hbox->PackEnd(m_selectedSystemLabels.systemName);
	hbox->PackEnd(m_selectedSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_selectedSystemLabels.starType = (new Gui::Label(""))->Color(255, 0, 255);
	m_selectedSystemLabels.shortDesc = (new Gui::Label(""))->Color(255, 0, 255);
	systemBox->PackEnd(m_selectedSystemLabels.starType);
	systemBox->PackEnd(m_selectedSystemLabels.shortDesc);
	locationsBox->PackEnd(systemBox);
	// 1.3 targeted system
	systemBox = new Gui::VBox();
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &SectorView::GotoHyperspaceTarget));
	hbox->PackEnd(b);
	hbox->PackEnd((new Gui::Label(Lang::HYPERSPACE_TARGET))->Color(255, 255, 255));
    m_hyperspaceLockLabel = (new Gui::Label(""))->Color(255, 255, 255);
    hbox->PackEnd(m_hyperspaceLockLabel);
	systemBox->PackEnd(hbox);
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_targetSystemLabels.systemName = (new Gui::Label(""))->Color(255, 255, 0);
	m_targetSystemLabels.distance = (new Gui::Label(""))->Color(255, 0, 0);
	hbox->PackEnd(m_targetSystemLabels.systemName);
	hbox->PackEnd(m_targetSystemLabels.distance);
	systemBox->PackEnd(hbox);
	m_targetSystemLabels.starType = (new Gui::Label(""))->Color(255, 0, 255);
	m_targetSystemLabels.shortDesc = (new Gui::Label(""))->Color(255, 0, 255);
	systemBox->PackEnd(m_targetSystemLabels.starType);
	systemBox->PackEnd(m_targetSystemLabels.shortDesc);
	locationsBox->PackEnd(systemBox);
	m_infoBox->PackEnd(locationsBox);

	// 2. holds options for displaying systems
	Gui::VBox *filterBox = new Gui::VBox();
	// 2.1 Draw vertical lines
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_drawSystemLegButton = (new Gui::ToggleButton());
	m_drawSystemLegButton->SetPressed(false); // TODO: replace with var
	hbox->PackEnd(m_drawSystemLegButton);
	Gui::Label *label = (new Gui::Label(Lang::DRAW_VERTICAL_LINES))->Color(255, 255, 255);
	hbox->PackEnd(label);
	filterBox->PackEnd(hbox);
	// 2.2 Draw planet labels
	hbox = new Gui::HBox();
	hbox->SetSpacing(5.0f);
	m_drawOutRangeLabelButton = new Gui::ToggleButton();
	m_drawOutRangeLabelButton->SetPressed(false); // TODO: replace with var
	hbox->PackEnd(m_drawOutRangeLabelButton);
	label = (new Gui::Label(Lang::DRAW_OUT_RANGE_LABELS))->Color(255, 255, 255);
	hbox->PackEnd(label);
	filterBox->PackEnd(hbox);

	m_infoBox->PackEnd(filterBox);

	m_onMouseWheelCon =
		Pi::onMouseWheel.connect(sigc::mem_fun(this, &SectorView::MouseWheel));

	UpdateSystemLabels(m_currentSystemLabels, m_current);
	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);

	UpdateHyperspaceLockLabel();

	m_factionBox = new Gui::VBox();
	m_factionBox->SetTransparency(false);
	m_factionBox->SetBgColor(Color(16,16,32,128));
	m_factionBox->SetSpacing(5.0f);
	m_factionBox->HideAll();
	Add(m_factionBox, 5, 5);
}

SectorView::~SectorView()
{
	m_onMouseWheelCon.disconnect();
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
	wr.Byte(m_detailBoxVisible);
}

void SectorView::OnSearchBoxKeyPress(const SDL_Keysym *keysym)
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

	for (SectorCache::SectorCacheMapConstIterator i = SectorCache::GetCache().begin(); i != SectorCache::GetCache().end(); ++i)

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

#define FFRAC(_x)	((_x)-floor(_x))

void SectorView::Draw3D()
{
	PROFILE_SCOPED()

	m_lineVerts->Clear();
	m_secLineVerts->Clear();
	m_clickableLabels->Clear();

	if (m_zoomClamped <= FAR_THRESHOLD) m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 300.f);
	else                                m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 600.f);

	matrix4x4f modelview = matrix4x4f::Identity();
	m_renderer->ClearScreen();

	m_sectorLabel->SetText(stringf(Lang::SECTOR_X_Y_Z,
		formatarg("x", int(floorf(m_pos.x))),
		formatarg("y", int(floorf(m_pos.y))),
		formatarg("z", int(floorf(m_pos.z)))));

	m_zoomLevelLabel->SetText(stringf(Lang::NUMBER_LY, formatarg("distance", ((m_zoomClamped/FAR_THRESHOLD )*(OUTER_RADIUS)) + 0.5 * Sector::SIZE)));

	if (m_inSystem) {
		vector3f dv = vector3f(floorf(m_pos.x)-m_current.sectorX, floorf(m_pos.y)-m_current.sectorY, floorf(m_pos.z)-m_current.sectorZ) * Sector::SIZE;
		m_distanceLabel->SetText(stringf(Lang::DISTANCE_LY, formatarg("distance", dv.Length())));
	}
	else {
		m_distanceLabel->SetText("");
	}

	Graphics::Renderer::MatrixTicket ticket(m_renderer, Graphics::MatrixMode::MODELVIEW);

	// units are lightyears, my friend
	modelview.Translate(0.f, 0.f, -10.f-10.f*m_zoom);    // not zoomClamped, let us zoom out a bit beyond what we're drawing
	modelview.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	modelview.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);
	modelview.Translate(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);
	m_renderer->SetTransform(modelview);

	m_renderer->SetBlendMode(BLEND_ALPHA);

	if (m_zoomClamped <= FAR_THRESHOLD)
		DrawNearSectors(modelview);
	else
		DrawFarSectors(modelview);

	//draw sector legs in one go
	m_renderer->SetTransform(matrix4x4f::Identity());
	if (m_lineVerts->GetNumVerts() > 2)
		m_renderer->DrawLines(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);

	if (m_secLineVerts->GetNumVerts() > 2)
		m_renderer->DrawLines(m_secLineVerts->GetNumVerts(), &m_secLineVerts->position[0], &m_secLineVerts->diffuse[0]);

	UpdateFactionToggles();

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

	// for performance don't animate the travel if we're Far Zoomed
	if (m_zoomClamped > FAR_THRESHOLD) {
		m_pos = m_posMovingTo;
		SectorCache::SetPosition(m_pos);
	}
}

void SectorView::GotoSystem(const SystemPath &path)
{
	Sector* ps = SectorCache::GetCached(path.sectorX, path.sectorY, path.sectorZ);
	const vector3f &p = ps->m_systems[path.systemIndex].p;
	m_posMovingTo.x = path.sectorX + p.x/Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y/Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z/Sector::SIZE;

	// for performance don't animate the travel if we're Far Zoomed
	if (m_zoomClamped > FAR_THRESHOLD) {
		m_pos = m_posMovingTo;
		SectorCache::SetPosition(m_pos);
	}
}

void SectorView::SetSelectedSystem(const SystemPath &path)
{
    m_selected = path;

	if (m_matchTargetToSelection && m_selected != m_current) {
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

void SectorView::PutSystemLabels(Sector *sec, const vector3f &origin, int drawRadius)
{
	PROFILE_SCOPED()
	Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator sys = sec->m_systems.begin(); sys !=sec->m_systems.end(); ++sys, ++sysIdx) {
		// skip the system if it doesn't fall within the sphere we're viewing.
		if ((m_pos*Sector::SIZE - (*sys).FullPosition()).Length() > drawRadius) continue;

		// if the system is the current system or target we can't skip it
		bool can_skip = !sys->IsSameSystem(m_selected)
						&& !sys->IsSameSystem(m_hyperspaceTarget)
						&& !sys->IsSameSystem(m_current);

		// skip the system if it belongs to a Faction we've toggled off and we can skip it
		if (m_hiddenFactions.find((*sys).faction) != m_hiddenFactions.end() && can_skip) continue;

		// determine if system in hyperjump range or not
		Sector *playerSec = SectorCache::GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
		float dist = Sector::DistanceBetween(sec, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// place the label
		vector3d systemPos = vector3d((*sys).FullPosition() - origin);
		vector3d screenPos;
		if (Gui::Screen::Project(systemPos, screenPos)) {
			// reject back-projected labels
			if(screenPos.z > 1.0f)
				continue;

			// work out the colour
			Color labelColor = (*sys).faction->AdjustedColour((*sys).population, inRange);

			// get a system path to pass to the event handler when the label is licked
			SystemPath sysPath = SystemPath((*sys).sx, (*sys).sy, (*sys).sz, sysIdx);

			// label text
			std::string text = "";
			if(inRange || m_drawOutRangeLabelButton->GetPressed() || !can_skip)
				text = (*sys).name;

			// setup the label;
			m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), sysPath), screenPos.x, screenPos.y, labelColor);
		}
	}
}

void SectorView::PutFactionLabels(const vector3f &origin)
{
	PROFILE_SCOPED()
	glDepthRange(0,1);
	Gui::Screen::EnterOrtho();
	for (std::set<Faction*>::iterator it = m_visibleFactions.begin(); it != m_visibleFactions.end(); ++it) {
		if ((*it)->hasHomeworld && m_hiddenFactions.find((*it)) == m_hiddenFactions.end()) {

			Sector::System sys = SectorCache::GetCached((*it)->homeworld)->m_systems[(*it)->homeworld.systemIndex];
			if ((m_pos*Sector::SIZE - sys.FullPosition()).Length() > (m_zoomClamped/FAR_THRESHOLD )*OUTER_RADIUS) continue;

			vector3d pos;
			if (Gui::Screen::Project(vector3d(sys.FullPosition() - origin), pos)) {

				std::string labelText    = sys.name + "\n" + (*it)->name;
				Color       labelColor  = (*it)->colour;
				float       labelHeight = 0;
				float       labelWidth  = 0;

				Gui::Screen::MeasureString(labelText, labelWidth, labelHeight);

				if (!m_material) m_material.Reset(m_renderer->CreateMaterial(Graphics::MaterialDescriptor()));

				{
					Graphics::VertexArray va(Graphics::ATTRIB_POSITION);
					va.Add(vector3f(pos.x - 5.f,              pos.y - 5.f,               0));
					va.Add(vector3f(pos.x - 5.f,              pos.y - 5.f + labelHeight, 0));
					va.Add(vector3f(pos.x + labelWidth + 5.f, pos.y - 5.f,               0));
					va.Add(vector3f(pos.x + labelWidth + 5.f, pos.y - 5.f + labelHeight, 0));
					m_material->diffuse = Color(13, 13, 31, 166);
					m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
				}

				{
					Graphics::VertexArray va(Graphics::ATTRIB_POSITION);
					va.Add(vector3f(pos.x - 8.f, pos.y,       0));
					va.Add(vector3f(pos.x      , pos.y + 8.f, 0));
					va.Add(vector3f(pos.x,       pos.y - 8.f, 0));
					va.Add(vector3f(pos.x + 8.f, pos.y,       0));
					m_material->diffuse = labelColor;
					m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
				}

				if (labelColor.GetLuminance() > 191) labelColor.a = 204;    // luminance is sometimes a bit overly
				m_clickableLabels->Add(labelText, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), (*it)->homeworld), pos.x, pos.y, labelColor);
			}
		}
	}
	Gui::Screen::LeaveOrtho();
}

void SectorView::UpdateSystemLabels(SystemLabels &labels, const SystemPath &path)
{
	PROFILE_SCOPED()
	Sector *sec = SectorCache::GetCached(path.sectorX, path.sectorY, path.sectorZ);
	Sector *playerSec = SectorCache::GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);

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
				labels.distance->Color(0, 255, 51);
				m_jumpLine.SetColor(Color(0, 255, 51, 255));
				break;
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				snprintf(format, sizeof(format), "[ %s | %s ]", Lang::NUMBER_LY, Lang::NUMBER_TONNES);
				labels.distance->SetText(stringf(format,
					formatarg("distance", dist), formatarg("mass", fuelRequired)));
				labels.distance->Color(255, 255, 0);
				m_jumpLine.SetColor(Color::YELLOW);
				break;
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				snprintf(format, sizeof(format), "[ %s ]", Lang::NUMBER_LY);
				labels.distance->SetText(stringf(format,
					formatarg("distance", dist)));
				labels.distance->Color(255, 0, 0);
				m_jumpLine.SetColor(Color::RED);
				break;
			default:
				labels.distance->SetText("");
				break;
		}
	}

	else if (path.IsSameSystem(Pi::player->GetHyperspaceDest())) {
		snprintf(format, sizeof(format), "[ %s ]", Lang::IN_TRANSIT);
		labels.distance->SetText(format);
		labels.distance->Color(102, 102, 255);
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

	if (m_detailBoxVisible == DETAILBOX_INFO) m_infoBox->ShowAll();
}

void SectorView::OnToggleFaction(Gui::ToggleButton* button, bool pressed, Faction* faction)
{
	// hide or show the faction's systems depending on whether the button is pressed
	if (pressed) m_hiddenFactions.erase(faction);
	else         m_hiddenFactions.insert(faction);

	m_toggledFaction = true;
}

void SectorView::UpdateFactionToggles()
{
	PROFILE_SCOPED()
	// make sure we have enough row in the ui
	while (m_visibleFactionLabels.size() < m_visibleFactions.size()) {
		Gui::HBox*         row    = new Gui::HBox();
		Gui::ToggleButton* toggle = new Gui::ToggleButton();
		Gui::Label*        label  = new Gui::Label("");

		toggle->SetToolTip("");
		label ->SetToolTip("");

		m_visibleFactionToggles.push_back(toggle);
		m_visibleFactionLabels.push_back(label);
		m_visibleFactionRows.push_back(row);

		row->SetSpacing(5.0f);
		row->PackEnd(toggle);
		row->PackEnd(label);
		m_factionBox->PackEnd(row);
	}

	// set up the faction labels, and the toggle buttons
	Uint32 rowIdx = 0;
	for (std::set<Faction*>::iterator it = m_visibleFactions.begin(); it != m_visibleFactions.end(); ++it, ++rowIdx) {
		m_visibleFactionLabels [rowIdx]->SetText((*it)->name);
		m_visibleFactionLabels [rowIdx]->Color((*it)->colour);
		m_visibleFactionToggles[rowIdx]->onChange.clear();
		m_visibleFactionToggles[rowIdx]->SetPressed(m_hiddenFactions.find((*it)) == m_hiddenFactions.end());
		m_visibleFactionToggles[rowIdx]->onChange.connect(sigc::bind(sigc::mem_fun(this, &SectorView::OnToggleFaction),*it));
		m_visibleFactionRows   [rowIdx]->ShowAll();
	}

	// hide any rows, and disconnect any toggle event handler, that we're not using
	for (; rowIdx < m_visibleFactionLabels.size(); rowIdx++) {
		m_visibleFactionToggles[rowIdx]->onChange.clear();
		m_visibleFactionRows   [rowIdx]->Hide();
	}

	if  (m_detailBoxVisible == DETAILBOX_FACTION) m_factionBox->Show();
	else                                          m_factionBox->HideAll();
}

void SectorView::DrawNearSectors(const matrix4x4f& modelview)
{
	PROFILE_SCOPED()
	m_visibleFactions.clear();

	const Sector *playerSec = SectorCache::GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
	const vector3f playerPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) + playerSec->m_systems[m_current.systemIndex].p;

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				DrawNearSector(int(floorf(m_pos.x))+sx, int(floorf(m_pos.y))+sy, int(floorf(m_pos.z))+sz, playerPos,
					modelview * matrix4x4f::Translation(Sector::SIZE*sx, Sector::SIZE*sy, Sector::SIZE*sz));
			}
		}
	}

	// ...then switch and do all the labels
	const vector3f secOrigin = vector3f(int(floorf(m_pos.x)), int(floorf(m_pos.y)), int(floorf(m_pos.z)));
	
	m_renderer->SetTransform(modelview);
	glDepthRange(0,1);
	Gui::Screen::EnterOrtho();
	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				PutSystemLabels(SectorCache::GetCached(sx + secOrigin.x, sy + secOrigin.y, sz + secOrigin.z), Sector::SIZE * secOrigin, Sector::SIZE * DRAW_RAD);
			}
		}
	}
	Gui::Screen::LeaveOrtho();
}

void SectorView::DrawNearSector(const int sx, const int sy, const int sz, const vector3f &playerAbsPos,const matrix4x4f &trans)
{
	PROFILE_SCOPED()
	m_renderer->SetTransform(trans);
	Sector* ps = SectorCache::GetCached(sx, sy, sz);

	int cz = int(floor(m_pos.z+0.5f));

	if (cz == sz) {
		const Color darkgreen(0, 51, 0, 255);
		const vector3f vts[] = {
			trans * vector3f(0.f, 0.f, 0.f),
			trans * vector3f(0.f, Sector::SIZE, 0.f),
			trans * vector3f(Sector::SIZE, Sector::SIZE, 0.f),
			trans * vector3f(Sector::SIZE, 0.f, 0.f)
		};

		m_secLineVerts->Add(vts[0], darkgreen);	// line segment 1
		m_secLineVerts->Add(vts[1], darkgreen);
		m_secLineVerts->Add(vts[1], darkgreen);	// line segment 2
		m_secLineVerts->Add(vts[2], darkgreen);
		m_secLineVerts->Add(vts[2], darkgreen);	// line segment 3
		m_secLineVerts->Add(vts[3], darkgreen);
		m_secLineVerts->Add(vts[3], darkgreen);	// line segment 4
		m_secLineVerts->Add(vts[0], darkgreen);
	}

	Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i, ++sysIdx) {
		// calculate where the system is in relation the centre of the view...
		const vector3f sysAbsPos = Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + (*i).p;
		const vector3f toCentreOfView = m_pos*Sector::SIZE - sysAbsPos;

		// ...and skip the system if it doesn't fall within the sphere we're viewing.
		if (toCentreOfView.Length() > OUTER_RADIUS) continue;

		const bool bIsCurrentSystem = i->IsSameSystem(m_current);

		// if the system is the current system or target we can't skip it
		bool can_skip = !i->IsSameSystem(m_selected)
						&& !i->IsSameSystem(m_hyperspaceTarget)
						&& !bIsCurrentSystem;

		// if the system belongs to a faction we've chosen to temporarily hide
		// then skip it if we can
		m_visibleFactions.insert(i->faction);
		if (m_hiddenFactions.find(i->faction) != m_hiddenFactions.end() && can_skip) continue;

		// determine if system in hyperjump range or not
		Sector *playerSec = SectorCache::GetCached(m_current.sectorX, m_current.sectorY, m_current.sectorZ);
		float dist = Sector::DistanceBetween(ps, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// don't worry about looking for inhabited systems if they're
		// unexplored (same calculation as in StarSystem.cpp) or we've
		// already retrieved their population.
		if ((*i).population < 0 && isqrt(1 + sx*sx + sy*sy + sz*sz) <= 90) {

			// only do this once we've pretty much stopped moving.
			vector3f diff = vector3f(
					fabs(m_posMovingTo.x - m_pos.x),
					fabs(m_posMovingTo.y - m_pos.y),
					fabs(m_posMovingTo.z - m_pos.z));

			// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
			if( (diff.x < 0.001f && diff.y < 0.001f && diff.z < 0.001f) ) {
				SystemPath current = SystemPath(sx, sy, sz, sysIdx);
				RefCountedPtr<StarSystem> pSS = StarSystem::GetCached(current);
				(*i).population = pSS->GetTotalPop();
			}

		}

		matrix4x4f systrans = trans * matrix4x4f::Translation((*i).p.x, (*i).p.y, (*i).p.z);
		m_renderer->SetTransform(systrans);

		// for out-of-range systems draw leg only if we draw label
		if ((m_drawSystemLegButton->GetPressed() && (inRange || m_drawOutRangeLabelButton->GetPressed())) || !can_skip) {

			const Color light(128);
			const Color dark(51);

			// draw system "leg"
			float z = -(*i).p.z;
			if (sz <= cz)
				z = z+abs(cz-sz)*Sector::SIZE;
			else
				z = z-abs(cz-sz)*Sector::SIZE;
			m_lineVerts->Add(systrans * vector3f(0.f, 0.f, z), light);
			m_lineVerts->Add(systrans * vector3f(0.f, 0.f, z * 0.5f), dark);
			m_lineVerts->Add(systrans * vector3f(0.f, 0.f, z * 0.5f), dark);
			m_lineVerts->Add(systrans * vector3f(0.f, 0.f, 0.f), light);

			//cross at other end
			m_lineVerts->Add(systrans * vector3f(-0.1f, -0.1f, z), light);
			m_lineVerts->Add(systrans * vector3f(0.1f, 0.1f, z), light);
			m_lineVerts->Add(systrans * vector3f(-0.1f, 0.1f, z), light);
			m_lineVerts->Add(systrans * vector3f(0.1f, -0.1f, z), light);
		}

		if (i->IsSameSystem(m_selected)) {
			m_jumpLine.SetStart(vector3f(0.f, 0.f, 0.f));
			m_jumpLine.SetEnd(playerAbsPos - sysAbsPos);
			m_jumpLine.Draw(m_renderer);
		}

		// draw star blob itself
		systrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
		systrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
		systrans.Scale((StarSystem::starScale[(*i).starType[0]]));
		m_renderer->SetTransform(systrans);

		Uint8 *col = StarSystem::starColors[(*i).starType[0]];
		m_disk->SetColor(Color(col[0], col[1], col[2]));
		m_disk->Draw(m_renderer);

		// player location indicator
		if (m_inSystem && bIsCurrentSystem) {
			glDepthRange(0.2,1.0);
			m_disk->SetColor(Color(0, 0, 204));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(3.f));
			m_disk->Draw(m_renderer);
		}
		// selected indicator
		if (bIsCurrentSystem) {
			glDepthRange(0.1,1.0);
			m_disk->SetColor(Color(0, 204, 0));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}
		// hyperspace target indicator (if different from selection)
		if (i->IsSameSystem(m_hyperspaceTarget) && m_hyperspaceTarget != m_selected && (!m_inSystem || m_hyperspaceTarget != m_current)) {
			glDepthRange(0.1,1.0);
			m_disk->SetColor(Color(77));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}
		if(bIsCurrentSystem && m_jumpSphere && m_playerHyperspaceRange>0.0f) {
			// not sure I should do these here on when applying the material?
			m_renderer->SetDepthWrite(false);
			m_renderer->SetDepthTest(false);
			m_renderer->SetBlendMode(BLEND_ALPHA);
			
			const matrix4x4f sphTrans = trans * matrix4x4f::Translation((*i).p.x, (*i).p.y, (*i).p.z);
			m_renderer->SetTransform(sphTrans * matrix4x4f::ScaleMatrix(m_playerHyperspaceRange));
			m_jumpSphere->Draw(m_renderer);
			m_jumpDisk->Draw(m_renderer);

			m_renderer->SetDepthWrite(true);
			m_renderer->SetBlendMode(BLEND_SOLID);
			m_renderer->SetDepthTest(true);
		}
	}
}

void SectorView::DrawFarSectors(const matrix4x4f& modelview)
{
	PROFILE_SCOPED()
	int buildRadius = ceilf((m_zoomClamped/FAR_THRESHOLD) * 3);
	if (buildRadius <= DRAW_RAD) buildRadius = DRAW_RAD;

	const vector3f secOrigin = vector3f(int(floorf(m_pos.x)), int(floorf(m_pos.y)), int(floorf(m_pos.z)));

	// build vertex and colour arrays for all the stars we want to see, if we don't already have them
	if (m_toggledFaction || buildRadius != m_radiusFar || !secOrigin.ExactlyEqual(m_secPosFar)) {
		m_farstars       .clear();
		m_farstarsColor  .clear();
		m_visibleFactions.clear();

		for (int sx = secOrigin.x-buildRadius; sx <= secOrigin.x+buildRadius; sx++) {
			for (int sy = secOrigin.y-buildRadius; sy <= secOrigin.y+buildRadius; sy++) {
				for (int sz = secOrigin.z-buildRadius; sz <= secOrigin.z+buildRadius; sz++) {
						if ((vector3f(sx,sy,sz) - secOrigin).Length() <= buildRadius){
							BuildFarSector(SectorCache::GetCached(sx, sy, sz), Sector::SIZE * secOrigin, m_farstars, m_farstarsColor);
						}
					}
				}
			}

		m_secPosFar      = secOrigin;
		m_radiusFar      = buildRadius;
		m_toggledFaction = false;
	}

	// always draw the stars, slightly altering their size for different different resolutions, so they still look okay
	if (m_farstars.size() > 0)
		m_renderer->DrawPoints(m_farstars.size(), &m_farstars[0], &m_farstarsColor[0], 1.f + (Graphics::GetScreenHeight() / 720.f));

	// also add labels for any faction homeworlds among the systems we've drawn
	PutFactionLabels(Sector::SIZE * secOrigin);
}

void SectorView::BuildFarSector(Sector* sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors)
{
	PROFILE_SCOPED()
	Color starColor;
	for (std::vector<Sector::System>::iterator i = sec->m_systems.begin(); i != sec->m_systems.end(); ++i) {
		// skip the system if it doesn't fall within the sphere we're viewing.
		if ((m_pos*Sector::SIZE - (*i).FullPosition()).Length() > (m_zoomClamped/FAR_THRESHOLD )*OUTER_RADIUS) continue;

		// if the system belongs to a faction we've chosen to hide also skip it, if it's not selectd in some way
		m_visibleFactions.insert(i->faction);
		if (m_hiddenFactions.find(i->faction) != m_hiddenFactions.end()
			&& !i->IsSameSystem(m_selected) && !i->IsSameSystem(m_hyperspaceTarget) && !i->IsSameSystem(m_current)) continue;

		// otherwise add the system's position (origin must be m_pos's *sector* or we get judder)
		// and faction color to the list to draw
		starColor = (*i).faction->colour;
		starColor.a = 191;

		points.push_back((*i).FullPosition() - origin);
		colors.push_back(starColor);
	}
}

void SectorView::OnSwitchTo()
{
	m_renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());

	if (!m_onKeyPressConnection.connected())
		m_onKeyPressConnection =
			Pi::onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnKeyPressed));

	Update();

	UpdateSystemLabels(m_selectedSystemLabels, m_selected);
	UpdateSystemLabels(m_targetSystemLabels, m_hyperspaceTarget);
}

void SectorView::RefreshDetailBoxVisibility()
{
	if (m_detailBoxVisible != DETAILBOX_INFO)    m_infoBox->HideAll();    else m_infoBox->ShowAll();
	if (m_detailBoxVisible != DETAILBOX_FACTION) m_factionBox->HideAll(); else UpdateFactionToggles();
}

void SectorView::OnKeyPressed(SDL_Keysym *keysym)
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

	// cycle through the info box, the faction box, and nothing
	if (keysym->sym == SDLK_TAB) {
		if (m_detailBoxVisible == DETAILBOX_FACTION) m_detailBoxVisible = DETAILBOX_NONE;
		else                                         m_detailBoxVisible++;
		RefreshDetailBoxVisibility();
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
	PROFILE_SCOPED()
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
		m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);

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

		SectorCache::SetPosition(m_pos);

		float diffX = m_rotXMovingTo - m_rotX;
		float travelX = diffX * 10.0f*frameTime;
		if (fabs(travelX) > fabs(diffX)) m_rotX = m_rotXMovingTo;
		else m_rotX = m_rotX + travelX;

		float diffZ = m_rotZMovingTo - m_rotZ;
		float travelZ = diffZ * 10.0f*frameTime;
		if (fabs(travelZ) > fabs(diffZ)) m_rotZ = m_rotZMovingTo;
		else m_rotZ = m_rotZ + travelZ;

		float prevZoom = m_zoom;
		float diffZoom = m_zoomMovingTo - m_zoom;
		float travelZoom = diffZoom * ZOOM_SPEED*frameTime;
		if (fabs(travelZoom) > fabs(diffZoom)) m_zoom = m_zoomMovingTo;
		else m_zoom = m_zoom + travelZoom;
		m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);

		// swtich between Info and Faction panels when we zoom over the threshold
		if (m_zoom <= FAR_THRESHOLD && prevZoom > FAR_THRESHOLD && m_detailBoxVisible == DETAILBOX_FACTION) {
			m_detailBoxVisible = DETAILBOX_INFO;
			RefreshDetailBoxVisibility();
		}
		if (m_zoom > FAR_THRESHOLD && prevZoom <= FAR_THRESHOLD && m_detailBoxVisible == DETAILBOX_INFO) {
			m_detailBoxVisible = DETAILBOX_FACTION;
			RefreshDetailBoxVisibility();
		}
	}

	if (m_selectionFollowsMovement) {
		SystemPath new_selected = SystemPath(int(floor(m_pos.x)), int(floor(m_pos.y)), int(floor(m_pos.z)), 0);

		Sector* ps = SectorCache::GetCached(new_selected.sectorX, new_selected.sectorY, new_selected.sectorZ);
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

	SectorCache::ShrinkCache();

	m_playerHyperspaceRange = Pi::player->GetStats().hyperspace_range;

	if(!m_jumpSphere)
	{
		Graphics::MaterialDescriptor matdesc;
		matdesc.effect = EFFECT_FRESNEL_SPHERE;
		RefCountedPtr<Graphics::Material> fresnelMat(Pi::renderer->CreateMaterial(matdesc));
		m_jumpSphere.reset( new Graphics::Drawables::Sphere3D(fresnelMat, 3, 1.0f) );
		m_jumpDisk.reset( new Graphics::Drawables::Disk(fresnelMat, 72, 1.0f) );
	}
}

void SectorView::ShowAll()
{
	View::ShowAll();
	if (m_detailBoxVisible != DETAILBOX_INFO)    m_infoBox->HideAll();
	if (m_detailBoxVisible != DETAILBOX_FACTION) m_factionBox->HideAll();
}

void SectorView::MouseWheel(bool up)
{
	if (this == Pi::GetView()) {
		if (!up)
			m_zoomMovingTo += ZOOM_SPEED * WHEEL_SENSITIVITY * Pi::GetMoveSpeedShiftModifier();
		else
			m_zoomMovingTo -= ZOOM_SPEED * WHEEL_SENSITIVITY * Pi::GetMoveSpeedShiftModifier();
	}
}
