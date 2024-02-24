// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SectorMap.h"

#include "GameSaveError.h"
#include "Input.h"
#include "Json.h"
#include "MathUtil.h"
#include "core/StringUtils.h"
#include "pigui/PiGuiRenderer.h"
#include "galaxy/Sector.h"
#include "galaxy/Galaxy.h"
#include "GameConfig.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"
#include "pigui/PiGui.h"

#include <float.h>
#include <array>
#include <cmath>
#include <sstream>

using namespace Graphics;

static const int DRAW_RAD = 5;
#define INNER_RADIUS (Sector::SIZE * 1.5f)
#define OUTER_RADIUS (Sector::SIZE * float(DRAW_RAD))
static const float FAR_THRESHOLD = 7.5f;
static const float FAR_LIMIT = 36.f;
static const float FAR_MAX = 46.f;

class SectorMap::Label {
public:
	virtual ~Label() {}
	// check if the cursor has hit the label, and if so, also save the boxes to select the text
	// should run inside imgui context
	virtual bool Hovered(const ImVec2 &p) = 0;
	virtual void Draw(ImDrawList &dl, bool hovered = false) = 0;
	virtual void OnClick() = 0;
	ImVec2 GetPos() { return ImVec2{ pos.x, pos.y }; }
	float Depth() const { return pos.z; }
	Label(Labels &host, const vector3f &pos, ImU32 color) :
		pos(pos), color(color), host(host){};

protected:
	vector3f pos;
	ImU32 color;
	Labels &host;
};

// the Label type must be complete for this constructor to work, because we
// have a unique_ptr<Label> here
SectorMap::Labels::Labels(SectorMap &map) :
	map(map) {}

// helper functions to detect when the cursor hits the label
static bool inbox(ImRect b, ImVec2 p)
{
	return p.x > b.Min.x && p.x < b.Max.x && p.y > b.Min.y && p.y < b.Max.y;
}

static bool incircle(ImVec2 c, float r, ImVec2 p)
{
	return (p.x - c.x) * (p.x - c.x) + (p.y - c.y) * (p.y - c.y) < r * r;
}

static bool indiamond(ImVec2 c, float r, ImVec2 p)
{
	float dx = p.x - c.x;
	float dy1 = p.y - (c.y - r);
	float dy2 = p.y - (c.y + r);
	return dy1 > dx && dy1 > -dx && dy2 < dx && dy2 < -dx;
}

class SectorMap::StarLabel : public SectorMap::Label {

	std::string name;
	ImVec2 namePos;
	// screen radius of the star
	float radius;
	// the height of the text of each label is different
	float textHeight;
	float scaledGap;
	SystemPath path;

	//  how much we will increase the original font so that it is less blurry for heavily enlarged labels
	static constexpr float fontScale = 5.f;

public:
	StarLabel(Labels &host, const vector3f &pos, const Color &color_, const std::string &name, const float radius, const SystemPath &path) :
		Label(host, pos, 0),
		name(name),
		radius(radius),
		path(path)
	{
		ImFont *font = host.starLabelFont;
		// we want to shade more distant labels, and make the closer ones transparent
		const float far = -0.001;
		// at a given depth, we see a label with a nominal font size
		const float mid = -0.03;
		const float near = -0.08;
		// the font may not exist in the very first frame
		const float fsize = font ? font->FontSize / fontScale : 15.f;
		textHeight = fsize / mid * pos.z;
		scaledGap = host.gap / mid * pos.z;
		const int alpha = pos.z > near ? 255 : std::max(int(near / pos.z * 255), 0);
		const float shad = pos.z < mid ? 1.0f : std::max((far - pos.z) / (far - mid), .0f);
		namePos = ImVec2(pos.x + radius + scaledGap + scaledGap, pos.y - textHeight / 2);
		color = IM_COL32(color_.r * shad, color_.g * shad, color_.b * shad, alpha);
	};

	bool Hovered(const ImVec2 &p) override
	{
		// something we can weed out right away
		if (p.x < pos.x - radius) return false;
		if (p.y < std::min(pos.y - radius, namePos.y)) return false;
		if (p.y > std::max(pos.y + radius, namePos.y + textHeight)) return false;
		ImFont *font = host.starLabelFont;
		ImGui::PushFont(font);
		ImVec2 ts = ImGui::CalcTextSize(name.c_str());
		ImGui::PopFont();
		// scale the size
		ts.x = textHeight / ts.y * ts.x;
		ts.y = textHeight;
		// sift out by last dimension
		if (p.x > namePos.x + ts.x + scaledGap) return false;
		// so, the cursor is somewhere in the dimensions of this label
		if (incircle(ImVec2(pos.x, pos.y), radius, p) ||
			inbox(ImRect(namePos.x - scaledGap, namePos.y - scaledGap, namePos.x + ts.x + scaledGap, namePos.y + font->FontSize + scaledGap), p)) {
			// so the cursor really covers this label
			// save the boxes for further highlighting the label
			host.starLabelHoverArea = ImRect(namePos.x - scaledGap, namePos.y - scaledGap, namePos.x + ts.x + scaledGap, namePos.y + ts.y + scaledGap);
			return true;
		} else
			return false;
	}

	void Draw(ImDrawList &dl, bool hovered = false) override
	{
		if (hovered) {
			dl.AddCircleFilled(ImVec2(pos.x, pos.y), radius + scaledGap, host.highlightColor);
			auto &box = host.starLabelHoverArea;
			dl.AddRectFilled(box.Min, box.Max, host.shadeColor, scaledGap);
		}
		ImFont *font = host.starLabelFont;
		ImGui::PushFont(font);
		dl.AddText(font, textHeight, namePos, color, name.c_str());
		ImGui::PopFont();
	}

	void OnClick() override
	{
		host.map.OnClickLabel(path);
	}

	static void InitFonts(Labels &host, ImDrawList &dl)
	{
		ImFont *&font = host.starLabelFont;
		font = host.map.GetContext().pigui->GetFont(host.fontName, host.fontSize * fontScale);
		if (!font) font = ImGui::GetFont();
		dl.PushTextureID(font->ContainerAtlas->TexID);
	}
};

class SectorMap::FactionLabel : public SectorMap::Label {

	// how much the font of the name of the faction is smaller than the font of the name of the homeworld
	constexpr static float nameScale = .8f;
	std::string home;
	ImVec2 homePos;
	std::string name;
	ImVec2 namePos;
	// half the width of the rhombus
	float radius;
	SystemPath path;

public:
	FactionLabel(Labels &host, const vector3f &pos, const Color &color, const std::string &home, const std::string &name, const float radius, const SystemPath &path) :
		Label(host, pos, IM_COL32(color.r, color.g, color.b, color.a)),
		home(home),
		homePos(pos.x + radius + host.gap, pos.y - host.fontSize - host.gap),
		name(name),
		namePos(pos.x + radius + host.gap, pos.y + host.gap),
		radius(radius),
		path(path){};

	bool Hovered(const ImVec2 &p) override
	{
		// something we can weed out right away
		auto &gap = host.gap;
		if (p.x < pos.x - radius) return false;
		if (p.y < std::min(pos.y - radius, homePos.y - gap)) return false;
		if (p.y > std::max(pos.y + radius, namePos.y + host.factionNameFont->FontSize * nameScale + gap)) return false;

		ImGui::PushFont(host.factionHomeFont);
		ImVec2 ts1 = ImGui::CalcTextSize(home.c_str());
		ImGui::PopFont();
		ImGui::PushFont(host.factionNameFont);
		ImVec2 ts2 = ImGui::CalcTextSize(name.c_str());
		ImGui::PopFont();
		// sift out by last dimension
		if (p.x > std::max(homePos.x + ts1.x + gap, namePos.x + ts2.x + gap)) return false;
		// so, the cursor is somewhere in the dimensions of this label
		if (indiamond(ImVec2(pos.x, pos.y), radius + gap * 1.4142, p) ||
			inbox(ImRect(homePos.x - gap, homePos.y - gap, homePos.x + ts1.x + gap, homePos.y + ts1.y + gap), p) ||
			inbox(ImRect(namePos.x - gap, namePos.y - gap, namePos.x + ts2.x + gap, namePos.y + ts2.y + gap), p)) {
			// so the cursor really covers this label
			// save the boxes for further highlighting the label
			host.factionHomeHoverArea = { homePos.x - gap, homePos.y - gap, homePos.x + ts1.x + gap, homePos.y + ts1.y + gap };
			host.factionNameHoverArea = { namePos.x - gap, namePos.y - gap, namePos.x + ts2.x + gap, namePos.y + ts2.y + gap };
			return true;
		} else
			return false;
	}

	void Draw(ImDrawList &dl, bool hovered = false) override
	{
		const float x = pos.x;
		const float y = pos.y;
		// rhombus
		ImVec2 points[] = {
			{ x, y - radius }, { x + radius, y }, { x, y + radius }, { x - radius, y }
		};
		dl.AddConvexPolyFilled(points, 4, color);

		if (hovered) {
			auto hcolor = host.highlightColor;
			auto scolor = host.shadeColor;
			auto &box1 = host.factionHomeHoverArea;
			auto &box2 = host.factionNameHoverArea;
			dl.AddRectFilled(box1.Min, box1.Max, scolor, host.gap);
			dl.AddRectFilled(box2.Min, box2.Max, scolor, host.gap);
			// 1.4142 ~ sqrt(2)
			float diaGap = host.gap * 1.4142;
			// rhombus enlarged by the gap and cut off from the side of the text, so as not to run over the text boxes
			ImVec2 pointsWithGap[] = {
				{ x, y - radius - diaGap }, { x + radius, y - diaGap }, { x + radius, y + diaGap }, { x, y + radius + diaGap }, { x - radius - diaGap, y }
			};
			dl.AddConvexPolyFilled(pointsWithGap, 5, hcolor);
		}
		ImGui::PushFont(host.factionHomeFont);
		dl.AddText(homePos, color, home.c_str());
		ImGui::PopFont();
		ImGui::PushFont(host.factionNameFont);
		dl.AddText(namePos, color, name.c_str());
		ImGui::PopFont();
	}

	void OnClick() override
	{
		host.map.OnClickLabel(path);
	}

	static void InitFonts(Labels &host, ImDrawList &dl)
	{
		auto *&factionHomeFont = host.factionHomeFont;
		auto *&factionNameFont = host.factionNameFont;
		auto &fontName = host.fontName;
		auto &fontSize = host.fontSize;
		factionHomeFont = host.map.GetContext().pigui->GetFont(fontName, fontSize);
		if (!factionHomeFont) factionHomeFont = ImGui::GetFont();
		factionNameFont = host.map.GetContext().pigui->GetFont(fontName, int(fontSize * nameScale));
		if (!factionNameFont) factionNameFont = ImGui::GetFont();
		dl.PushTextureID(factionHomeFont->ContainerAtlas->TexID);
		dl.PushTextureID(factionNameFont->ContainerAtlas->TexID);
	}
};

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
SystemPath SectorMap::NearestSystemToPos(const vector3f &pos)
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
	while (!search_nearest_in_box(m_sectorCache.Get(), sectorPos, crd, searchBox, prevSearchBox, min_dist_sq, best_sys)) {
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
		search_nearest_in_box(m_sectorCache.Get(), sectorPos, crd, searchBox, prevSearchBox, min_dist_sq, best_sys);
	return best_sys;
}

REGISTER_INPUT_BINDING(SectorMap)
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
	AXIS_BINDING("BindMapViewZoom", SDLK_KP_PLUS, SDLK_KP_MINUS)
	AXIS_BINDING("BindMapViewMoveUp", SDLK_r, SDLK_f)
	AXIS_BINDING("BindMapViewMoveLeft", SDLK_a, SDLK_d)
	AXIS_BINDING("BindMapViewMoveForward", SDLK_w, SDLK_s)

	BINDING_GROUP(SectorMapViewControls)
	KEY_BINDING("BindMapToggleSelectionFollowView", SDLK_RETURN, SDLK_KP_ENTER)
	KEY_BINDING("BindMapWarpToSelectedSystem", SDLK_g, 0)

#undef KEY_BINDING
#undef AXIS_BINDING
#undef BINDING_GROUP
}

void SectorMap::InputBinding::RegisterBindings()
{
	mapViewMoveForward = AddAxis("BindMapViewMoveForward");
	mapViewMoveLeft = AddAxis("BindMapViewMoveLeft");
	mapViewMoveUp = AddAxis("BindMapViewMoveUp");

	mapViewYaw = AddAxis("BindMapViewYaw");
	mapViewPitch = AddAxis("BindMapViewPitch");
	mapViewZoom = AddAxis("BindMapViewZoom");
}

SectorMap::SectorMap(SectorMapContext &&context) :
	InputBindings(context.input),
	m_context(std::move(context)),
	m_pos{},
	m_posMovingTo{},
	m_labels(*this)
{
	InitDefaults();

	m_rotX = m_rotXMovingTo = m_rotXDefault;
	m_rotZ = m_rotZMovingTo = m_rotZDefault;
	m_zoom = m_zoomMovingTo = m_zoomDefault;
	m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);

	m_detailBoxVisible = DETAILBOX_INFO;
	m_toggledFaction = false;

	InitObject();
}

SectorMap::SectorMap(const Json &jsonObj, SectorMapContext &&context) :
	InputBindings(context.input),
	m_context(std::move(context)),
	m_labels(*this)
{
	InitDefaults();

	try {
		Json sectorViewObj = jsonObj["sector_map"];

		m_pos.x = m_posMovingTo.x = sectorViewObj["pos_x"];
		m_pos.y = m_posMovingTo.y = sectorViewObj["pos_y"];
		m_pos.z = m_posMovingTo.z = sectorViewObj["pos_z"];
		m_rotX = m_rotXMovingTo = sectorViewObj["rot_x"];
		m_rotZ = m_rotZMovingTo = sectorViewObj["rot_z"];
		m_zoom = m_zoomMovingTo = sectorViewObj["zoom"];
		m_zoomClamped = Clamp(m_zoom, 1.f, FAR_LIMIT);
		m_detailBoxVisible = sectorViewObj["detail_box_visible"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	InitObject();
}

void SectorMap::InitDefaults()
{
	m_rotXDefault = m_context.config->Float("SectorViewXRotation");
	m_rotZDefault = m_context.config->Float("SectorViewZRotation");
	m_zoomDefault = m_context.config->Float("SectorViewZoom");
	m_rotXDefault = Clamp(m_rotXDefault, -170.0f, -10.0f);
	m_zoomDefault = Clamp(m_zoomDefault, 0.1f, 5.0f);

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

	m_sectorCache = m_context.galaxy->NewSectorSlaveCache();
	InputBindings.RegisterBindings();
	m_size.x = m_context.renderer->GetWindowWidth();
	m_size.y = m_context.renderer->GetWindowHeight();
}

void SectorMap::InitObject()
{
	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_customLineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_secLineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, 500));
	m_starVerts.reset(new Graphics::VertexArray(
		Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0, 500));

	Graphics::RenderStateDesc rsd{};
	rsd.blendMode = Graphics::BLEND_ALPHA;

	Graphics::MaterialDescriptor bbMatDesc;
	m_starMaterial.Reset(m_context.renderer->CreateMaterial("sphereimpostor", bbMatDesc, rsd));

	rsd.depthWrite = false;
	rsd.cullMode = CULL_NONE;

	Graphics::MaterialDescriptor starPointDesc;
	starPointDesc.vertexColors = true;
	m_farStarsMat.Reset(m_context.renderer->CreateMaterial("unlit", starPointDesc, rsd));

	rsd.primitiveType = Graphics::LINE_SINGLE;

	Graphics::MaterialDescriptor lineDesc;
	m_lineMat.Reset(m_context.renderer->CreateMaterial("vtxColor", lineDesc, rsd));

	m_drawList.reset(new ImDrawList(ImGui::GetDrawListSharedData()));
}

SectorMap::~SectorMap() {}

void SectorMap::SaveToJson(Json &jsonObj)
{
	Json sectorMapObj{};

	sectorMapObj["pos_x"] = m_pos.x;
	sectorMapObj["pos_y"] = m_pos.y;
	sectorMapObj["pos_z"] = m_pos.z;
	sectorMapObj["rot_x"] = m_rotX;
	sectorMapObj["rot_z"] = m_rotZ;
	sectorMapObj["zoom"] = m_zoom;

	sectorMapObj["detail_box_visible"] = m_detailBoxVisible;

	jsonObj["sector_map"] = sectorMapObj; // Add sector view object to supplied object.
}

matrix4x4f SectorMap::PointOfView() {
	matrix4x4f result = matrix4x4f::Identity();
	// units are lightyears, my friend
	result.Translate(0.f, 0.f, -10.f - 10.f * m_zoom); // not zoomClamped, let us zoom out a bit beyond what we're drawing
	result.Rotate(DEG2RAD(m_rotX), 1.f, 0.f, 0.f);
	result.Rotate(DEG2RAD(m_rotZ), 0.f, 0.f, 1.f);
	return result;
}

void SectorMap::Draw3D()
{
	PROFILE_SCOPED()

	auto &renderer = m_context.renderer;
	m_secLineVerts->Clear();
	m_labels.array.clear();

	float aspect = m_size.x / m_size.y;

	if (m_zoomClamped <= FAR_THRESHOLD)
		renderer->SetPerspectiveProjection(40.f, aspect, 1.f, 300.f);
	else
		renderer->SetPerspectiveProjection(40.f, aspect, 1.f, 600.f);

	renderer->ClearScreen();

	Graphics::Renderer::MatrixTicket ticket(renderer);

	matrix4x4f modelview = PointOfView();

	// sector drawing expects only the fractional part of the position,
	// and handles sector offset internally
	matrix4x4f sectorTrans = modelview;
	sectorTrans.Translate(-FFRAC(m_pos.x) * Sector::SIZE, -FFRAC(m_pos.y) * Sector::SIZE, -FFRAC(m_pos.z) * Sector::SIZE);
	renderer->SetTransform(sectorTrans);

	if (m_zoomClamped <= FAR_THRESHOLD)
		DrawNearSectors(sectorTrans);
	else
		DrawFarSectors(sectorTrans);

	modelview.Translate(-m_pos * Sector::SIZE);

	//draw custom line verts in one go
	renderer->SetTransform(modelview);
	if (!m_customLineVerts->IsEmpty()) {
		if(m_updateCustomLines) {
			m_customlines.SetData(m_customLineVerts->GetNumVerts(), &m_customLineVerts->position[0], &m_customLineVerts->diffuse[0]);
			m_updateCustomLines = false;
		}
		m_customlines.Draw(renderer, m_lineMat.Get());
	}

	renderer->SetTransform(matrix4x4f::Identity());

	//draw star billboards in one go
	renderer->SetAmbientColor(Color(30, 30, 30));
	renderer->DrawBuffer(m_starVerts.get(), m_starMaterial.Get());
	m_starVerts->Clear();

	//draw sector legs in one go
	if (!m_lineVerts->IsEmpty()) {
		m_lines.SetData(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);
		m_lines.Draw(renderer, m_lineMat.Get());
	}

	if (!m_secLineVerts->IsEmpty()) {
		m_sectorlines.SetData(m_secLineVerts->GetNumVerts(), &m_secLineVerts->position[0], &m_secLineVerts->diffuse[0]);
		m_sectorlines.Draw(renderer, m_lineMat.Get());
	}

	if (!m_sphereParams.empty()) {
		for (const auto& s : m_sphereParams) {
			m_context.renderer->SetTransform(s.trans);
			m_fresnelMat->diffuse = s.color;
			m_sphere->Draw(m_context.renderer, m_fresnelMat.Get());
		}
	}
}

void SectorMap::DrawLabels(bool interactive, const ImVec2 &imagePos)
{
	if (!m_hideLabels) DrawLabelsInternal(interactive, imagePos);
}

void SectorMap::SetSize(vector2d size)
{
	ImVec2 new_size(size.x, size.y);
	if (new_size.x == m_size.x && new_size.y == m_size.y) return;

	m_size = new_size;
	m_needsResize = true;
}

void SectorMap::CreateRenderTarget()
{
	if (m_renderTarget)
		m_renderTarget.reset();

	Graphics::RenderTargetDesc rtDesc{
		uint16_t(m_size.x), uint16_t(m_size.y),
		Graphics::TextureFormat::TEXTURE_RGBA_8888,
		Graphics::TextureFormat::TEXTURE_DEPTH, true
	};

	m_renderTarget.reset(m_context.renderer->CreateRenderTarget(rtDesc));
	if (!m_renderTarget) Error("Error creating render target for model viewer.");

	m_needsResize = false;
}

// render to image and insert
void SectorMap::DrawEmbed()
{
	PROFILE_SCOPED()

	if (m_needsResize) CreateRenderTarget();
	if (!m_renderTarget) return;

	// Draw the image and stretch it over the available region.
	// ImGui inverts the vertical axis to get top-left coordinates, so we need to invert our UVs to match.
	ImVec2 imagePos = ImGui::GetCursorScreenPos();
	ImGui::Image(m_renderTarget->GetColorTexture(), m_size, ImVec2(0, 1), ImVec2(1, 0));

	auto *r = m_context.renderer;
	const auto &desc = m_renderTarget.get()->GetDesc();

	{
		// state ticket resets all draw state at the end of the scope
		Graphics::Renderer::StateTicket ticket(r);

		r->SetRenderTarget(m_renderTarget.get());
		r->SetViewport({ 0, 0, desc.width, desc.height });

		Draw3D();
		DrawLabels(ImGui::IsItemHovered(), imagePos);
	}

	if (ImGui::IsItemHovered()) {
		ImGui::CaptureMouseFromApp(false);
	}
}

// function is used to pass label parameters from lua
void SectorMap::SetLabelParams(std::string fontName, int fontSize, float gap, Color highlight, Color shade)
{
	m_labels.fontName = fontName;
	m_labels.fontSize = fontSize;
	m_labels.gap = gap;
	m_labels.highlightColor = IM_COL32(highlight.r, highlight.g, highlight.b, highlight.a);
	m_labels.shadeColor = IM_COL32(shade.r, shade.g, shade.b, shade.a);
}

// function should run inside imgui context
void SectorMap::DrawLabelsInternal(bool interactive, const ImVec2 &imagePos)
{
	// sort the labels to draw them starting from the farthest
	std::sort(m_labels.array.begin(), m_labels.array.end(), [](const std::unique_ptr<Label> &a, const std::unique_ptr<Label> &b) {
		return a->Depth() > b->Depth();
	});

	const size_t NOT_FOUND = m_labels.array.size();
	// the index of the label the cursor is hovering over
	size_t hovered_i = NOT_FOUND;

	m_drawList->_ResetForNewFrame();
	StarLabel::InitFonts(m_labels, *m_drawList);
	FactionLabel::InitFonts(m_labels, *m_drawList);
	m_drawList->PushClipRect({ 0.f, 0.f }, m_size);

	// iterate over the labels starting with the closest one until we find the one over which the cursor is hanging
	if (interactive) {
		ImVec2 mpos = ImGui::GetMousePos();
		mpos.x -= imagePos.x;
		mpos.y -= imagePos.y;
		for (int i = m_labels.array.size() - 1; i >= 0; --i) {
			Label &label = *m_labels.array[i];
			if (hovered_i == NOT_FOUND && !m_rotateWithMouseButton && label.Hovered(mpos)) {
				hovered_i = i;
				if (m_context.input->IsMouseButtonReleased(SDL_BUTTON_LEFT)) {
					label.OnClick();
				}
				break;
			}
		}
	}

	// draw labels
	for (size_t i = 0; i < m_labels.array.size(); ++i) {
		if (i == hovered_i) continue;
		Label &label = *m_labels.array[i];
		m_drawList->AddDrawCmd();
		m_drawList->CmdBuffer.back().PrimDepth = -label.Depth() * 1.01;
		label.Draw(*m_drawList);
	}

	// draw the label over which the cursor is hovering the most recent
	if (hovered_i != NOT_FOUND) {
		Label &hovered = *m_labels.array[hovered_i];
		m_drawList->AddDrawCmd();
		// seems with this depth, the label will be drawn on top of all objects
		m_drawList->CmdBuffer.back().PrimDepth = 1.0f;
		hovered.Draw(*m_drawList, true);
	}

	ImDrawData drawData{};
	ImDrawList *dl = m_drawList.get();

	drawData.Valid = true;
	drawData.CmdLists.push_back(dl);
	drawData.CmdListsCount = 1;
	drawData.TotalVtxCount = dl->VtxBuffer.size();
	drawData.TotalIdxCount = dl->IdxBuffer.size();

	drawData.DisplayPos = ImVec2(0.0f, 0.0f);
	drawData.DisplaySize = ImVec2(m_size.x, m_size.y);
	drawData.FramebufferScale = ImVec2(1.0f, 1.0f);

	m_context.pigui->GetRenderer()->RenderDrawData(&drawData);
}

void SectorMap::GotoSector(const SystemPath &path)
{
	m_posMovingTo = vector3f(path.sectorX, path.sectorY, path.sectorZ);
}

void SectorMap::GotoSystem(const SystemPath &path)
{
	RefCountedPtr<Sector> ps = GetCached(path);
	const vector3f &p = ps->m_systems[path.systemIndex].GetPosition();
	m_posMovingTo.x = path.sectorX + p.x / Sector::SIZE;
	m_posMovingTo.y = path.sectorY + p.y / Sector::SIZE;
	m_posMovingTo.z = path.sectorZ + p.z / Sector::SIZE;
}

bool SectorMap::IsCenteredOn(const SystemPath &path)
{
	RefCountedPtr<Sector> ps = GetCached(path);
	const vector3f &p = ps->m_systems[path.systemIndex].GetPosition();
	vector3f diff = vector3f(
		fabs(m_pos.x - path.sectorX - p.x / Sector::SIZE),
		fabs(m_pos.y - path.sectorY - p.y / Sector::SIZE),
		fabs(m_pos.z - path.sectorZ - p.z / Sector::SIZE));
	return ((diff.x < 0.005f && diff.y < 0.005f && diff.z < 0.005f));
}

void SectorMap::OnClickLabel(const SystemPath &path)
{
	// redirect to the real host
	m_context.callbacks->OnClickLabel(path);
}

void SectorMap::PutSystemLabel(const Sector::System &sys, bool shadow)
{
	PROFILE_SCOPED()

	// place the label
	//vector3d systemPos = vector3d((*sys).GetFullPosition() - origin);
	vector3d screenPos = Graphics::ProjectToScreen(m_context.renderer, vector3d(0.0));
	// reject back-projected labels (negative Z in clipspace is in front of the view plane)
	if (screenPos.z < 0.0f && (sys.GetPopulation() > 0 || m_drawUninhabitedLabels)) {
		vector3d screenStarEdge = Graphics::ProjectToScreen(m_context.renderer, vector3d(0.25, 0.0, 0.0));
		float screenStarRadius = screenStarEdge.x - screenPos.x;
		// work out the colour
		Color labelColor = sys.GetFaction()->AdjustedColour(sys.GetPopulation(), shadow);
		// get a system path to pass to the event handler when the label is clicked
		SystemPath sysPath = sys.GetPath();
		// label text
		std::string text = sys.GetName();
		const float x = screenPos.x;
		const float y = m_size.y - screenPos.y;
		const float z = screenPos.z;
		m_labels.array.emplace_back(std::make_unique<StarLabel>(m_labels, vector3f(x, y, z), labelColor, text, screenStarRadius, sysPath));
	}
}

void SectorMap::PutFactionLabels(const vector3f &origin)
{
	PROFILE_SCOPED()
	for (auto it = m_visibleFactions.begin(); it != m_visibleFactions.end(); ++it) {
		if ((*it)->hasHomeworld && m_hiddenFactions.find((*it)) == m_hiddenFactions.end()) {
			Sector::System sys = GetCached((*it)->homeworld)->m_systems[(*it)->homeworld.systemIndex];
			if ((m_pos * Sector::SIZE - sys.GetFullPosition()).Length() > (m_zoomClamped / FAR_THRESHOLD) * OUTER_RADIUS) continue;

			vector3d pos = Graphics::ProjectToScreen(m_context.renderer, vector3d(sys.GetFullPosition() - origin));
			if (pos.z < 0.0f) { // reject back-projected stars
				pos.y = m_size.y - pos.y;

				std::string factionHome = sys.GetName();
				std::string factionName = (*it)->name;
				Color labelColor = (*it)->colour;

				m_labels.array.emplace_back(std::make_unique<FactionLabel>(m_labels, vector3f(pos), labelColor, factionHome, factionName, 15.f, sys.GetPath()));
			}
		}
	}
}

void SectorMap::AddSphere(const matrix4x4f &trans, const Color &color)
{
	m_sphereParams.push_back({ trans, color });
}

void SectorMap::AddStarBillboard(const matrix4x4f &trans, const vector3f &pos, const Color &col, float size)
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

void SectorMap::AddLineVert(const vector3f &v, const Color &c)
{
	m_customLineVerts->Add(v, c);
}

void SectorMap::ClearLineVerts()
{
	m_customLineVerts->Clear();
	m_updateCustomLines = true;
}

void SectorMap::DrawNearSectors(const matrix4x4f &modelview)
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
}

void SectorMap::DrawNearSector(const int sx, const int sy, const int sz, const matrix4x4f &trans)
{
	PROFILE_SCOPED()
	m_context.renderer->SetTransform(trans);
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

		auto showMode = m_context.callbacks->GetDisplayMode(i->GetPath());

		bool can_skip = !(showMode & m_context.ALWAYS);

		// if the system belongs to a faction we've chosen to temporarily hide
		// then skip it if we can
		m_visibleFactions.insert(i->GetFaction());
		if (can_skip && m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end()) continue;

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
				RefCountedPtr<StarSystem> pSS = m_context.galaxy->GetStarSystem(current);
				i->SetPopulation(pSS->GetTotalPop());
			}
		}

		matrix4x4f systrans = trans * matrix4x4f::Translation(i->GetPosition().x, i->GetPosition().y, i->GetPosition().z);
		m_context.renderer->SetTransform(systrans);

		if ((m_drawVerticalLines && (i->GetPopulation() > 0 || m_drawUninhabitedLabels)) || !can_skip) {

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

		// draw star blob itself
		systrans.Rotate(DEG2RAD(-m_rotZ), 0, 0, 1);
		systrans.Rotate(DEG2RAD(-m_rotX), 1, 0, 0);
		systrans.Scale((StarSystem::starScale[(*i).GetStarType(0)]));
		m_context.renderer->SetTransform(systrans);

		const Uint8 *col = StarSystem::starColors[(*i).GetStarType(0)];
		AddStarBillboard(systrans, vector3f(0.f), Color(col[0], col[1], col[2], 255), 0.5f);

		// add label
		if (!(showMode & m_context.HIDE_LABEL)) PutSystemLabel(*i, showMode & m_context.SHADOW_LABEL);
	}
}

void SectorMap::DrawFarSectors(const matrix4x4f &modelview)
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
		// TODO: this should query screen DPI instead of platform window height
		float sizeFactor = 0.25f * (m_context.renderer->GetWindowHeight() / 720.f);
		m_farstarsPoints.SetData(m_context.renderer, m_farstars.size(), &m_farstars[0], &m_farstarsColor[0], modelview, sizeFactor);
		m_farstarsPoints.Draw(m_context.renderer, m_farStarsMat.Get());
	}

	// also add labels for any faction homeworlds among the systems we've drawn
	PutFactionLabels(Sector::SIZE * secOrigin);
}

void SectorMap::BuildFarSector(RefCountedPtr<Sector> sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors)
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
		if (m_hiddenFactions.find(i->GetFaction()) != m_hiddenFactions.end()) continue;

		// otherwise add the system's position (origin must be m_pos's *sector* or we get judder)
		// and faction color to the list to draw
		starColor = i->GetFaction()->colour;
		starColor.a = 120;

		points.push_back((*i).GetFullPosition() - origin);
		colors.push_back(starColor);
	}
}

void SectorMap::Update(float frameTime)
{
	PROFILE_SCOPED()

	auto input = m_context.input;

	matrix4x4f rot = matrix4x4f::Identity();
	rot.RotateX(DEG2RAD(-m_rotX));
	rot.RotateZ(DEG2RAD(-m_rotZ));

	matrix3x3f shiftRot = matrix3x3f::Rotate(DEG2RAD(m_rotZ), { 0, 0, 1 });

	const float moveSpeed = input->GetMoveSpeedShiftModifier();
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

	int wheel = input->GetMouseWheel();
	if (wheel != 0) {
		m_zoomMovingTo -= wheel * ZOOM_SPEED * WHEEL_SENSITIVITY * input->GetMoveSpeedShiftModifier();
	}

	if (InputBindings.mapViewZoom->IsActive()) m_zoomMovingTo -= move * InputBindings.mapViewZoom->GetValue();

	m_zoomMovingTo = Clamp(m_zoomMovingTo, 0.1f, FAR_MAX);

	if (InputBindings.mapViewYaw->IsActive()) m_rotZMovingTo += 0.5f * moveSpeed * InputBindings.mapViewYaw->GetValue();
	if (InputBindings.mapViewPitch->IsActive()) m_rotXMovingTo += 0.5f * moveSpeed * InputBindings.mapViewPitch->GetValue();

	// to capture mouse when button was pressed and release when released
	if (input->MouseButtonState(SDL_BUTTON_MIDDLE) != m_rotateWithMouseButton) {
		m_rotateWithMouseButton = !m_rotateWithMouseButton;
		input->SetCapturingMouse(m_rotateWithMouseButton);
	}

	if (m_rotateWithMouseButton || m_rotateView) {
		int motion[2];
		input->GetMouseMotion(motion);
		m_rotXMovingTo += 0.2f * float(motion[1]);
		m_rotZMovingTo += 0.2f * float(motion[0]);
	} else if (m_zoomView) {
		input->SetCapturingMouse(true);
		int motion[2];
		input->GetMouseMotion(motion);
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

	ShrinkCache();

	if (!m_sphere) {
		Graphics::RenderStateDesc rsd;
		rsd.blendMode = Graphics::BLEND_ALPHA;
		rsd.depthTest = false;
		rsd.depthWrite = false;
		rsd.cullMode = Graphics::CULL_NONE;

		Graphics::MaterialDescriptor matdesc;
		m_fresnelMat.Reset(m_context.renderer->CreateMaterial("fresnel_sphere", matdesc, rsd));
		m_fresnelMat->diffuse = Color::WHITE;
		m_sphere.reset(new Graphics::Drawables::Sphere3D(m_context.renderer, 4, 1.0f));
	}
	m_sphereParams.clear();
	m_lineVerts->Clear();
}

void SectorMap::ShrinkCache()
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

double SectorMap::GetZoomLevel() const
{
	return ((m_zoomClamped / FAR_THRESHOLD) * (OUTER_RADIUS)) + 0.5 * Sector::SIZE;
}

std::vector<SystemPath> SectorMap::GetNearbyStarSystemsByName(std::string pattern)
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

void SectorMap::SetFactionVisible(const Faction *faction, bool visible)
{
	if (visible)
		m_hiddenFactions.erase(faction);
	else
		m_hiddenFactions.insert(faction);
	m_toggledFaction = true;
}

void SectorMap::SetZoomMode(bool enable)
{
	if (enable != m_zoomView) {
		m_context.input->SetCapturingMouse(enable);
		m_zoomView = enable;
		if (m_zoomView) m_rotateView = false;
	}
}

void SectorMap::SetRotateMode(bool enable)
{
	if (enable != m_rotateView) {
		m_context.input->SetCapturingMouse(enable);
		m_rotateView = enable;
		if (m_rotateView) m_zoomView = false;
	}
}

void SectorMap::ResetView()
{
	while (m_rotZ < -180.0f)
		m_rotZ += 360.0f;
	while (m_rotZ > 180.0f)
		m_rotZ -= 360.0f;
	m_rotXMovingTo = m_rotXDefault;
	m_rotZMovingTo = m_rotZDefault;
	m_zoomMovingTo = m_zoomDefault;
}

vector3f SectorMap::GetSystemPosition(const SystemPath &path)
{
	const Sector::System sys = GetCached(path)->m_systems[path.systemIndex];
	return Sector::SIZE * vector3f(float(path.sectorX), float(path.sectorY), float(path.sectorZ)) + sys.GetPosition();
}
