// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Factions.h"
#include "Game.h"
#include "Lang.h"
#include "LuaConstants.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "ShipCpanel.h"
#include "StringF.h"
#include "SystemInfoView.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/GalaxyCache.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "gui/Gui.h"
#include "KeyBindings.h"
#include "GameSaveError.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <SDL_stdinc.h>

using namespace Graphics;

static const int DRAW_RAD = 5;
#define INNER_RADIUS (Sector::SIZE*1.5f)
#define OUTER_RADIUS (Sector::SIZE*float(DRAW_RAD))
static const float FAR_THRESHOLD = 7.5f;
static const float FAR_LIMIT     = 36.f;
static const float FAR_MAX       = 46.f;

enum DetailSelection {
	DETAILBOX_NONE    = 0,
	DETAILBOX_INFO    = 1,
	DETAILBOX_FACTION = 2
};

static const float ZOOM_SPEED = 15;
static const float WHEEL_SENSITIVITY = .03f;		// Should be a variable in user settings.

SectorView::SectorView(Game* game) : UIView(), m_galaxy(game->GetGalaxy())
{
	InitDefaults();

	m_rotX = m_rotXMovingTo = m_rotXDefault;
	m_rotZ = m_rotZMovingTo = m_rotZDefault;
	m_zoom = m_zoomMovingTo = m_zoomDefault;

	// XXX I have no idea if this is correct,
	// I just copied it from the one other place m_zoomClamped is set
	m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);

	m_inSystem = true;

	RefCountedPtr<StarSystem> system = Pi::game->GetSpace()->GetStarSystem();
	m_current = system->GetPath();
	assert(!m_current.IsSectorPath());
	m_current = m_current.SystemOnly();
	m_selected = m_hyperspaceTarget = system->GetStars()[0]->GetPath(); // XXX This always selects the first star of the system

	GotoSystem(m_current);
	m_pos = m_posMovingTo;

	m_matchTargetToSelection   = true;
	m_automaticSystemSelection = true;
	m_detailBoxVisible         = DETAILBOX_INFO;
	m_toggledFaction           = false;

	InitObject();
}

SectorView::SectorView(const Json &jsonObj, Game* game) : UIView(), m_galaxy(game->GetGalaxy())
{
	InitDefaults();

	try {
		Json sectorViewObj = jsonObj["sector_view"];

		m_pos.x = m_posMovingTo.x = sectorViewObj["pos_x"];
		m_pos.y = m_posMovingTo.y = sectorViewObj["pos_y"];
		m_pos.z = m_posMovingTo.z = sectorViewObj["pos_z"];
		m_rotX = m_rotXMovingTo = sectorViewObj["rot_x"];
		m_rotZ = m_rotZMovingTo = sectorViewObj["rot_z"];
		m_zoom = m_zoomMovingTo = sectorViewObj["zoom"];
		// XXX I have no idea if this is correct,
		// I just copied it from the one other place m_zoomClamped is set
		m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);
		m_inSystem = sectorViewObj["in_system"];
		m_current = SystemPath::FromJson(sectorViewObj["current"]);
		m_selected = SystemPath::FromJson(sectorViewObj["selected"]);
		m_hyperspaceTarget = SystemPath::FromJson(sectorViewObj["hyperspace"]);
		m_matchTargetToSelection = sectorViewObj["match_target_to_selection"];
		m_automaticSystemSelection = sectorViewObj["automatic_system_selection"];
		m_detailBoxVisible = sectorViewObj["detail_box_visible"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

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
	m_cacheXMin = 0;
	m_cacheXMax = 0;
	m_cacheYMin = 0;
	m_cacheYMax = 0;
	m_cacheYMin = 0;
	m_cacheYMax = 0;

	m_sectorCache = m_galaxy->NewSectorSlaveCache();

	m_drawRouteLines = true; // where should this go?!
	m_route = std::vector<SystemPath>();
}

void SectorView::InitObject()
{
	SetTransparency(true);

	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_secLineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_starVerts.reset(new Graphics::VertexArray(
						    Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0, 500));

	Gui::Screen::PushFont("OverlayFont");
	m_clickableLabels = new Gui::LabelSet();
	m_clickableLabels->SetLabelColor(Color(178,178,178,191));
	Add(m_clickableLabels, 0, 0);
	Gui::Screen::PopFont();

	Gui::Screen::PushFont("OverlayFont");

	m_renderer = Pi::renderer; //XXX pass cleanly to all views constructors!

	Graphics::RenderStateDesc rsd;
	m_solidState = m_renderer->CreateRenderState(rsd);

	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	rsd.cullMode = CULL_NONE;
	m_alphaBlendState = m_renderer->CreateRenderState(rsd);

	Graphics::MaterialDescriptor bbMatDesc;
	bbMatDesc.effect = Graphics::EFFECT_SPHEREIMPOSTOR;
	m_starMaterial.Reset(m_renderer->CreateMaterial(bbMatDesc));

	m_disk.reset(new Graphics::Drawables::Disk(m_renderer, m_solidState, Color::WHITE, 0.2f));

	m_onMouseWheelCon =
		Pi::input.onMouseWheel.connect(sigc::mem_fun(this, &SectorView::MouseWheel));
}

SectorView::~SectorView()
{
	m_onMouseWheelCon.disconnect();
	if (m_onKeyPressConnection.connected()) m_onKeyPressConnection.disconnect();
}

void SectorView::SaveToJson(Json &jsonObj)
{
	Json sectorViewObj({}); // Create JSON object to contain sector view data.

	sectorViewObj["pos_x"] = m_pos.x;
	sectorViewObj["pos_y"] = m_pos.y;
	sectorViewObj["pos_z"] = m_pos.z;
	sectorViewObj["rot_x"] = m_rotX;
	sectorViewObj["rot_z"] = m_rotZ;
	sectorViewObj["zoom"] = m_zoom;
	sectorViewObj["in_system"] = m_inSystem;

	Json currentSystemObj({}); // Create JSON object to contain current system data.
	m_current.ToJson(currentSystemObj);
	sectorViewObj["current"] = currentSystemObj; // Add current system object to sector view object.

	Json selectedSystemObj({}); // Create JSON object to contain selected system data.
	m_selected.ToJson(selectedSystemObj);
	sectorViewObj["selected"] = selectedSystemObj; // Add selected system object to sector view object.

	Json hyperspaceSystemObj({}); // Create JSON object to contain hyperspace system data.
	m_hyperspaceTarget.ToJson(hyperspaceSystemObj);
	sectorViewObj["hyperspace"] = hyperspaceSystemObj; // Add hyperspace system object to sector view object.

	sectorViewObj["match_target_to_selection"] = m_matchTargetToSelection;
	sectorViewObj["automatic_system_selection"] = m_automaticSystemSelection;
	sectorViewObj["detail_box_visible"] = m_detailBoxVisible;

	jsonObj["sector_view"] = sectorViewObj; // Add sector view object to supplied object.
}


#define FFRAC(_x)	((_x)-floor(_x))

void SectorView::Draw3D()
{
	PROFILE_SCOPED()

	m_lineVerts->Clear();
	m_secLineVerts->Clear();
	m_clickableLabels->Clear();
	m_starVerts->Clear();

	if (m_zoomClamped <= FAR_THRESHOLD) m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 300.f);
	else                                m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 600.f);

	matrix4x4f modelview = matrix4x4f::Identity();

	m_renderer->ClearScreen();

	Graphics::Renderer::MatrixTicket ticket(m_renderer, Graphics::MatrixMode::MODELVIEW);

	// units are lightyears, my friend
	modelview.Translate(0.f, 0.f, -10.f-10.f*m_zoom);    // not zoomClamped, let us zoom out a bit beyond what we're drawing
	modelview.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	modelview.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);
	modelview.Translate(-FFRAC(m_pos.x)*Sector::SIZE, -FFRAC(m_pos.y)*Sector::SIZE, -FFRAC(m_pos.z)*Sector::SIZE);
	m_renderer->SetTransform(modelview);

	RefCountedPtr<const Sector> playerSec = GetCached(m_current);
	const vector3f playerPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) + playerSec->m_systems[m_current.systemIndex].GetPosition();


	if (m_zoomClamped <= FAR_THRESHOLD)
		DrawNearSectors(modelview);
	else
		DrawFarSectors(modelview);

	m_renderer->SetTransform(matrix4x4f::Identity());

	//draw star billboards in one go
	m_renderer->SetAmbientColor(Color(30, 30, 30));
	m_renderer->DrawTriangles(m_starVerts.get(), m_solidState, m_starMaterial.Get());

	//draw sector legs in one go
	if(!m_lineVerts->IsEmpty()) {
		m_lines.SetData(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);
		m_lines.Draw(m_renderer, m_alphaBlendState);
	}

	if (!m_secLineVerts->IsEmpty()) {
		m_sectorlines.SetData( m_secLineVerts->GetNumVerts(), &m_secLineVerts->position[0], &m_secLineVerts->diffuse[0]);
		m_sectorlines.Draw(m_renderer, m_alphaBlendState);
	}

	// not quite the same as modelview
	matrix4x4f trans = matrix4x4f::Identity();
	trans.Translate(0.f, 0.f, -10.f - 10.f*m_zoom);
	trans.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	trans.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);
	trans.Translate(-(m_pos.x)*Sector::SIZE, -(m_pos.y)*Sector::SIZE, -(m_pos.z)*Sector::SIZE);

	DrawRouteLines(playerPos, trans);

	UIView::Draw3D();
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
	FloatHyperspaceTarget();

	if (!old.IsSameSystem(m_hyperspaceTarget)) {
		onHyperspaceTargetChanged.emit();
	}
}

void SectorView::GotoSector(const SystemPath &path)
{
	m_posMovingTo = vector3f(path.sectorX, path.sectorY, path.sectorZ);

	// for performance don't animate the travel if we're Far Zoomed
	if (m_zoomClamped > FAR_THRESHOLD) {
		m_pos = m_posMovingTo;
	}
}

void SectorView::GotoSystem(const SystemPath &path)
{
	RefCountedPtr<Sector> ps = GetCached(path);
	const vector3f &p = ps->m_systems[path.systemIndex].GetPosition();
	m_posMovingTo.x = path.sectorX + p.x/Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y/Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z/Sector::SIZE;

	// for performance don't animate the travel if we're Far Zoomed
	if (m_zoomClamped > FAR_THRESHOLD) {
		m_pos = m_posMovingTo;
	}
}

void SectorView::SetSelected(const SystemPath &path)
{
	m_selected = path;

	if (m_matchTargetToSelection && m_selected != m_current) {
		m_hyperspaceTarget = m_selected;
		onHyperspaceTargetChanged.emit();
	}
}

void SectorView::SwapSelectedHyperspaceTarget()
{
	SystemPath tmpTarget = GetHyperspaceTarget();
	SetHyperspaceTarget(GetSelected());
	if (m_automaticSystemSelection) {
		GotoSystem(tmpTarget);
	} else {
		RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(tmpTarget);
		SetSelected(system->GetStars()[0]->GetPath());
	}
}

void SectorView::OnClickSystem(const SystemPath &path)
{
	if (path.IsSameSystem(m_selected)) {
		RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(path);
		if (system->GetNumStars() > 1 && m_selected.IsBodyPath()) {
			unsigned i;
			for (i = 0; i < system->GetNumStars(); ++i)
				if (system->GetStars()[i]->GetPath() == m_selected) break;
			if (i >= system->GetNumStars() - 1)
				SetSelected(system->GetStars()[0]->GetPath());
			else
				SetSelected(system->GetStars()[i+1]->GetPath());
		} else {
			SetSelected(system->GetStars()[0]->GetPath());
		}
	} else {
		if (m_automaticSystemSelection) {
			GotoSystem(path);
		} else {
			RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(path);
			SetSelected(system->GetStars()[0]->GetPath());
		}
	}
}

void SectorView::PutSystemLabels(RefCountedPtr<Sector> sec, const vector3f &origin, int drawRadius)
{
	PROFILE_SCOPED()
		Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator sys = sec->m_systems.begin(); sys !=sec->m_systems.end(); ++sys, ++sysIdx) {
		// skip the system if it doesn't fall within the sphere we're viewing.
		if ((m_pos*Sector::SIZE - (*sys).GetFullPosition()).Length() > drawRadius) continue;

		// if the system is the current system or target we can't skip it
		bool can_skip = !sys->IsSameSystem(m_selected)
			&& !sys->IsSameSystem(m_hyperspaceTarget)
			&& !sys->IsSameSystem(m_current);

		// skip the system if it belongs to a Faction we've toggled off and we can skip it
		if (m_hiddenFactions.find(sys->GetFaction()) != m_hiddenFactions.end() && can_skip) continue;

		// determine if system in hyperjump range or not
		RefCountedPtr<const Sector> playerSec = GetCached(m_current);
		float dist = Sector::DistanceBetween(sec, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// place the label
		vector3d systemPos = vector3d((*sys).GetFullPosition() - origin);
		vector3d screenPos;
		if (Gui::Screen::Project(systemPos, screenPos)) {
			// reject back-projected labels
			if(screenPos.z > 1.0f)
				continue;

			// work out the colour
			Color labelColor = sys->GetFaction()->AdjustedColour(sys->GetPopulation(), inRange);

			// get a system path to pass to the event handler when the label is licked
			SystemPath sysPath = SystemPath((*sys).sx, (*sys).sy, (*sys).sz, sysIdx);

			// label text
			std::string text = "";
			if(((inRange || m_drawOutRangeLabels) && (sys->GetPopulation() > 0 || m_drawUninhabitedLabels)) || !can_skip)
				text = sys->GetName();

			// setup the label;
			m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), sysPath), screenPos.x, screenPos.y, labelColor);
		}
	}
}

void SectorView::PutFactionLabels(const vector3f &origin)
{
	PROFILE_SCOPED()

		m_renderer->SetDepthRange(0,1);
	Gui::Screen::EnterOrtho();

	if (!m_material)
		m_material.Reset(m_renderer->CreateMaterial(Graphics::MaterialDescriptor()));

	static const Color labelBorder(13, 13, 31, 166);
	const auto renderState = Gui::Screen::alphaBlendState;

	for (auto it = m_visibleFactions.begin(); it != m_visibleFactions.end(); ++it) {
		if ((*it)->hasHomeworld && m_hiddenFactions.find((*it)) == m_hiddenFactions.end()) {

			Sector::System sys = GetCached((*it)->homeworld)->m_systems[(*it)->homeworld.systemIndex];
			if ((m_pos*Sector::SIZE - sys.GetFullPosition()).Length() > (m_zoomClamped/FAR_THRESHOLD )*OUTER_RADIUS) continue;

			vector3d pos;
			if (Gui::Screen::Project(vector3d(sys.GetFullPosition() - origin), pos)) {

				std::string labelText    = sys.GetName() + "\n" + (*it)->name;
				Color labelColor  = (*it)->colour;
				float labelHeight = 0;
				float labelWidth  = 0;

				Gui::Screen::MeasureString(labelText, labelWidth, labelHeight);

				// draw a big diamond for the location of the star
				static const float STARSIZE = 5;
				Graphics::VertexArray outline(Graphics::ATTRIB_POSITION);
				outline.Add(vector3f(pos.x - STARSIZE - 1.f, pos.y, 1));
				outline.Add(vector3f(pos.x, pos.y + STARSIZE + 1.f, 1));
				outline.Add(vector3f(pos.x, pos.y - STARSIZE - 1.f, 1));
				outline.Add(vector3f(pos.x + STARSIZE + 1.f, pos.y, 1));
				m_material->diffuse = { 0, 0, 0, 255 };
				m_renderer->DrawTriangles(&outline, renderState, m_material.Get(), Graphics::TRIANGLE_STRIP);

				Graphics::VertexArray marker(Graphics::ATTRIB_POSITION);
				marker.Add(vector3f(pos.x - STARSIZE, pos.y, 0));
				marker.Add(vector3f(pos.x, pos.y + STARSIZE, 0));
				marker.Add(vector3f(pos.x, pos.y - STARSIZE, 0));
				marker.Add(vector3f(pos.x + STARSIZE, pos.y, 0));
				m_material->diffuse = labelColor;
				m_renderer->DrawTriangles(&marker, renderState, m_material.Get(), Graphics::TRIANGLE_STRIP);

				// draw a surface for the label to sit on
				static const float MARGINLEFT = 8;
				float halfheight = labelHeight / 2.0;
				Graphics::VertexArray surface(Graphics::ATTRIB_POSITION);
				surface.Add(vector3f(pos.x + MARGINLEFT - 2.f, pos.y - halfheight, 0));
				surface.Add(vector3f(pos.x + MARGINLEFT - 2.f, pos.y + halfheight, 0));
				surface.Add(vector3f(pos.x + MARGINLEFT + labelWidth + 2.f, pos.y - halfheight, 0));
				surface.Add(vector3f(pos.x + MARGINLEFT + labelWidth + 2.f, pos.y + halfheight, 0));
				m_material->diffuse = labelBorder;
				m_renderer->DrawTriangles(&surface, renderState, m_material.Get(), Graphics::TRIANGLE_STRIP);

				if (labelColor.GetLuminance() > 204)
					labelColor.a = 204;    // luminance is sometimes a bit overly
				m_clickableLabels->Add(labelText, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), (*it)->homeworld), pos.x + MARGINLEFT, pos.y - (halfheight / 2.0) - 1.f, labelColor);
			}
		}
	}
	Gui::Screen::LeaveOrtho();
}

void SectorView::AddStarBillboard(const matrix4x4f &trans, const vector3f &pos, const Color &col, float size)
{
	const matrix3x3f rot = trans.GetOrient().Transpose();

	const vector3f offset = trans * pos;

	const vector3f rotv1 = rot * vector3f(size/2.f, -size/2.f, 0.0f);
	const vector3f rotv2 = rot * vector3f(size/2.f, size/2.f, 0.0f);

	Graphics::VertexArray &va = *m_starVerts;
	va.Add(offset-rotv1, col, vector2f(0.f, 0.f)); //top left
	va.Add(offset-rotv2, col, vector2f(0.f, 1.f)); //bottom left
	va.Add(offset+rotv2, col, vector2f(1.f, 0.f)); //top right

	va.Add(offset+rotv2, col, vector2f(1.f, 0.f)); //top right
	va.Add(offset-rotv2, col, vector2f(0.f, 1.f)); //bottom left
	va.Add(offset+rotv1, col, vector2f(1.f, 1.f)); //bottom right
}

void SectorView::OnToggleFaction(Gui::ToggleButton* button, bool pressed, const Faction* faction)
{
	// hide or show the faction's systems depending on whether the button is pressed
	if (pressed) m_hiddenFactions.erase(faction);
	else         m_hiddenFactions.insert(faction);

	m_toggledFaction = true;
}

void SectorView::DrawNearSectors(const matrix4x4f& modelview)
{
	PROFILE_SCOPED()
		m_visibleFactions.clear();

	RefCountedPtr<const Sector> playerSec = GetCached(m_current);
	const vector3f playerPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) + playerSec->m_systems[m_current.systemIndex].GetPosition();


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
	m_renderer->SetDepthRange(0,1);
	Gui::Screen::EnterOrtho();
	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				PutSystemLabels(GetCached(SystemPath(sx + secOrigin.x, sy + secOrigin.y, sz + secOrigin.z)), Sector::SIZE * secOrigin, Sector::SIZE * DRAW_RAD);
			}
		}
	}
	Gui::Screen::LeaveOrtho();
}

bool SectorView::MoveRouteItemUp(const std::vector<SystemPath>::size_type element) {
	if (element <= 0 || element >= m_route.size()) return false;

	std::swap(m_route[element - 1], m_route[element]);

	return true;
}

bool SectorView::MoveRouteItemDown(const std::vector<SystemPath>::size_type element) {
	if (element < 0 || element >= m_route.size() - 1) return false;

	std::swap(m_route[element + 1], m_route[element]);

	return true;
}

void SectorView::AddToRoute(const SystemPath &path)
{
	m_route.push_back(path);
}

bool SectorView::RemoveRouteItem(const std::vector<SystemPath>::size_type element) {
	if (element >= 0 && element < m_route.size()) {
		m_route.erase(m_route.begin() + element);
		return true;
	} else {
		return false;
	}
}

void SectorView::ClearRoute()
{
	m_route.clear();
}

std::vector<SystemPath> SectorView::GetRoute()
{
	return m_route;
}

void SectorView::AutoRoute(const SystemPath &start, const SystemPath &target, std::vector<SystemPath> &outRoute) const
{
	const RefCountedPtr<const Sector> start_sec = m_galaxy->GetSector(start);
	const RefCountedPtr<const Sector> target_sec = m_galaxy->GetSector(target);

	// Get the player's hyperdrive from Lua, later used to calculate the duration between systems
	const ScopedTable hyperdrive = ScopedTable(LuaObject<Player>::CallMethod<LuaRef>(Pi::player, "GetEquip", "engine", 1));
	// Cache max range so it doesn't get recalculated every time we call GetDuration
	const float max_range = hyperdrive.CallMethod<float>("GetMaximumRange", Pi::player);

	const float dist = Sector::DistanceBetween(start_sec, start.systemIndex, target_sec, target.systemIndex);

	// nodes[0] is always start
	std::vector<SystemPath> nodes;
	nodes.push_back(start);

	const Sint32 minX = std::min(start.sectorX, target.sectorX)-2, maxX = std::max(start.sectorX, target.sectorX)+2;
	const Sint32 minY = std::min(start.sectorY, target.sectorY)-2, maxY = std::max(start.sectorY, target.sectorY)+2;
	const Sint32 minZ = std::min(start.sectorZ, target.sectorZ)-2, maxZ = std::max(start.sectorZ, target.sectorZ)+2;
	const vector3f start_pos = start_sec->m_systems[start.systemIndex].GetFullPosition();
	const vector3f target_pos = target_sec->m_systems[target.systemIndex].GetFullPosition();

	// go sector by sector for the minimum cube of sectors and add systems
	// if they are within 110% of dist of both start and target
	for (Sint32 sx = minX; sx <= maxX; sx++) {
		for (Sint32 sy = minY; sy <= maxY; sy++) {
			for (Sint32 sz = minZ; sz < maxZ; sz++) {
				const SystemPath sec_path = SystemPath(sx, sy, sz);
				RefCountedPtr<const Sector> sec = m_galaxy->GetSector(sec_path);
				for (std::vector<Sector::System>::size_type s = 0; s < sec->m_systems.size(); s++) {
					if (start.IsSameSystem(sec->m_systems[s].GetPath()))
						continue; // start is already nodes[0]

					const float lineDist = MathUtil::DistanceFromLine(start_pos, target_pos, sec->m_systems[s].GetFullPosition());

					if (Sector::DistanceBetween(start_sec, start.systemIndex, sec, sec->m_systems[s].idx) <= dist * 1.10 &&
						Sector::DistanceBetween(target_sec, target.systemIndex, sec, sec->m_systems[s].idx) <= dist * 1.10 &&
						lineDist<(Sector::SIZE*3))
					{
						nodes.push_back(sec->m_systems[s].GetPath());
					}
				}
			}
		}
	}
	Output("SectorView::AutoRoute, nodes to search = %lu\n", nodes.size());

	// setup inital values and set everything as unvisited
	std::vector<float> path_dist; // distance from source to node
	std::vector<std::vector<SystemPath>::size_type> path_prev; // previous node in optimal path
	std::unordered_set<std::vector<SystemPath>::size_type> unvisited;
	for (std::vector<SystemPath>::size_type i = 0; i < nodes.size(); i++) {
		path_dist.push_back(INFINITY);
		path_prev.push_back(0);
		unvisited.insert(i);
	}

	// distance to the start is 0
	path_dist[0] = 0.f;

	size_t totalSkipped = 0u;
	while (unvisited.size() > 0) {
		// find the closest node (for the first loop this will be start)
		std::vector<SystemPath>::size_type closest_i = *unvisited.begin();
		for (auto it : unvisited) {
			if (path_dist[it] < path_dist[closest_i])
				closest_i = it;
		}

		// mark it as visited
		unvisited.erase(closest_i);

		// if this is the target then we have found the route
		const SystemPath &closest = nodes[closest_i];
		if (closest.IsSameSystem(target))
			break;

		RefCountedPtr<const Sector> closest_sec = m_galaxy->GetSector(closest);

		// if not, loop through all unvisited nodes
		// since every system is technically reachable from every other system
		// everything is a neighbor :)
		for (auto it : unvisited) {
			const SystemPath &v = nodes[it];
			// everything is a neighbor isn't quite true as the ship has a max_range for each jump!
			if ((SystemPath::SectorDistance(closest, v)*Sector::SIZE) > max_range) {
				++totalSkipped;
				continue;
			}

			RefCountedPtr<const Sector> v_sec = m_galaxy->GetSector(v); // this causes it to generate a sector (slooooooow)

			const float v_dist_ly = Sector::DistanceBetween(closest_sec, closest.systemIndex, v_sec, v.systemIndex);

			// in this case, duration is used for the distance since that's what we are optimizing
			float v_dist = hyperdrive.CallMethod<float>("GetDuration", Pi::player, v_dist_ly, max_range);

			v_dist += path_dist[closest_i]; // we want the total duration from start to this node
			if (v_dist < path_dist[it]) {
				// if our calculated duration is less than a previous value, this path is more efficent
				// so store/override it
				path_dist[it] = v_dist;
				path_prev[it] = closest_i;
			}
		}
	}
	Output("SectorView::AutoRoute, total times that nodes were skipped = %lu\n", totalSkipped);

	bool foundRoute = false;
	std::vector<SystemPath>::size_type u = 0;

	// find the index of our target
	for (std::vector<SystemPath>::size_type i = 0, numNodes = nodes.size(); i < numNodes; i++) {
		if (target.IsSameSystem(nodes[i])) {
			u = i;
			foundRoute = true;
			break;
		}
	}

	// It's posible that there is no valid route
	if (foundRoute) {
		outRoute.reserve(nodes.size());
		// Build the route, in reverse starting with the target
		while (u != 0) {
			outRoute.push_back(nodes[u]);
			u = path_prev[u];
		}
		std::reverse(std::begin(outRoute), std::end(outRoute));
	}
}

void SectorView::DrawRouteLines(const vector3f &playerAbsPos, const matrix4x4f &trans)
{
	for (std::vector<SystemPath>::size_type i = 0; i < m_route.size(); ++i) {
		RefCountedPtr<const Sector> jumpSec = m_galaxy->GetSector(m_route[i]);
		const Sector::System& jumpSecSys = jumpSec->m_systems[m_route[i].systemIndex];
		const vector3f jumpAbsPos = Sector::SIZE*vector3f(float(jumpSec->sx), float(jumpSec->sy), float(jumpSec->sz)) + jumpSecSys.GetPosition();

		vector3f startPos;
		if (i == 0) {
			startPos = playerAbsPos;
		} else {
			RefCountedPtr<const Sector> prevSec = m_galaxy->GetSector(m_route[i-1]);
			const Sector::System& prevSecSys = prevSec->m_systems[m_route[i-1].systemIndex];
			const vector3f prevAbsPos = Sector::SIZE*vector3f(float(prevSec->sx), float(prevSec->sy), float(prevSec->sz)) + prevSecSys.GetPosition();
			startPos = prevAbsPos;
		}
		std::unique_ptr<Graphics::VertexArray> verts;
		Graphics::Drawables::Lines lines;
		verts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
		verts->Clear();

		verts->position.reserve(2);
		verts->diffuse.reserve(2);

		verts->Add(trans* startPos, Color(20, 20, 0, 127));
		verts->Add(trans* jumpAbsPos, Color(255, 255, 0, 255));

		lines.SetData(verts->GetNumVerts(), &verts->position[0], &verts->diffuse[0]);
		lines.Draw(m_renderer, m_alphaBlendState);
	}
}

void SectorView::DrawNearSector(const int sx, const int sy, const int sz, const vector3f &playerAbsPos,const matrix4x4f &trans)
{
	PROFILE_SCOPED()
		m_renderer->SetTransform(trans);
	RefCountedPtr<Sector> ps = GetCached(SystemPath(sx, sy, sz));

	const int cz = int(floor(m_pos.z+0.5f));

	if (cz == sz) {
		static const Color darkgreen(0, 51, 0, 255);
		const vector3f vts[] = {
			trans * vector3f(0.f, 0.f, 0.f),
			trans * vector3f(0.f, Sector::SIZE, 0.f),
			trans * vector3f(Sector::SIZE, Sector::SIZE, 0.f),
			trans * vector3f(Sector::SIZE, 0.f, 0.f)
		};

		// reserve some more space
		const size_t newNum = m_secLineVerts->GetNumVerts() + 8;
		m_secLineVerts->position.reserve(newNum);
		m_secLineVerts->diffuse.reserve(newNum);

		m_secLineVerts->Add(vts[0], darkgreen);	// line segment 1
		m_secLineVerts->Add(vts[1], darkgreen);
		m_secLineVerts->Add(vts[1], darkgreen);	// line segment 2
		m_secLineVerts->Add(vts[2], darkgreen);
		m_secLineVerts->Add(vts[2], darkgreen);	// line segment 3
		m_secLineVerts->Add(vts[3], darkgreen);
		m_secLineVerts->Add(vts[3], darkgreen);	// line segment 4
		m_secLineVerts->Add(vts[0], darkgreen);
	}

	const size_t numLineVerts = ps->m_systems.size() * 8;
	m_lineVerts->position.reserve(numLineVerts);
	m_lineVerts->diffuse.reserve(numLineVerts);

	Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i, ++sysIdx) {
		// calculate where the system is in relation the centre of the view...
		const vector3f sysAbsPos = Sector::SIZE*vector3f(float(sx), float(sy), float(sz)) + i->GetPosition();
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
		m_visibleFactions.insert(i->GetFaction());
		if (m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end() && can_skip) continue;

		// determine if system in hyperjump range or not
		RefCountedPtr<const Sector> playerSec = GetCached(m_current);
		float dist = Sector::DistanceBetween(ps, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// don't worry about looking for inhabited systems if they're
		// unexplored (same calculation as in StarSystem.cpp) or we've
		// already retrieved their population.
		if (i->GetPopulation() < 0 && isqrt(1 + sx*sx + sy*sy + sz*sz) <= 90) {

			// only do this once we've pretty much stopped moving.
			vector3f diff = vector3f(
															 fabs(m_posMovingTo.x - m_pos.x),
															 fabs(m_posMovingTo.y - m_pos.y),
															 fabs(m_posMovingTo.z - m_pos.z));

			// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
			if( (diff.x < 0.001f && diff.y < 0.001f && diff.z < 0.001f) ) {
				SystemPath current = SystemPath(sx, sy, sz, sysIdx);
				RefCountedPtr<StarSystem> pSS = m_galaxy->GetStarSystem(current);
				i->SetPopulation(pSS->GetTotalPop());
			}

		}

		matrix4x4f systrans = trans * matrix4x4f::Translation(i->GetPosition().x, i->GetPosition().y, i->GetPosition().z);
		m_renderer->SetTransform(systrans);

		// for out-of-range systems draw leg only if we draw label
		if ((m_drawVerticalLines && (inRange || m_drawOutRangeLabels) && (i->GetPopulation() > 0 || m_drawUninhabitedLabels)) || !can_skip) {

			const Color light(128, 128, 128);
			const Color dark(51, 51, 51);

			// draw system "leg"
			float z = -i->GetPosition().z;
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
			if (m_selected != m_current) {
				m_selectedLine.SetStart(vector3f(0.f, 0.f, 0.f));
				m_selectedLine.SetEnd(playerAbsPos - sysAbsPos);
				m_selectedLine.Draw(m_renderer, m_solidState);
			} else {
			}
			if (m_selected != m_hyperspaceTarget) {
				RefCountedPtr<Sector> hyperSec = GetCached(m_hyperspaceTarget);
				const vector3f hyperAbsPos =
					Sector::SIZE*vector3f(m_hyperspaceTarget.sectorX, m_hyperspaceTarget.sectorY, m_hyperspaceTarget.sectorZ)
					+ hyperSec->m_systems[m_hyperspaceTarget.systemIndex].GetPosition();
				if (m_selected != m_current) {
					m_secondLine.SetStart(vector3f(0.f, 0.f, 0.f));
					m_secondLine.SetEnd(hyperAbsPos - sysAbsPos);
					m_secondLine.Draw(m_renderer, m_solidState);
				}

				if (m_hyperspaceTarget != m_current) {
					// FIXME: Draw when drawing hyperjump target or current system
					m_jumpLine.SetStart(hyperAbsPos - sysAbsPos);
					m_jumpLine.SetEnd(playerAbsPos - sysAbsPos);
					m_jumpLine.Draw(m_renderer, m_solidState);
				}
			} else {
			}
		}

		// draw star blob itself
		systrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
		systrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
		systrans.Scale((StarSystem::starScale[(*i).GetStarType(0)]));
		m_renderer->SetTransform(systrans);

		const Uint8 *col = StarSystem::starColors[(*i).GetStarType(0)];
		AddStarBillboard(systrans, vector3f(0.f), Color(col[0], col[1], col[2], 255), 0.5f);

		// player location indicator
		if (m_inSystem && bIsCurrentSystem) {
			m_renderer->SetDepthRange(0.2,1.0);
			m_disk->SetColor(Color(0, 0, 204));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(3.f));
			m_disk->Draw(m_renderer);
		}
		// selected indicator
		if (bIsCurrentSystem) {
			m_renderer->SetDepthRange(0.1,1.0);
			m_disk->SetColor(Color(0, 204, 0));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}
		// hyperspace target indicator (if different from selection)
		if (i->IsSameSystem(m_hyperspaceTarget) && m_hyperspaceTarget != m_selected && (!m_inSystem || m_hyperspaceTarget != m_current)) {
			m_renderer->SetDepthRange(0.1,1.0);
			m_disk->SetColor(Color(77, 77, 77));
			m_renderer->SetTransform(systrans * matrix4x4f::ScaleMatrix(2.f));
			m_disk->Draw(m_renderer);
		}
		if(bIsCurrentSystem && m_jumpSphere && m_playerHyperspaceRange>0.0f) {
			const matrix4x4f sphTrans = trans * matrix4x4f::Translation(i->GetPosition().x, i->GetPosition().y, i->GetPosition().z);
			m_renderer->SetTransform(sphTrans * matrix4x4f::ScaleMatrix(m_playerHyperspaceRange));
			m_jumpSphere->Draw(m_renderer);
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
						BuildFarSector(GetCached(SystemPath(sx, sy, sz)), Sector::SIZE * secOrigin, m_farstars, m_farstarsColor);
					}
				}
			}
		}

		m_secPosFar      = secOrigin;
		m_radiusFar      = buildRadius;
		m_toggledFaction = false;
	}

	// always draw the stars, slightly altering their size for different different resolutions, so they still look okay
	if (m_farstars.size() > 0) {
		m_farstarsPoints.SetData(m_renderer, m_farstars.size(), &m_farstars[0], &m_farstarsColor[0], modelview, 0.25f * (Graphics::GetScreenHeight() / 720.f));
		m_farstarsPoints.Draw(m_renderer, m_alphaBlendState);
	}

	// also add labels for any faction homeworlds among the systems we've drawn
	PutFactionLabels(Sector::SIZE * secOrigin);
}

void SectorView::BuildFarSector(RefCountedPtr<Sector> sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors)
{
	PROFILE_SCOPED()
		Color starColor;
	for (std::vector<Sector::System>::iterator i = sec->m_systems.begin(); i != sec->m_systems.end(); ++i) {
		// skip the system if it doesn't fall within the sphere we're viewing.
		if ((m_pos*Sector::SIZE - (*i).GetFullPosition()).Length() > (m_zoomClamped/FAR_THRESHOLD )*OUTER_RADIUS) continue;

		if (!i->IsExplored())
		{
			points.push_back((*i).GetFullPosition() - origin);
			colors.push_back({ 100,100,100,155 });					// flat gray for unexplored systems
			continue;
		}

		// if the system belongs to a faction we've chosen to hide also skip it, if it's not selectd in some way
		m_visibleFactions.insert(i->GetFaction());
		if (m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end()
				&& !i->IsSameSystem(m_selected) && !i->IsSameSystem(m_hyperspaceTarget) && !i->IsSameSystem(m_current)) continue;

		// otherwise add the system's position (origin must be m_pos's *sector* or we get judder)
		// and faction color to the list to draw
		starColor = i->GetFaction()->colour;
		starColor.a = 120;

		points.push_back((*i).GetFullPosition() - origin);
		colors.push_back(starColor);
	}
}

void SectorView::OnSwitchTo()
{
	m_renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());

	if (!m_onKeyPressConnection.connected())
		m_onKeyPressConnection =
			Pi::input.onKeyPress.connect(sigc::mem_fun(this, &SectorView::OnKeyPressed));

	UIView::OnSwitchTo();

	Update();
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

	// space "locks" (or unlocks) the hyperspace target to the selected system
	if (KeyBindings::mapLockHyperspaceTarget.Matches(keysym)) {
		if ((m_matchTargetToSelection || m_hyperspaceTarget != m_selected) && !m_selected.IsSameSystem(m_current))
			SetHyperspaceTarget(m_selected);
		else
			ResetHyperspaceTarget();
		return;
	}

	if (KeyBindings::mapToggleSelectionFollowView.Matches(keysym)) {
		m_automaticSystemSelection = !m_automaticSystemSelection;
		return;
	}

	bool reset_view = false;

	// fast move selection to current player system or hyperspace target
	const bool shifted = (Pi::input.KeyState(SDLK_LSHIFT) || Pi::input.KeyState(SDLK_RSHIFT));
	if (KeyBindings::mapWarpToCurrent.Matches(keysym)) {
		GotoSystem(m_current);
		reset_view = shifted;
	} else if (KeyBindings::mapWarpToSelected.Matches(keysym)) {
		GotoSystem(m_selected);
		reset_view = shifted;
	} else if (KeyBindings::mapWarpToHyperspaceTarget.Matches(keysym)) {
		GotoSystem(m_hyperspaceTarget);
		reset_view = shifted;
	}

	// reset rotation and zoom
	if (reset_view || KeyBindings::mapViewReset.Matches(keysym)) {
		while (m_rotZ < -180.0f) m_rotZ += 360.0f;
		while (m_rotZ > 180.0f)  m_rotZ -= 360.0f;
		m_rotXMovingTo = m_rotXDefault;
		m_rotZMovingTo = m_rotZDefault;
		m_zoomMovingTo = m_zoomDefault;
	}
}

void SectorView::Update()
{
	PROFILE_SCOPED()
	SystemPath last_current = m_current;

	if (Pi::game->IsNormalSpace()) {
		m_inSystem = true;
		m_current = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	}
	else {
		m_inSystem = false;
		m_current = Pi::player->GetHyperspaceDest();
	}

	const float frameTime = Pi::GetFrameTime();

	matrix4x4f rot = matrix4x4f::Identity();
	rot.RotateX(DEG2RAD(-m_rotX));
	rot.RotateZ(DEG2RAD(-m_rotZ));

	// don't check raw keypresses if the search box is active
	// XXX ugly hack checking for Lua console here
	if (!Pi::IsConsoleActive()) {
		const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
		float move = moveSpeed*frameTime;
		vector3f shift(0.0f);
		if (KeyBindings::mapViewShiftLeft.IsActive()) shift.x -= move;
		if (KeyBindings::mapViewShiftRight.IsActive()) shift.x += move;
		if (KeyBindings::mapViewShiftUp.IsActive()) shift.y += move;
		if (KeyBindings::mapViewShiftDown.IsActive()) shift.y -= move;
		if (KeyBindings::mapViewShiftForward.IsActive()) shift.z -= move;
		if (KeyBindings::mapViewShiftBackward.IsActive()) shift.z += move;
		m_posMovingTo += shift * rot;

		if (KeyBindings::viewZoomIn.IsActive())
			m_zoomMovingTo -= move;
		if (KeyBindings::viewZoomOut.IsActive())
			m_zoomMovingTo += move;
		m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);

		if (KeyBindings::mapViewRotateLeft.IsActive()) m_rotZMovingTo -= 0.5f * moveSpeed;
		if (KeyBindings::mapViewRotateRight.IsActive()) m_rotZMovingTo += 0.5f * moveSpeed;
		if (KeyBindings::mapViewRotateUp.IsActive()) m_rotXMovingTo -= 0.5f * moveSpeed;
		if (KeyBindings::mapViewRotateDown.IsActive()) m_rotXMovingTo += 0.5f * moveSpeed;
	}

	if (Pi::input.MouseButtonState(SDL_BUTTON_RIGHT)) {
		int motion[2];
		Pi::input.GetMouseMotion(motion);

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
		m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);

	}

	if (m_automaticSystemSelection) {
		SystemPath new_selected = SystemPath(int(floor(m_pos.x)), int(floor(m_pos.y)), int(floor(m_pos.z)), 0);

		RefCountedPtr<Sector> ps = GetCached(new_selected);
		if (ps->m_systems.size()) {
			float px = FFRAC(m_pos.x)*Sector::SIZE;
			float py = FFRAC(m_pos.y)*Sector::SIZE;
			float pz = FFRAC(m_pos.z)*Sector::SIZE;

			float min_dist = FLT_MAX;
			for (unsigned int i=0; i<ps->m_systems.size(); i++) {
				Sector::System *ss = &ps->m_systems[i];
				float dx = px - ss->GetPosition().x;
				float dy = py - ss->GetPosition().y;
				float dz = pz - ss->GetPosition().z;
				float dist = sqrtf(dx*dx + dy*dy + dz*dz);
				if (dist < min_dist) {
					min_dist = dist;
					new_selected.systemIndex = i;
				}
			}

			if (!m_selected.IsSameSystem(new_selected)) {
				RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(new_selected);
				SetSelected(system->GetStars()[0]->GetPath());
			}
		}
	}

	ShrinkCache();

	m_playerHyperspaceRange = LuaObject<Player>::CallMethod<float>(Pi::player, "GetHyperspaceRange");

	if(!m_jumpSphere)
		{
			Graphics::RenderStateDesc rsd;
			rsd.blendMode = Graphics::BLEND_ALPHA;
			rsd.depthTest = false;
			rsd.depthWrite = false;
			rsd.cullMode = Graphics::CULL_NONE;
			m_jumpSphereState = m_renderer->CreateRenderState(rsd);

			Graphics::MaterialDescriptor matdesc;
			matdesc.effect = EFFECT_FRESNEL_SPHERE;
			m_fresnelMat.Reset(m_renderer->CreateMaterial(matdesc));
			m_fresnelMat->diffuse = Color::WHITE;
			m_jumpSphere.reset( new Graphics::Drawables::Sphere3D(m_renderer, m_fresnelMat, m_jumpSphereState, 4, 1.0f) );
		}

	UIView::Update();
}

void SectorView::ShowAll()
{
	View::ShowAll();
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

void SectorView::ShrinkCache()
{
	PROFILE_SCOPED()
		// we're going to use these to determine if our sectors are within the range that we'll ever render
		const int drawRadius = (m_zoomClamped <= FAR_THRESHOLD) ? DRAW_RAD : ceilf((m_zoomClamped/FAR_THRESHOLD) * DRAW_RAD);

	const int xmin = int(floorf(m_pos.x))-drawRadius;
	const int xmax = int(floorf(m_pos.x))+drawRadius;
	const int ymin = int(floorf(m_pos.y))-drawRadius;
	const int ymax = int(floorf(m_pos.y))+drawRadius;
	const int zmin = int(floorf(m_pos.z))-drawRadius;
	const int zmax = int(floorf(m_pos.z))+drawRadius;

	// XXX don't clear the current/selected/target sectors

	if  (xmin != m_cacheXMin || xmax != m_cacheXMax
			 || ymin != m_cacheYMin || ymax != m_cacheYMax
			 || zmin != m_cacheZMin || zmax != m_cacheZMax) {
		auto iter = m_sectorCache->Begin();
		while (iter != m_sectorCache->End())	{
			RefCountedPtr<Sector> s = iter->second;
			//check_point_in_box
			if (!s->WithinBox( xmin, xmax, ymin, ymax, zmin, zmax )) {
				m_sectorCache->Erase( iter++ );
			} else {
				iter++;
			}
		}

		m_cacheXMin = xmin;
		m_cacheXMax = xmax;
		m_cacheYMin = ymin;
		m_cacheYMax = ymax;
		m_cacheZMin = zmin;
		m_cacheZMax = zmax;
	}
}

double SectorView::GetZoomLevel() const {
	return ((m_zoomClamped/FAR_THRESHOLD )*(OUTER_RADIUS)) + 0.5 * Sector::SIZE;
}

void SectorView::ZoomIn() {
	const float frameTime = Pi::GetFrameTime();
	const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
	float move = moveSpeed*frameTime;
	m_zoomMovingTo -= move;
	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);
}

void SectorView::ZoomOut() {
	const float frameTime = Pi::GetFrameTime();
	const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
	float move = moveSpeed*frameTime;
	m_zoomMovingTo += move;
	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);
}

vector3f SectorView::GetCenterSector() {
	return m_pos;
}

double SectorView::GetCenterDistance() {
	if (m_inSystem) {
		vector3f dv = vector3f(floorf(m_pos.x)-m_current.sectorX, floorf(m_pos.y)-m_current.sectorY, floorf(m_pos.z)-m_current.sectorZ) * Sector::SIZE;
		return dv.Length();
	} else {
		return 0.0;
	}
}

void SectorView::LockHyperspaceTarget(bool lock) {
	if(lock) {
		SetHyperspaceTarget(GetSelected());
	} else {
		FloatHyperspaceTarget();
	}
}
std::vector<SystemPath> SectorView::GetNearbyStarSystemsByName(std::string pattern)
{
	std::vector<SystemPath> result;
	for(auto i = m_sectorCache->Begin(); i != m_sectorCache->End(); ++i) {
		for (unsigned int systemIndex = 0; systemIndex < (*i).second->m_systems.size(); systemIndex++)
			{
				const Sector::System *ss = &((*i).second->m_systems[systemIndex]);

				// compare with the start of the current system
				if (strncasecmp(pattern.c_str(), ss->GetName().c_str(), pattern.size()) == 0
						// look for the pattern term somewhere within the current system
						|| pi_strcasestr(ss->GetName().c_str(), pattern.c_str()))
					{
						SystemPath match((*i).first);
						match.systemIndex = systemIndex;
						result.push_back(match);
					}
			}
	}
	return result;
}

void SectorView::SetFactionVisible(const Faction *faction, bool visible)
{
	if (visible) m_hiddenFactions.erase(faction);
	else         m_hiddenFactions.insert(faction);
	m_toggledFaction = true;
}
