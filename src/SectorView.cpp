// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SectorView.h"

#include "AnimationCurves.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "Input.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "StringF.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyCache.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"
#include "matrix4x4.h"
#include "sigc++/functors/mem_fun.h"
#include "utils.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

using namespace Graphics;

static const int DRAW_RAD = 5;
#define INNER_RADIUS (Sector::SIZE * 1.5f)
#define OUTER_RADIUS (Sector::SIZE * float(DRAW_RAD))
static const float FAR_THRESHOLD = 7.5f;
static const float FAR_LIMIT = 36.f;
static const float FAR_MAX = 46.f;

enum DetailSelection {
	DETAILBOX_NONE = 0,
	DETAILBOX_INFO = 1,
	DETAILBOX_FACTION = 2
};

static const float ZOOM_SPEED = 15;
static const float WHEEL_SENSITIVITY = .03f; // Should be a variable in user settings.

#define FFRAC(_x) ((_x)-floor(_x))

enum Side {
	XMIN,
	XMAX,
	YMIN,
	YMAX,
	ZMIN,
	ZMAX,
	SIDES
};

using box_t = std::array<int, SIDES>;

static bool in_box(int x, int y, int z, const box_t &box)
{
	return x >= box[XMIN] && x < box[XMAX] &&
		y >= box[YMIN] && y < box[YMAX] &&
		z >= box[ZMIN] && z < box[ZMAX];
}

static int box_max_size(const box_t &box)
{
	return std::max(std::max(box[XMAX] - box[XMIN], box[YMAX] - box[YMIN]), box[ZMAX] - box[ZMIN]);
}

// helper function
// cache - source of sector and system data
// crd - coordinates of the origin point in the sector, l.y.
// originSector - coordinates of the sector containing the origin point
// targetSector - coordinates of the sector in which we are looking for the nearest system
// min_dist_sq - here we will accumulate the minimum distance
// best_sys - here we will accumulate the nearest system
static void try_sector_for_nearer_system(SectorCache::Slave *cache, const vector3f &crd, const SystemPath &originSector, const SystemPath &targetSector, float &min_dist_sq, SystemPath &best_sys)
{
	RefCountedPtr<Sector> ps = cache->GetCached(targetSector);
	vector3f base(
		targetSector.sectorX - originSector.sectorX,
		targetSector.sectorY - originSector.sectorY,
		targetSector.sectorZ - originSector.sectorZ);
	base *= Sector::SIZE;
	for (unsigned int i = 0; i < ps->m_systems.size(); i++) {
		Sector::System *ss = &ps->m_systems[i];
		vector3f dx = crd - (ss->GetPosition() + base);
		float dist = dx.LengthSqr();
		if (dist < min_dist_sq) {
			min_dist_sq = dist;
			best_sys = ss->GetPath();
		}
	}
}

// helper function
// return true, if at least one system is found
// cache - source of sector and system data
// sectorPos - coordinates of the sector containing the origin point
// crd - coordinates of the origin point in the sector, l.y.
// searchBox - we should search inside this box
// prevSearchBox - we shouldn't search inside this box (because already searched)
// min_dist_sq - here we will accumulate the minimum distance
// best_sys - here we will accumulate the nearest system
static bool search_nearest_in_box(SectorCache::Slave *cache, const SystemPath &sectorPos, const vector3f &crd, box_t &searchBox, box_t &prevSearchBox, float &min_dist_sq, SystemPath &best_sys)
{
	for (int x = searchBox[XMIN]; x < searchBox[XMAX]; ++x)
		for (int y = searchBox[YMIN]; y < searchBox[YMAX]; ++y)
			for (int z = searchBox[ZMIN]; z < searchBox[ZMAX]; ++z)
				if (!in_box(x, y, z, prevSearchBox)) // we are not looking where we have already looked
					try_sector_for_nearer_system(cache, crd, sectorPos, SystemPath(x, y, z), min_dist_sq, best_sys);
	// we will not search next time where already searched
	prevSearchBox = searchBox;
	return min_dist_sq != FLT_MAX;
}

// the function searches for the closest star to the given coordinates
// cache - source of sector and system data
// pos -  absolute coordinates of a point (in sectors, float)
// returns the found SystemPath, (system closest to the given position)
// if returned !SystemPath.IsSystemPath(), it means that nothing was found
static SystemPath nearest_system_to_pos(SectorCache::Slave *cache, const vector3f &pos)
{
	// maximum size in sectors of the search box
	const int MAX_SEARCH_BOX = 10;
	// extract the coordinates of the sector, and the coordinates of the position within the sector
	SystemPath sectorPos(floor(pos.x), floor(pos.y), floor(pos.z));
	vector3f crd(FFRAC(pos.x), FFRAC(pos.y), FFRAC(pos.z));
	crd *= Sector::SIZE; // convert to l.y.
	// directions of "growth" of the search box, if necessary
	box_t enlarge = { -1, 1, -1, 1, -1, 1 };
	// remember the initial sector coordinates and point coordinates as boxes with size 0, for ease of comparison
	box_t originBox = {
		sectorPos.sectorX, sectorPos.sectorX,
		sectorPos.sectorY, sectorPos.sectorY,
		sectorPos.sectorZ, sectorPos.sectorZ
	};
	std::array<float, SIDES> posBox = { crd.x, crd.x, crd.y, crd.y, crd.z, crd.z };
	// we will remember in which box we were already looked, so as not to repeat the search
	box_t prevSearchBox = originBox;
	// here we will accumulate the minimum distance
	float min_dist_sq = FLT_MAX;
	// here we will accumulate the nearest system (this will be the result of the function)
	SystemPath best_sys;

	// the first iteration of the search - only in the sector where the base point is located
	box_t searchBox = {
		sectorPos.sectorX, sectorPos.sectorX + 1,
		sectorPos.sectorY, sectorPos.sectorY + 1,
		sectorPos.sectorZ, sectorPos.sectorZ + 1
	};
	// scan sectors, and expand the search box until we find at least one nearest system
	while (!search_nearest_in_box(cache, sectorPos, crd, searchBox, prevSearchBox, min_dist_sq, best_sys)) {
		// expand the search box
		for (int side = 0; side < SIDES; ++side) {
			searchBox[side] += enlarge[side];
		}
		// could not find anything, within the maximum search range - we return an empty path
		if (box_max_size(searchBox) > MAX_SEARCH_BOX)
			return SystemPath();
	}

	// we found a system, but it may not be the closest one - let's check all the
	// edges of the search box, whether they are closer than the found star
	bool search_complete = true;
	for (int side = 0; side < SIDES; ++side) {
		float dist_to_edge = posBox[side] - (searchBox[side] - originBox[side]) * Sector::SIZE;
		if (dist_to_edge * dist_to_edge < min_dist_sq) {
			// the origin is closer to this edge of the search box than to the nearest found - we expand in this direction
			searchBox[side] += enlarge[side];
			search_complete = false;
		}
	}
	// if we enlarged the box, we will finally look for the closest system once again
	if (!search_complete)
		search_nearest_in_box(cache, sectorPos, crd, searchBox, prevSearchBox, min_dist_sq, best_sys);
	return best_sys;
}

REGISTER_INPUT_BINDING(SectorView)
{
	using namespace InputBindings;
	auto *mapView = input->GetBindingPage("MapControls");
	Input::BindingGroup *group;

#define BINDING_GROUP(n) group = mapView->GetBindingGroup(#n);
#define KEY_BINDING(name, k1, k2) \
	input->AddActionBinding(name, group, InputBindings::Action({ k1 }, { k2 }));
#define AXIS_BINDING(name, k1, k2) \
	input->AddAxisBinding(name, group, InputBindings::Axis({}, { k1 }, { k2 }));

	BINDING_GROUP(GeneralViewControls)
	KEY_BINDING("BindResetOrientationAndZoom", SDLK_t, 0)
	AXIS_BINDING("BindMapViewYaw", SDLK_KP_4, SDLK_KP_6)
	AXIS_BINDING("BindMapViewPitch", SDLK_KP_8, SDLK_KP_2)
	AXIS_BINDING("BindViewZoom", SDLK_KP_PLUS, SDLK_KP_MINUS)
	AXIS_BINDING("BindMapViewMoveUp", SDLK_r, SDLK_f)
	AXIS_BINDING("BindMapViewMoveLeft", SDLK_a, SDLK_d)
	AXIS_BINDING("BindMapViewMoveForward", SDLK_w, SDLK_s)

	BINDING_GROUP(SectorMapViewControls)
	KEY_BINDING("BindMapToggleSelectionFollowView", SDLK_RETURN, SDLK_KP_ENTER)
	KEY_BINDING("BindMapWarpToCurrentSystem", SDLK_c, 0)
	KEY_BINDING("BindMapWarpToSelectedSystem", SDLK_g, 0)

#undef KEY_BINDING
#undef AXIS_BINDING
#undef BINDING_GROUP
}

void SectorView::InputBinding::RegisterBindings()
{
	mapViewReset = AddAction("BindResetOrientationAndZoom");
	mapToggleSelectionFollowView = AddAction("BindMapToggleSelectionFollowView");
	mapWarpToCurrent = AddAction("BindMapWarpToCurrentSystem");
	mapWarpToSelected = AddAction("BindMapWarpToSelectedSystem");

	mapViewMoveForward = AddAxis("BindMapViewMoveForward");
	mapViewMoveLeft = AddAxis("BindMapViewMoveLeft");
	mapViewMoveUp = AddAxis("BindMapViewMoveUp");

	mapViewYaw = AddAxis("BindMapViewYaw");
	mapViewPitch = AddAxis("BindMapViewPitch");
	mapViewZoom = AddAxis("BindMapViewZoom");
}

SectorView::SectorView(Game *game) :
	PiGuiView("sector-view"),
	InputBindings(Pi::input),
	m_galaxy(game->GetGalaxy())
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

	m_automaticSystemSelection = true;
	m_detailBoxVisible = DETAILBOX_INFO;
	m_toggledFaction = false;

	InitObject();
}

SectorView::SectorView(const Json &jsonObj, Game *game) :
	PiGuiView("sector-view"),
	InputBindings(Pi::input),
	m_galaxy(game->GetGalaxy())
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

	// Note: INT_MAX != (int) ((float) INT_MAX)
	const float farPos = static_cast<float>(INT_MAX);
	m_secPosFar = vector3f(farPos, farPos, farPos);
	m_radiusFar = 0;
	m_cacheXMin = 0;
	m_cacheXMax = 0;
	m_cacheYMin = 0;
	m_cacheYMax = 0;
	m_cacheYMin = 0;
	m_cacheYMax = 0;

	m_sectorCache = m_galaxy->NewSectorSlaveCache();
	InputBindings.RegisterBindings();

	m_drawRouteLines = true; // where should this go?!
	m_route = std::vector<SystemPath>();
}

void SectorView::InitObject()
{
	// single keystroke handlers
	m_onToggleSelectionFollowView =
		InputBindings.mapToggleSelectionFollowView->onPressed.connect([&]() {
			m_automaticSystemSelection = !m_automaticSystemSelection;
		});
	m_onWarpToCurrent =
		InputBindings.mapWarpToCurrent->onPressed.connect([&]() {
			GotoSystem(m_current);
		});
	m_onWarpToSelected =
		InputBindings.mapWarpToSelected->onPressed.connect([&]() {
			GotoSystem(m_selected);
		});
	m_onViewReset =
		InputBindings.mapViewReset->onPressed.connect([&]() {
			while (m_rotZ < -180.0f)
				m_rotZ += 360.0f;
			while (m_rotZ > 180.0f)
				m_rotZ -= 360.0f;
			m_rotXMovingTo = m_rotXDefault;
			m_rotZMovingTo = m_rotZDefault;
			m_zoomMovingTo = m_zoomDefault;
		});

	m_onMouseWheelCon =
		Pi::input->onMouseWheel.connect(sigc::mem_fun(this, &SectorView::MouseWheel));

	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_secLineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_starVerts.reset(new Graphics::VertexArray(
		Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0, 500));

	m_renderer = Pi::renderer; //XXX pass cleanly to all views constructors!

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;

	Graphics::MaterialDescriptor bbMatDesc;
	bbMatDesc.effect = Graphics::EFFECT_SPHEREIMPOSTOR;
	m_starMaterial.Reset(m_renderer->CreateMaterial(bbMatDesc, rsd));

	rsd.depthWrite = false;
	rsd.cullMode = CULL_NONE;

	Graphics::MaterialDescriptor starPointDesc;
	starPointDesc.vertexColors = true;
	m_farStarsMat.Reset(m_renderer->CreateMaterial(starPointDesc, rsd));

	rsd.primitiveType = Graphics::LINE_SINGLE;

	Graphics::MaterialDescriptor lineDesc;
	lineDesc.effect = Graphics::EFFECT_VTXCOLOR;
	m_lineMat.Reset(m_renderer->CreateMaterial(lineDesc, rsd));
}

SectorView::~SectorView()
{
	m_onMouseWheelCon.disconnect();
	m_onToggleSelectionFollowView.disconnect();
	m_onWarpToCurrent.disconnect();
	m_onWarpToSelected.disconnect();
	m_onViewReset.disconnect();
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

	sectorViewObj["automatic_system_selection"] = m_automaticSystemSelection;
	sectorViewObj["detail_box_visible"] = m_detailBoxVisible;

	jsonObj["sector_view"] = sectorViewObj; // Add sector view object to supplied object.
}

void SectorView::Draw3D()
{
	PROFILE_SCOPED()

	m_lineVerts->Clear();
	m_secLineVerts->Clear();
	// m_clickableLabels->Clear();
	m_starVerts->Clear();

	if (m_zoomClamped <= FAR_THRESHOLD)
		m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 300.f);
	else
		m_renderer->SetPerspectiveProjection(40.f, m_renderer->GetDisplayAspect(), 1.f, 600.f);

	matrix4x4f modelview = matrix4x4f::Identity();

	m_renderer->ClearScreen();

	Graphics::Renderer::MatrixTicket ticket(m_renderer);

	// units are lightyears, my friend
	modelview.Translate(0.f, 0.f, -10.f - 10.f * m_zoom); // not zoomClamped, let us zoom out a bit beyond what we're drawing
	modelview.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	modelview.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);

	// sector drawing expects only the fractional part of the position,
	// and handles sector offset internally
	matrix4x4f sectorTrans = modelview;
	sectorTrans.Translate(-FFRAC(m_pos.x) * Sector::SIZE, -FFRAC(m_pos.y) * Sector::SIZE, -FFRAC(m_pos.z) * Sector::SIZE);
	m_renderer->SetTransform(sectorTrans);

	if (m_zoomClamped <= FAR_THRESHOLD)
		DrawNearSectors(sectorTrans);
	else
		DrawFarSectors(sectorTrans);

	modelview.Translate(-m_pos * Sector::SIZE);

	// calculate the player's location (it will be interpolated between systems during a hyperjump)
	vector3f playerPos;
	float currentStarSize;
	GetPlayerPosAndStarSize(playerPos, currentStarSize);

	// player location indicator
	// starting with the "modelview" matrix, see above, adding translation to playerPos
	matrix4x4f pTrans = modelview * matrix4x4f::Translation(playerPos);
	// prior to modelview transformation, rotate in the opposite direction so that the billboard is always facing the camera
	pTrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
	pTrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
	// move this disk 0.03 light years further so that it does not overlap the star, and selected indicator and hyperspace target indicator
	AddStarBillboard(pTrans, vector3f(0.f, 0.f, -0.03f), Color(0, 0, 204), 1.5f);

	m_renderer->SetTransform(matrix4x4f::Identity());

	//draw star billboards in one go
	m_renderer->SetAmbientColor(Color(30, 30, 30));
	m_renderer->DrawBuffer(m_starVerts.get(), m_starMaterial.Get());

	//draw sector legs in one go
	if (!m_lineVerts->IsEmpty()) {
		m_lines.SetData(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);
		m_lines.Draw(m_renderer, m_lineMat.Get());
	}

	if (!m_secLineVerts->IsEmpty()) {
		m_sectorlines.SetData(m_secLineVerts->GetNumVerts(), &m_secLineVerts->position[0], &m_secLineVerts->diffuse[0]);
		m_sectorlines.Draw(m_renderer, m_lineMat.Get());
	}

	// the next function assumes that m_renderer->(matrix4x4f::Identity()),
	// so we run this function before we touch m_renderer again
	if (m_drawRouteLines && !m_route.empty()) {
		if (m_setupRouteLines) {
			SetupRouteLines(playerPos);
		}
		DrawRouteLines(modelview);
	}
}

void SectorView::SetHyperspaceTarget(const SystemPath &path)
{
	m_hyperspaceTarget = path;
	onHyperspaceTargetChanged.emit();
}

void SectorView::ResetHyperspaceTarget()
{
	SystemPath old = m_hyperspaceTarget;
	m_hyperspaceTarget = Pi::game->GetSpace()->GetStarSystem()->GetStars()[0]->GetPath();

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
	m_posMovingTo.x = path.sectorX + p.x / Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y / Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z / Sector::SIZE;

	// for performance don't animate the travel if we're Far Zoomed
	if (m_zoomClamped > FAR_THRESHOLD) {
		m_pos = m_posMovingTo;
	}
}

bool SectorView::IsCenteredOn(const SystemPath &path)
{
	RefCountedPtr<Sector> ps = GetCached(path);
	const vector3f &p = ps->m_systems[path.systemIndex].GetPosition();
	vector3f diff = vector3f(
		fabs(m_pos.x - path.sectorX - p.x / Sector::SIZE),
		fabs(m_pos.y - path.sectorY - p.y / Sector::SIZE),
		fabs(m_pos.z - path.sectorZ - p.z / Sector::SIZE));
	return ((diff.x < 0.005f && diff.y < 0.005f && diff.z < 0.005f));
}

void SectorView::SetSelected(const SystemPath &path)
{
	if (path.IsBodyPath())
		m_selected = path;
	else if (path.IsSystemPath()) {
		RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(path);
		m_selected = CheckPathInRoute(system->GetStars()[0]->GetPath());
	}
}

void SectorView::SwitchToPath(const SystemPath &path)
{
	SetSelected(path);
	if (m_automaticSystemSelection)
		GotoSystem(path);
}

void SectorView::OnClickSystem(const SystemPath &path)
{
	SwitchToPath(path);
}

// check the route, maybe this system is there, then we'll take the path from the route
const SystemPath &SectorView::CheckPathInRoute(const SystemPath &path)
{
	for (const auto &p : m_route)
		if (p.IsSameSystem(path))
			return p;
	return path;
}

void SectorView::PutSystemLabels(RefCountedPtr<Sector> sec, const vector3f &origin, int drawRadius)
{
	PROFILE_SCOPED()

	Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator sys = sec->m_systems.begin(); sys != sec->m_systems.end(); ++sys, ++sysIdx) {
		// skip the system if it doesn't fall within the sphere we're viewing.
		if ((m_pos * Sector::SIZE - (*sys).GetFullPosition()).Length() > drawRadius) continue;

		// if the system is the current system or target we can't skip it
		bool can_skip = !sys->IsSameSystem(m_selected) && !sys->IsSameSystem(m_hyperspaceTarget) && !sys->IsSameSystem(m_current);

		// skip if we have no population and won't drawn uninhabited systems
		if (can_skip && (sys->GetPopulation() <= 0 && !m_drawUninhabitedLabels)) continue;

		// skip the system if it belongs to a Faction we've toggled off and we can skip it
		if (can_skip && m_hiddenFactions.find(sys->GetFaction()) != m_hiddenFactions.end()) continue;

		// determine if system in hyperjump range or not
		RefCountedPtr<const Sector> playerSec = GetCached(m_current);
		float dist = Sector::DistanceBetween(sec, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// skip if we're out of rangen and won't draw out of range systems systems
		if (can_skip && (!inRange && !m_drawOutRangeLabels)) continue;

		// place the label
		vector3d systemPos = vector3d((*sys).GetFullPosition() - origin);
		vector3d screenPos = Graphics::ProjectToScreen(m_renderer, systemPos);
		// reject back-projected labels (negative Z in clipspace is in front of the view plane)
		if (screenPos.z < 0.0f) {
			// work out the colour
			Color labelColor = sys->GetFaction()->AdjustedColour(sys->GetPopulation(), inRange);

			// get a system path to pass to the event handler when the label is licked
			SystemPath sysPath = SystemPath((*sys).sx, (*sys).sy, (*sys).sz, sysIdx);

			// label text
			std::string text = "";
			if (((inRange || m_drawOutRangeLabels) && (sys->GetPopulation() > 0 || m_drawUninhabitedLabels)) || !can_skip)
				text = sys->GetName();

			// setup the label;
			// FIXME: send this info to Lua for decoration
			// m_clickableLabels->Add(text, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), sysPath), screenPos.x, screenPos.y, labelColor);
		}
	}
}

void SectorView::PutFactionLabels(const vector3f &origin)
{
	PROFILE_SCOPED()
	/* FIXME: replace this with an imgui-based solution (push system data into Lua for decoration?)

	Gui::Screen::EnterOrtho();

	if (!m_material)
		m_material.Reset(m_renderer->CreateMaterial(Graphics::MaterialDescriptor()));

	static const Color labelBorder(13, 13, 31, 166);
	const auto renderState = Gui::Screen::alphaBlendState;

	for (auto it = m_visibleFactions.begin(); it != m_visibleFactions.end(); ++it) {
		if ((*it)->hasHomeworld && m_hiddenFactions.find((*it)) == m_hiddenFactions.end()) {
			Sector::System sys = GetCached((*it)->homeworld)->m_systems[(*it)->homeworld.systemIndex];
			if ((m_pos * Sector::SIZE - sys.GetFullPosition()).Length() > (m_zoomClamped / FAR_THRESHOLD) * OUTER_RADIUS) continue;

			vector3d pos;
			Gui::Screen::Project(vector3d(sys.GetFullPosition() - origin), pos);
			if (pos.z < 0.0f) { // reject back-projected stars

				std::string labelText = sys.GetName() + "\n" + (*it)->name;
				Color labelColor = (*it)->colour;
				float labelHeight = 0;
				float labelWidth = 0;

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
					labelColor.a = 204; // luminance is sometimes a bit overly
				m_clickableLabels->Add(labelText, sigc::bind(sigc::mem_fun(this, &SectorView::OnClickSystem), (*it)->homeworld), pos.x + MARGINLEFT, pos.y - (halfheight / 2.0) - 1.f, labelColor);
			}
		}
	}
	Gui::Screen::LeaveOrtho();
	*/
}

void SectorView::AddStarBillboard(const matrix4x4f &trans, const vector3f &pos, const Color &col, float size)
{
	const matrix3x3f rot = trans.GetOrient().Transpose();

	const vector3f offset = trans * pos;

	const vector3f rotv1 = rot * vector3f(size / 2.f, -size / 2.f, 0.0f);
	const vector3f rotv2 = rot * vector3f(size / 2.f, size / 2.f, 0.0f);

	Graphics::VertexArray &va = *m_starVerts;
	va.Add(offset - rotv1, col, vector2f(0.f, 0.f)); //top left
	va.Add(offset - rotv2, col, vector2f(0.f, 1.f)); //bottom left
	va.Add(offset + rotv2, col, vector2f(1.f, 0.f)); //top right

	va.Add(offset + rotv2, col, vector2f(1.f, 0.f)); //top right
	va.Add(offset - rotv2, col, vector2f(0.f, 1.f)); //bottom left
	va.Add(offset + rotv1, col, vector2f(1.f, 1.f)); //bottom right
}

void SectorView::DrawNearSectors(const matrix4x4f &modelview)
{
	PROFILE_SCOPED()
	m_visibleFactions.clear();

	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				DrawNearSector(int(floorf(m_pos.x)) + sx, int(floorf(m_pos.y)) + sy, int(floorf(m_pos.z)) + sz,
					modelview * matrix4x4f::Translation(Sector::SIZE * sx, Sector::SIZE * sy, Sector::SIZE * sz));
			}
		}
	}

	// ...then switch and do all the labels
	const vector3f secOrigin = vector3f(int(floorf(m_pos.x)), int(floorf(m_pos.y)), int(floorf(m_pos.z)));

	m_renderer->SetTransform(modelview);
	for (int sx = -DRAW_RAD; sx <= DRAW_RAD; sx++) {
		for (int sy = -DRAW_RAD; sy <= DRAW_RAD; sy++) {
			for (int sz = -DRAW_RAD; sz <= DRAW_RAD; sz++) {
				PutSystemLabels(GetCached(SystemPath(sx + secOrigin.x, sy + secOrigin.y, sz + secOrigin.z)), Sector::SIZE * secOrigin, Sector::SIZE * DRAW_RAD);
			}
		}
	}
}

bool SectorView::MoveRouteItemUp(const std::vector<SystemPath>::size_type element)
{
	if (element == 0 || element >= m_route.size()) return false;

	std::swap(m_route[element - 1], m_route[element]);

	m_setupRouteLines = true;
	return true;
}

bool SectorView::MoveRouteItemDown(const std::vector<SystemPath>::size_type element)
{
	if (element >= m_route.size() - 1) return false;

	std::swap(m_route[element + 1], m_route[element]);

	m_setupRouteLines = true;
	return true;
}

void SectorView::UpdateRouteItem(const std::vector<SystemPath>::size_type element, const SystemPath &path)
{
	m_route[element] = path;
	m_setupRouteLines = true;
}

void SectorView::AddToRoute(const SystemPath &path)
{
	m_route.push_back(path);
	m_setupRouteLines = true;
}

bool SectorView::RemoveRouteItem(const std::vector<SystemPath>::size_type element)
{
	if (element < m_route.size()) {
		m_route.erase(m_route.begin() + element);
		m_setupRouteLines = true;
		return true;
	} else {
		return false;
	}
}

void SectorView::ClearRoute()
{
	m_route.clear();
	m_setupRouteLines = true;
}

std::vector<SystemPath> SectorView::GetRoute()
{
	return m_route;
}

const std::string SectorView::AutoRoute(const SystemPath &start, const SystemPath &target, std::vector<SystemPath> &outRoute) const
{
	const RefCountedPtr<const Sector> start_sec = m_galaxy->GetSector(start);
	const RefCountedPtr<const Sector> target_sec = m_galaxy->GetSector(target);

	LuaRef try_hdrive = LuaObject<Player>::CallMethod<LuaRef>(Pi::player, "GetEquip", "engine", 1);
	if (try_hdrive.IsNil())
		return "NO_DRIVE";
	// Get the player's hyperdrive from Lua, later used to calculate the duration between systems
	const ScopedTable hyperdrive = ScopedTable(try_hdrive);
	// Cache max range so it doesn't get recalculated every time we call GetDuration
	const float max_range = hyperdrive.CallMethod<float>("GetMaximumRange", Pi::player);
	const float max_range_sqr = max_range * max_range;

	// use the square of the distance to avoid doing a sqrt for each sector
	const float distSqr = Sector::DistanceBetweenSqr(start_sec, start.systemIndex, target_sec, target.systemIndex) * 1.10;

	// the maximum distance that anything can be from the direct line between the start and target systems
	const float max_dist_from_straight_line = (Sector::SIZE * 3);

	// nodes[0] is always start
	std::vector<SystemPath> nodes;
	{
		// calculate an approximate initial number of nodes
		const float dist = sqrt(distSqr);
		// +2 for the sectors near start and target, *9 for the sectors immediately around the path... might want to be bigger but seems to work
		const size_t num_sectors_covered = (size_t(dist / Sector::SIZE) + 2) * 9;
		const size_t num_systems_per_sector = 6; // total guess
		nodes.reserve(num_sectors_covered * num_systems_per_sector);
	}
	nodes.push_back(start);

	const Sint32 minX = std::min(start.sectorX, target.sectorX) - 2, maxX = std::max(start.sectorX, target.sectorX) + 2;
	const Sint32 minY = std::min(start.sectorY, target.sectorY) - 2, maxY = std::max(start.sectorY, target.sectorY) + 2;
	const Sint32 minZ = std::min(start.sectorZ, target.sectorZ) - 2, maxZ = std::max(start.sectorZ, target.sectorZ) + 2;
	const vector3f start_pos = start_sec->m_systems[start.systemIndex].GetFullPosition();
	const vector3f target_pos = target_sec->m_systems[target.systemIndex].GetFullPosition();

	// go sector by sector for the minimum cube of sectors and add systems
	// if they are within 110% of dist of both start and target
	size_t secLineToFar = 0u;
	for (Sint32 sx = minX; sx <= maxX; sx++) {
		for (Sint32 sy = minY; sy <= maxY; sy++) {
			for (Sint32 sz = minZ; sz < maxZ; sz++) {
				const SystemPath sec_path = SystemPath(sx, sy, sz);

				// early out here if the sector is too far from the direct line
				vector3f sec_centre(Sector::SIZE * vector3f(float(sx) + 0.5f, float(sy) + 0.5f, float(sz) + 0.5f));
				const float secLineDist = MathUtil::DistanceFromLine(start_pos, target_pos, sec_centre);
				// this test is deliberately conservative to over include sectors, a better test might use the nearest corner of an AABB to the line - Math hard, Andy tired
				if ((secLineDist - Sector::SIZE) > max_dist_from_straight_line) {
					++secLineToFar;
					continue;
				}

				// GetSector is very expensive if it's not in the cache
				RefCountedPtr<const Sector> sec = m_galaxy->GetSector(sec_path);
				for (std::vector<Sector::System>::size_type s = 0; s < sec->m_systems.size(); s++) {
					if (start.IsSameSystem(sec->m_systems[s].GetPath()))
						continue; // start is already nodes[0]

					const float lineDist = MathUtil::DistanceFromLine(start_pos, target_pos, sec->m_systems[s].GetFullPosition());

					if (Sector::DistanceBetweenSqr(start_sec, start.systemIndex, sec, sec->m_systems[s].idx) <= distSqr &&
						Sector::DistanceBetweenSqr(target_sec, target.systemIndex, sec, sec->m_systems[s].idx) <= distSqr &&
						lineDist < max_dist_from_straight_line) {
						nodes.push_back(sec->m_systems[s].GetPath());
					}
				}
			}
		}
	}
	Output("SectorView::AutoRoute, nodes to search = %lu, earlied out sector distance from line: %lu times.\n", nodes.size(), secLineToFar);

	// setup inital values and set everything as unvisited
	std::vector<float> path_dist;							   // distance from source to node
	std::vector<std::vector<SystemPath>::size_type> path_prev; // previous node in optimal path
	std::unordered_set<std::vector<SystemPath>::size_type> unvisited;
	// we know how big these need to be, to reserve space up front
	path_dist.reserve(nodes.size());
	path_prev.reserve(nodes.size());
	unvisited.reserve(nodes.size());
	// initialise them
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
			if ((SystemPath::SectorDistanceSqr(closest, v) * Sector::SIZE) > max_range_sqr) {
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
			outRoute.push_back(m_galaxy->GetStarSystem(nodes[u])->GetStars()[0]->GetPath());
			u = path_prev[u];
		}
		std::reverse(std::begin(outRoute), std::end(outRoute));
		return "OKAY";
	}
	return "NO_VALID_ROUTE";
}

void SectorView::DrawRouteLines(const matrix4x4f &trans)
{
	if (m_route.empty())
		return;

	m_renderer->SetTransform(trans);
	m_routeLines.Draw(m_renderer, m_lineMat.Get());
}

void SectorView::SetupRouteLines(const vector3f &playerAbsPos)
{
	assert(!m_route.empty());
	assert(m_setupRouteLines);
	m_setupRouteLines = false;

	std::unique_ptr<Graphics::VertexArray> verts;
	verts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, m_route.size() * 2));
	verts->Clear();

	vector3f startPos = playerAbsPos;
	for (std::vector<SystemPath>::size_type i = 0; i < m_route.size(); ++i) {
		RefCountedPtr<const Sector> jumpSec = m_galaxy->GetSector(m_route[i]);
		const Sector::System &jumpSecSys = jumpSec->m_systems[m_route[i].systemIndex];
		const vector3f jumpAbsPos = Sector::SIZE * vector3f(float(jumpSec->sx), float(jumpSec->sy), float(jumpSec->sz)) + jumpSecSys.GetPosition();

		if (i > 0) {
			RefCountedPtr<const Sector> prevSec = m_galaxy->GetSector(m_route[i - 1]);
			const Sector::System &prevSecSys = prevSec->m_systems[m_route[i - 1].systemIndex];
			const vector3f prevAbsPos = Sector::SIZE * vector3f(float(prevSec->sx), float(prevSec->sy), float(prevSec->sz)) + prevSecSys.GetPosition();
			startPos = prevAbsPos;
		}

		verts->Add(startPos, Color(20, 20, 0, 127));
		verts->Add(jumpAbsPos, Color(255, 255, 0, 255));
	}

	m_routeLines.SetData(verts->GetNumVerts(), &verts->position[0], &verts->diffuse[0]);
}

void SectorView::GetPlayerPosAndStarSize(vector3f &playerPosOut, float &currentStarSizeOut)
{
	// calculate the player's location (it will be interpolated between systems during a hyperjump)
	const Sector::System currentSystem = GetCached(m_current)->m_systems[m_current.systemIndex];
	playerPosOut = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) + currentSystem.GetPosition();
	currentStarSizeOut = StarSystem::starScale[currentSystem.GetStarType(0)];
	if (!m_inSystem) {
		// we are in hyperspace, interpolate
		// m_current has been set to the destination path
		// destination system data:
		const SystemPath dstPath = Pi::game->GetHyperspaceDest();
		const Sector::System dstSystem = GetCached(dstPath)->m_systems[dstPath.systemIndex];
		const vector3f dstPos = Sector::SIZE * vector3f(float(dstPath.sectorX), float(dstPath.sectorY), float(dstPath.sectorZ)) + dstSystem.GetPosition();
		// jumpProcess: 0.0f (at source) .. 1.0f (at destination)
		// smoothing speed at the ends with cubic interpolation
		const float jumpProcess = AnimationCurves::InOutCubicEasing(Pi::game->GetHyperspaceProgress() / Pi::game->GetHyperspaceDuration());
		// player location indicator's size of the star, on which it is drawn
		// so we interpolate it too
		const float dstStarSize = StarSystem::starScale[dstSystem.GetStarType(0)];
		// interpolating player location indicator's position and size
		playerPosOut = playerPosOut.Lerp(dstPos, jumpProcess);
		currentStarSizeOut = MathUtil::Lerp(currentStarSizeOut, dstStarSize, jumpProcess);
	}
}

void SectorView::DrawNearSector(const int sx, const int sy, const int sz, const matrix4x4f &trans)
{
	PROFILE_SCOPED()
	m_renderer->SetTransform(trans);
	RefCountedPtr<Sector> ps = GetCached(SystemPath(sx, sy, sz));

	const int cz = int(floor(m_pos.z + 0.5f));

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

		m_secLineVerts->Add(vts[0], darkgreen); // line segment 1
		m_secLineVerts->Add(vts[1], darkgreen);
		m_secLineVerts->Add(vts[1], darkgreen); // line segment 2
		m_secLineVerts->Add(vts[2], darkgreen);
		m_secLineVerts->Add(vts[2], darkgreen); // line segment 3
		m_secLineVerts->Add(vts[3], darkgreen);
		m_secLineVerts->Add(vts[3], darkgreen); // line segment 4
		m_secLineVerts->Add(vts[0], darkgreen);
	}

	const size_t numLineVerts = ps->m_systems.size() * 8;
	m_lineVerts->position.reserve(numLineVerts);
	m_lineVerts->diffuse.reserve(numLineVerts);

	Uint32 sysIdx = 0;
	for (std::vector<Sector::System>::iterator i = ps->m_systems.begin(); i != ps->m_systems.end(); ++i, ++sysIdx) {
		// calculate where the system is in relation the centre of the view...
		const vector3f sysAbsPos = Sector::SIZE * vector3f(float(sx), float(sy), float(sz)) + i->GetPosition();
		const vector3f toCentreOfView = m_pos * Sector::SIZE - sysAbsPos;

		// ...and skip the system if it doesn't fall within the sphere we're viewing.
		if (toCentreOfView.Length() > OUTER_RADIUS) continue;

		const bool bIsCurrentSystem = i->IsSameSystem(m_current);
		const bool bInRoute = std::find_if(m_route.begin(), m_route.end(), [i](const SystemPath &a) -> bool { return i->IsSameSystem(a); }) != m_route.end();

		// if the system is the current system or target we can't skip it
		bool can_skip = !i->IsSameSystem(m_selected) && !i->IsSameSystem(m_hyperspaceTarget) && !bIsCurrentSystem && !bInRoute;

		// if the system belongs to a faction we've chosen to temporarily hide
		// then skip it if we can
		m_visibleFactions.insert(i->GetFaction());
		if (can_skip && m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end()) continue;

		// determine if system in hyperjump range or not
		RefCountedPtr<const Sector> playerSec = GetCached(m_current);
		float dist = Sector::DistanceBetween(ps, sysIdx, playerSec, m_current.systemIndex);
		bool inRange = dist <= m_playerHyperspaceRange;

		// don't worry about looking for inhabited systems if they're
		// unexplored (same calculation as in StarSystem.cpp) or we've
		// already retrieved their population.
		if (i->GetPopulation() < 0 && isqrt(1 + sx * sx + sy * sy + sz * sz) <= 90) {

			// only do this once we've pretty much stopped moving.
			vector3f diff = vector3f(
				fabs(m_posMovingTo.x - m_pos.x),
				fabs(m_posMovingTo.y - m_pos.y),
				fabs(m_posMovingTo.z - m_pos.z));

			// Ideally, since this takes so f'ing long, it wants to be done as a threaded job but haven't written that yet.
			if ((diff.x < 0.001f && diff.y < 0.001f && diff.z < 0.001f)) {
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
				z = z + abs(cz - sz) * Sector::SIZE;
			else
				z = z - abs(cz - sz) * Sector::SIZE;
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
			if (m_selected != m_current && !bInRoute) {
				const vector3f playerAbsPos = Sector::SIZE * vector3f(float(m_current.sectorX), float(m_current.sectorY), float(m_current.sectorZ)) +
					GetCached(m_current)->m_systems[m_current.systemIndex].GetPosition();

				m_lineVerts->Add(systrans * vector3f(0.f, 0.f, 0.f), Color::BLANK);
				m_lineVerts->Add(systrans * (playerAbsPos - sysAbsPos), Color::WHITE);
			}

			if (m_route.size() > 0) {
				const SystemPath &routeBack = m_route.back();
				if (!bInRoute) {
					RefCountedPtr<Sector> backSec = GetCached(routeBack);
					const vector3f hyperAbsPos =
						Sector::SIZE * vector3f(routeBack.sectorX, routeBack.sectorY, routeBack.sectorZ) + backSec->m_systems[routeBack.systemIndex].GetPosition();
					if (m_selected != m_current) {
						m_lineVerts->Add(systrans * vector3f(0.f, 0.f, 0.f), Color::BLANK);
						m_lineVerts->Add(systrans * (hyperAbsPos - sysAbsPos), Color::WHITE);
					}
				}
			}
		}

		// draw star blob itself
		systrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
		systrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
		systrans.Scale((StarSystem::starScale[(*i).GetStarType(0)]));
		m_renderer->SetTransform(systrans);

		const Uint8 *col = StarSystem::starColors[(*i).GetStarType(0)];
		AddStarBillboard(systrans, vector3f(0.f), Color(col[0], col[1], col[2], 255), 0.5f);

		// selected indicator
		if (i->IsSameSystem(m_selected)) {
			// move this disk 0.01 light years further so that it does not overlap the star
			AddStarBillboard(systrans, vector3f(0.f, 0.f, -0.01), Color(0, 204, 0), 1.0);
			// m_disk->SetColor(Color(0, 204, 0));
			// m_renderer->SetTransform(matrix4x4f::Translation((systrans * vector3f(.0f)).NormalizedSafe() * 0.01) * systrans * matrix4x4f::ScaleMatrix(2.f));
			// m_disk->Draw(m_renderer);
		}
		// hyperspace target indicator (if different from selection)
		if (i->IsSameSystem(m_hyperspaceTarget) && m_hyperspaceTarget != m_selected && (!m_inSystem || m_hyperspaceTarget != m_current)) {
			// move this disk 0.02 light years further so that it does not overlap the star, and selected indicator
			AddStarBillboard(systrans, vector3f(0.f, 0.f, -0.02), Color(77, 77, 77), 1.0);
			// m_disk->SetColor(Color(77, 77, 77));
			// m_renderer->SetTransform(matrix4x4f::Translation((systrans * vector3f(.0f)).NormalizedSafe() * 0.02) * systrans * matrix4x4f::ScaleMatrix(2.f));
			// m_disk->Draw(m_renderer);
		}
		// hyperspace range sphere
		if (bIsCurrentSystem && m_jumpSphere && m_playerHyperspaceRange > 0.0f) {
			const matrix4x4f sphTrans = trans * matrix4x4f::Translation(i->GetPosition().x, i->GetPosition().y, i->GetPosition().z);
			m_renderer->SetTransform(sphTrans * matrix4x4f::ScaleMatrix(m_playerHyperspaceRange));
			m_jumpSphere->Draw(m_renderer);
		}
	}
}

void SectorView::DrawFarSectors(const matrix4x4f &modelview)
{
	PROFILE_SCOPED()
	int buildRadius = ceilf((m_zoomClamped / FAR_THRESHOLD) * 3);
	if (buildRadius <= DRAW_RAD) buildRadius = DRAW_RAD;

	const vector3f secOrigin = vector3f(int(floorf(m_pos.x)), int(floorf(m_pos.y)), int(floorf(m_pos.z)));

	// build vertex and colour arrays for all the stars we want to see, if we don't already have them
	if (m_toggledFaction || buildRadius != m_radiusFar || !secOrigin.ExactlyEqual(m_secPosFar)) {
		m_farstars.clear();
		m_farstarsColor.clear();
		m_visibleFactions.clear();

		for (int sx = secOrigin.x - buildRadius; sx <= secOrigin.x + buildRadius; sx++) {
			for (int sy = secOrigin.y - buildRadius; sy <= secOrigin.y + buildRadius; sy++) {
				for (int sz = secOrigin.z - buildRadius; sz <= secOrigin.z + buildRadius; sz++) {
					if ((vector3f(sx, sy, sz) - secOrigin).Length() <= buildRadius) {
						BuildFarSector(GetCached(SystemPath(sx, sy, sz)), Sector::SIZE * secOrigin, m_farstars, m_farstarsColor);
					}
				}
			}
		}

		m_secPosFar = secOrigin;
		m_radiusFar = buildRadius;
		m_toggledFaction = false;
	}

	// always draw the stars, slightly altering their size for different different resolutions, so they still look okay
	if (m_farstars.size() > 0) {
		m_farstarsPoints.SetData(m_renderer, m_farstars.size(), &m_farstars[0], &m_farstarsColor[0], modelview, 0.25f * (Graphics::GetScreenHeight() / 720.f));
		m_farstarsPoints.Draw(m_renderer, m_farStarsMat.Get());
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
		if ((m_pos * Sector::SIZE - (*i).GetFullPosition()).Length() > (m_zoomClamped / FAR_THRESHOLD) * OUTER_RADIUS) continue;

		if (!i->IsExplored()) {
			points.push_back((*i).GetFullPosition() - origin);
			colors.push_back({ 100, 100, 100, 155 }); // flat gray for unexplored systems
			continue;
		}

		// if the system belongs to a faction we've chosen to hide also skip it, if it's not selectd in some way
		m_visibleFactions.insert(i->GetFaction());
		if (m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end() && !i->IsSameSystem(m_selected) && !i->IsSameSystem(m_hyperspaceTarget) && !i->IsSameSystem(m_current)) continue;

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
	m_renderer->SetViewport({ 0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight() });
	Pi::input->AddInputFrame(&InputBindings);
	Update();
}

void SectorView::OnSwitchFrom()
{
	Pi::input->RemoveInputFrame(&InputBindings);
}

void SectorView::Update()
{
	PROFILE_SCOPED()
	SystemPath last_current = m_current;

	if (Pi::game->IsNormalSpace()) {
		m_inSystem = true;
		m_current = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	} else {
		m_inSystem = false;
		m_current = Pi::game->GetHyperspaceSource();
	}

	const float frameTime = Pi::GetFrameTime();

	matrix4x4f rot = matrix4x4f::Identity();
	rot.RotateX(DEG2RAD(-m_rotX));
	rot.RotateZ(DEG2RAD(-m_rotZ));

	matrix3x3f shiftRot = matrix3x3f::Rotate(DEG2RAD(m_rotZ), { 0, 0, 1 });

	const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
	float move = moveSpeed * frameTime * m_zoomClamped;
	vector3f shift(0.0f);
	if (m_manualMove && (m_pos - m_posMovingTo).LengthSqr() < 0.001f)
		m_manualMove = false; // used so that when you click on a system and move there, passing systems are not selected
	if (InputBindings.mapViewMoveLeft->IsActive()) {
		shift.x -= move * InputBindings.mapViewMoveLeft->GetValue();
		m_manualMove = true;
	}
	if (InputBindings.mapViewMoveForward->IsActive()) {
		shift.y += move * InputBindings.mapViewMoveForward->GetValue();
		m_manualMove = true;
	}
	if (InputBindings.mapViewMoveUp->IsActive()) {
		shift.z += move * InputBindings.mapViewMoveUp->GetValue();
		m_manualMove = true;
	}
	m_posMovingTo += shift * shiftRot;

	if (InputBindings.mapViewZoom->IsActive()) m_zoomMovingTo -= move * InputBindings.mapViewZoom->GetValue();
	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);

	if (InputBindings.mapViewYaw->IsActive()) m_rotZMovingTo += 0.5f * moveSpeed * InputBindings.mapViewYaw->GetValue();
	if (InputBindings.mapViewPitch->IsActive()) m_rotXMovingTo += 0.5f * moveSpeed * InputBindings.mapViewPitch->GetValue();

	// to capture mouse when button was pressed and release when released
	if (Pi::input->MouseButtonState(SDL_BUTTON_MIDDLE) != m_rotateWithMouseButton) {
		m_rotateWithMouseButton = !m_rotateWithMouseButton;
		Pi::input->SetCapturingMouse(m_rotateWithMouseButton);
	}

	if (m_rotateWithMouseButton || m_rotateView) {
		int motion[2];
		Pi::input->GetMouseMotion(motion);
		m_rotXMovingTo += 0.2f * float(motion[1]);
		m_rotZMovingTo += 0.2f * float(motion[0]);
	} else if (m_zoomView) {
		Pi::input->SetCapturingMouse(true);
		int motion[2];
		Pi::input->GetMouseMotion(motion);
		m_zoomMovingTo += ZOOM_SPEED * 0.002f * motion[1];
	}

	m_rotXMovingTo = Clamp(m_rotXMovingTo, -170.0f, -10.0f);

	{
		vector3f diffPos = m_posMovingTo - m_pos;
		vector3f travelPos = diffPos * 10.0f * frameTime;
		if (travelPos.Length() > diffPos.Length())
			m_pos = m_posMovingTo;
		else
			m_pos = m_pos + travelPos;

		float diffX = m_rotXMovingTo - m_rotX;
		float travelX = diffX * 10.0f * frameTime;
		if (fabs(travelX) > fabs(diffX))
			m_rotX = m_rotXMovingTo;
		else
			m_rotX = m_rotX + travelX;

		float diffZ = m_rotZMovingTo - m_rotZ;
		float travelZ = diffZ * 10.0f * frameTime;
		if (fabs(travelZ) > fabs(diffZ))
			m_rotZ = m_rotZMovingTo;
		else
			m_rotZ = m_rotZ + travelZ;

		float diffZoom = m_zoomMovingTo - m_zoom;
		float travelZoom = diffZoom * ZOOM_SPEED * frameTime;
		if (fabs(travelZoom) > fabs(diffZoom))
			m_zoom = m_zoomMovingTo;
		else
			m_zoom = m_zoom + travelZoom;
		m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);
	}

	if (m_automaticSystemSelection && m_manualMove) {
		SystemPath new_selected = nearest_system_to_pos(m_sectorCache.Get(), m_pos);
		if (new_selected.IsSystemPath() && !m_selected.IsSameSystem(new_selected)) {
			RefCountedPtr<StarSystem> system = m_galaxy->GetStarSystem(new_selected);
			SetSelected(CheckPathInRoute(system->GetStars()[0]->GetPath()));
		}
	}

	ShrinkCache();

	m_playerHyperspaceRange = LuaObject<Player>::CallMethod<float>(Pi::player, "GetHyperspaceRange");

	if (!m_jumpSphere) {
		Graphics::RenderStateDesc rsd;
		rsd.blendMode = Graphics::BLEND_ALPHA;
		rsd.depthTest = false;
		rsd.depthWrite = false;
		rsd.cullMode = Graphics::CULL_NONE;

		Graphics::MaterialDescriptor matdesc;
		matdesc.effect = EFFECT_FRESNEL_SPHERE;
		m_fresnelMat.Reset(m_renderer->CreateMaterial(matdesc, rsd));
		m_fresnelMat->diffuse = Color::WHITE;
		m_jumpSphere.reset(new Graphics::Drawables::Sphere3D(m_renderer, m_fresnelMat, 4, 1.0f));
	}
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
	const int drawRadius = (m_zoomClamped <= FAR_THRESHOLD) ? DRAW_RAD : ceilf((m_zoomClamped / FAR_THRESHOLD) * DRAW_RAD);

	const int xmin = int(floorf(m_pos.x)) - drawRadius;
	const int xmax = int(floorf(m_pos.x)) + drawRadius;
	const int ymin = int(floorf(m_pos.y)) - drawRadius;
	const int ymax = int(floorf(m_pos.y)) + drawRadius;
	const int zmin = int(floorf(m_pos.z)) - drawRadius;
	const int zmax = int(floorf(m_pos.z)) + drawRadius;

	// XXX don't clear the current/selected/target sectors

	if (xmin != m_cacheXMin || xmax != m_cacheXMax || ymin != m_cacheYMin || ymax != m_cacheYMax || zmin != m_cacheZMin || zmax != m_cacheZMax) {
		auto iter = m_sectorCache->Begin();
		while (iter != m_sectorCache->End()) {
			RefCountedPtr<Sector> s = iter->second;
			//check_point_in_box
			if (!s->WithinBox(xmin, xmax, ymin, ymax, zmin, zmax)) {
				m_sectorCache->Erase(iter++);
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

double SectorView::GetZoomLevel() const
{
	return ((m_zoomClamped / FAR_THRESHOLD) * (OUTER_RADIUS)) + 0.5 * Sector::SIZE;
}

void SectorView::ZoomIn()
{
	const float frameTime = Pi::GetFrameTime();
	const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
	float move = moveSpeed * frameTime;
	m_zoomMovingTo -= move;
	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);
}

void SectorView::ZoomOut()
{
	const float frameTime = Pi::GetFrameTime();
	const float moveSpeed = Pi::GetMoveSpeedShiftModifier();
	float move = moveSpeed * frameTime;
	m_zoomMovingTo += move;
	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);
}

vector3f SectorView::GetCenterSector()
{
	return m_pos;
}

double SectorView::GetCenterDistance()
{
	if (m_inSystem) {
		vector3f dv = vector3f(floorf(m_pos.x) - m_current.sectorX, floorf(m_pos.y) - m_current.sectorY, floorf(m_pos.z) - m_current.sectorZ) * Sector::SIZE;
		return dv.Length();
	} else {
		return 0.0;
	}
}

std::vector<SystemPath> SectorView::GetNearbyStarSystemsByName(std::string pattern)
{
	std::vector<SystemPath> result;
	for (auto i = m_sectorCache->Begin(); i != m_sectorCache->End(); ++i) {
		for (unsigned int systemIndex = 0; systemIndex < (*i).second->m_systems.size(); systemIndex++) {
			const Sector::System *ss = &((*i).second->m_systems[systemIndex]);

			// compare with the start of the current system
			if (strncasecmp(pattern.c_str(), ss->GetName().c_str(), pattern.size()) == 0
				// look for the pattern term somewhere within the current system
				|| pi_strcasestr(ss->GetName().c_str(), pattern.c_str())) {
				SystemPath match((*i).first);
				match.systemIndex = systemIndex;
				result.push_back(match);
			}
			// now also check other names of this system, if there are any
			for (const std::string &other_name : ss->GetOtherNames()) {
				if (strncasecmp(pattern.c_str(), other_name.c_str(), pattern.size()) == 0
					// look for the pattern term somewhere within the current system
					|| pi_strcasestr(other_name.c_str(), pattern.c_str())) {
					SystemPath match((*i).first);
					match.systemIndex = systemIndex;
					result.push_back(match);
				}
			}
		}
	}
	return result;
}

void SectorView::SetFactionVisible(const Faction *faction, bool visible)
{
	if (visible)
		m_hiddenFactions.erase(faction);
	else
		m_hiddenFactions.insert(faction);
	m_toggledFaction = true;
}

void SectorView::SetZoomMode(bool enable)
{
	if (enable != m_zoomView) {
		Pi::input->SetCapturingMouse(enable);
		m_zoomView = enable;
		if (m_zoomView) m_rotateView = false;
	}
}

void SectorView::SetRotateMode(bool enable)
{
	if (enable != m_rotateView) {
		Pi::input->SetCapturingMouse(enable);
		m_rotateView = enable;
		if (m_rotateView) m_zoomView = false;
	}
}

void SectorView::ResetView()
{
	SwitchToPath(m_current);
	m_rotXMovingTo = m_rotXDefault;
	m_rotZMovingTo = m_rotZDefault;
	m_zoomMovingTo = m_zoomDefault;
}
