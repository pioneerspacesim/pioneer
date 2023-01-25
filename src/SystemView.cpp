// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/SystemBody.h"
#include "pigui/LuaPiGui.h"

#include "SystemView.h"

#include "AnimationCurves.h"
#include "Background.h"
#include "Game.h"
#include "GameLog.h"
#include "Input.h"
#include "Lang.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Space.h"
#include "StringF.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "galaxy/SystemPath.h"
#include "gameconsts.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "lua/LuaObject.h"
#include "lua/LuaTable.h"

#include "libs.h"
#include "matrix4x4.h"
#include <iomanip>
#include <sstream>

using namespace Graphics;

static constexpr Uint16 N_VERTICES_MAX = 100;
static const float MIN_ZOOM = 1e-30f; // Just to avoid having 0
static const float MAX_ZOOM = 1e30f;
static const float MIN_ATLAS_ZOOM = 0.5f; // Just to avoid having 0
static const float MAX_ATLAS_ZOOM = 2.0f;
static const float ZOOM_IN_SPEED = 3;
static const float ZOOM_OUT_SPEED = 3;
static const float WHEEL_SENSITIVITY = .1f; // Should be a variable in user settings.
static const double DEFAULT_VIEW_DISTANCE = 10.0;
static const int MAX_TRANSITION_FRAMES = 60;

static const float ATLAS_SCROLL_SENS = .1f;

void SystemView::InputBindings::RegisterBindings()
{
	mapViewPitch = AddAxis("BindMapViewPitch");
	mapViewYaw = AddAxis("BindMapViewYaw");
	mapViewZoom = AddAxis("BindMapViewZoom");
}

SystemView::SystemView(Game *game) :
	PiGuiView("system-view"),
	m_input(Pi::input),
	m_game(game),
	m_viewingCurrentSystem(false),
	m_displayMode(Mode::Orrery),
	m_showL4L5(LAG_OFF),
	m_shipDrawing(OFF),
	m_gridDrawing(GridDrawing::OFF),
	m_trans(0.0),
	m_transTo(0.0)
{
	m_rot_y = 0;
	m_rot_x = 50;
	m_atlasPos = vector2f();
	m_zoom = 1.0f / float(AU);
	m_atlasZoom = m_atlasZoomTo = 1.0f;

	m_input.RegisterBindings();

	Graphics::MaterialDescriptor lineMatDesc;

	Graphics::RenderStateDesc rsd;
	rsd.primitiveType = Graphics::LINE_STRIP;

	m_lineMat.reset(Pi::renderer->CreateMaterial("vtxColor", lineMatDesc, rsd)); //m_renderer not set yet

	rsd.primitiveType = Graphics::LINE_SINGLE;
	m_gridMat.reset(Pi::renderer->CreateMaterial("vtxColor", lineMatDesc, rsd));

	m_realtime = true;
	m_unexplored = true;

	m_onMouseWheelCon =
		Pi::input->onMouseWheel.connect(sigc::mem_fun(this, &SystemView::MouseWheel));

	ResetViewpoint();

	RefreshShips();
	m_planner = Pi::planner;

	m_orbitVts.reset(new vector3f[N_VERTICES_MAX + 1]);
	m_orbitColors.reset(new Color[N_VERTICES_MAX + 1]);
}

SystemView::~SystemView()
{
	m_contacts.clear();
}

void SystemView::AccelerateTime(float step)
{
	m_realtime = false;
	m_timeStep = step;
}

void SystemView::SetRealTime()
{
	m_realtime = true;
}

void SystemView::ResetViewpoint()
{
	m_viewedObject.type = Projectable::NONE;
	m_rot_y_to = 0;
	m_rot_x_to = 50;
	m_zoomTo = 1.0f / float(AU);
	m_timeStep = 1.0f;
	m_time = m_game->GetTime();
	m_transTo *= 0.0;
	m_animateTransition = MAX_TRANSITION_FRAMES;

	if (!m_renderer) return;

	float height = tan(DEG2RAD(CAMERA_FOV)) * 30.0f;
	m_atlasViewW = height * m_renderer->GetDisplayAspect();
	m_atlasViewH = height;

	bool hasChildren = m_atlasLayout.children.size();
	bool isBinary = m_atlasLayout.isBinary;
	vector2f size = m_atlasLayout.size;
	vector2f avail = vector2f(m_atlasViewW, m_atlasViewH) * 0.85f;

	m_atlasZoomDefault = Clamp(std::max(size.x / avail.x, size.y / avail.y), 1.0f, MAX_ATLAS_ZOOM);

	// This framing algorithm doesn't produce perfect results, but it works in 95% of cases.
	// Future developers can tweak it for prettier results in the SystemView.
	m_atlasPosDefault = 0.5f * vector2f(hasChildren ? std::min(size.x / avail.x, 1.0f) * avail.x : 0, isBinary ? std::min(size.y / avail.y, 1.0f) * avail.y : 0);

	m_atlasPosTo *= 0.0;
	m_atlasZoomTo = m_atlasZoomDefault;
}

RefCountedPtr<StarSystem> SystemView::GetCurrentSystem()
{
	return m_system;
}

template <typename RefType>
void SystemView::PutOrbit(Projectable::bases base, RefType *ref, const Orbit *orbit, const vector3d &offset, const Color &color, const double planetRadius, const bool showLagrange)
{
	double ecc = orbit->GetEccentricity();
	double timeshift = ecc > 0.6 ? 0.0 : 0.5;
	double maxT = 1.;
	unsigned short num_vertices = 0;
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = (double(i) + timeshift) / double(N_VERTICES_MAX);
		const vector3d pos = orbit->EvenSpacedPosTrajectory(t);
		if (pos.Length() < planetRadius) {
			maxT = t;
			break;
		}
	}

	static const float startTrailPercent = 0.85;
	static const float fadedColorParameter = 0.8;

	Uint16 fadingColors = 0;
	const double tMinust0 = GetOrbitTime(m_time, ref);
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = (double(i) + timeshift) / double(N_VERTICES_MAX) * maxT;
		if (fadingColors == 0 && t >= startTrailPercent * maxT)
			fadingColors = i;
		const vector3d pos = orbit->EvenSpacedPosTrajectory(t, tMinust0);
		m_orbitVts[i] = vector3f(offset + pos);
		++num_vertices;
		if (pos.Length() < planetRadius)
			break;
	}


	if (num_vertices > 1) {
		//close the loop for thin ellipses
		if (!(maxT < 1. || ecc > 1.0 || ecc < 0.6)) {
			m_orbitVts[num_vertices] = m_orbitVts[0];
			m_orbitColors[num_vertices] = m_orbitColors[0];
			++num_vertices;
		}

		// fade trail
		const Color fadedColor = color * fadedColorParameter;
		std::fill_n(m_orbitColors.get(), num_vertices, fadedColor);
		const Uint16 trailLength = num_vertices - fadingColors;
		for (Uint16 currentColor = 0; currentColor < trailLength; ++currentColor) {
			float scalingParameter = (1.f - static_cast<float>(currentColor) / (trailLength - 1));
			m_orbitColors[currentColor + fadingColors] = fadedColor * scalingParameter;
		}

		m_orbits.SetData(num_vertices, m_orbitVts.get(), m_orbitColors.get());
		m_orbits.Draw(m_renderer, m_lineMat.get());
	}

	AddProjected<RefType>(Projectable::PERIAPSIS, base, ref, offset + orbit->Perigeum());
	AddProjected<RefType>(Projectable::APOAPSIS, base, ref, offset + orbit->Apogeum());

	if (showLagrange && m_showL4L5 != LAG_OFF) {
		const vector3d posL4 = orbit->EvenSpacedPosTrajectory((1.0 / 360.0) * 60.0, tMinust0);
		AddProjected<RefType>(Projectable::L4, base, ref, offset + posL4);

		const vector3d posL5 = orbit->EvenSpacedPosTrajectory((1.0 / 360.0) * 300.0, tMinust0);
		AddProjected<RefType>(Projectable::L5, base, ref, offset + posL5);
	}
}

// returns the position of the ground spaceport relative to the center of the planet at the specified time
static vector3d position_of_surface_starport_relative_to_parent(const SystemBody *starport, double time)
{
	const SystemBody *parent = starport->GetParent();
	// planet axis tilt
	return matrix3x3d::RotateX(parent->GetAxialTilt()) *
		// the angle the planet has turned since the beginning of time
		matrix3x3d::RotateY(-2 * M_PI / parent->GetRotationPeriod() * time + parent->GetRotationPhaseAtStart()) *
		// the original coordinates of the starport are saved as a 3x3 matrix,
		starport->GetOrbit().GetPlane() *
		// to get the direction to the station, you need to multiply them by 0.0, 1.0, 0.0
		vector3d(0.0, 1.0, 0.0) *
		// we need the distance to the center of the planet
		(Pi::game->IsNormalSpace() && Pi::game->GetSpace()->GetStarSystem()->GetPath().IsSameSystem(Pi::game->GetSectorView()->GetSelected()) ?
				  // if we look at the current system, the relief is known, we take the height from the physical body
				  Pi::game->GetSpace()->FindBodyForPath(&(starport->GetPath()))->GetPosition().Length() :
				  // if the remote system - take the radius of the planet
				  parent->GetRadius());
}

void SystemView::PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans)
{
	if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
		return;

	if (b->GetType() != SystemBody::TYPE_GRAVPOINT) {
		const double radius = b->GetRadius();

		matrix4x4f invRot = trans;
		invRot.ClearToRotOnly();
		invRot.Renormalize();
		invRot = invRot.Inverse();

		matrix4x4f bodyTrans = trans;
		bodyTrans.Translate(vector3f(offset));
		bodyTrans.Scale(radius);

		m_renderer->SetTransform(bodyTrans * invRot);
		m_bodyMat->diffuse = svColor[SYSTEMBODY];
		m_bodyIcon->Draw(m_renderer, m_bodyMat.get());

		m_renderer->SetTransform(trans);

		AddProjected<const SystemBody>(Projectable::OBJECT, Projectable::SYSTEMBODY, b, offset);
	}

	// display all child bodies and their orbits
	if (b->HasChildren()) {
		for (const SystemBody *kid : b->GetChildren()) {
			if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
				AddProjected<const SystemBody>(Projectable::OBJECT, Projectable::SYSTEMBODY, kid, position_of_surface_starport_relative_to_parent(kid, m_time) + offset);
				continue;
			}

			if (is_zero_general(kid->GetOrbit().GetSemiMajorAxis())) continue;

			const double axisZoom = kid->GetOrbit().GetSemiMajorAxis();
			//semimajor axis radius should be at least 1% of screen width to show the orbit
			if (ProjectedSize(axisZoom, offset) > 0.01) {
				const SystemBody::BodySuperType bst = kid->GetSuperType();
				const bool showLagrange = (bst == SystemBody::SUPERTYPE_ROCKY_PLANET || bst == SystemBody::SUPERTYPE_GAS_GIANT);
				PutOrbit<const SystemBody>(Projectable::SYSTEMBODY, kid, &(kid->GetOrbit()), offset, svColor[SYSTEMBODY_ORBIT], 0.0, showLagrange);
			}

			// not using current time yet
			const vector3d pos = kid->GetOrbit().OrbitalPosAtTime(m_time);
			PutBody(kid, offset + pos, trans);
		}
	}
}

void SystemView::GetTransformTo(const SystemBody *b, vector3d &pos)
{
	const SystemBody *parent = b->GetParent();
	if (parent) {
		GetTransformTo(parent, pos);
		if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
			pos -= position_of_surface_starport_relative_to_parent(b, m_time);
		else
			pos -= b->GetOrbit().OrbitalPosAtTime(m_time);
	}
}

void SystemView::GetTransformTo(Projectable &p, vector3d &pos)
{
	if (p.type == Projectable::NONE) {
		// nothing selected
		pos *= 0.0;
		return;
	}

	// accept only real objects (no orbit icons or lagrange points)
	assert(p.type == Projectable::OBJECT);
	pos = vector3d(0., 0., 0.);
	if (p.base == Projectable::SYSTEMBODY)
		GetTransformTo(p.ref.sbody, pos);
	else if (p.ref.body->GetType() == ObjectType::SHIP || p.ref.body->GetType() == ObjectType::PLAYER) {
		const Ship *s = static_cast<const Ship *>(p.ref.body);
		CalculateShipPositionAtTime(s, s->ComputeOrbit(), m_time, pos);
		pos = -pos;
		// sometimes ships can disappear from world (i.e. docking / undocking)
		if (std::isnan(pos.x)) { // failsafe: calculate parent systembody instead
			pos = vector3d(0., 0., 0.);
			GetTransformTo(Frame::GetFrame(Frame::GetFrame(Pi::player->GetFrame())->GetNonRotFrame())->GetSystemBody(), pos);
		}
	}
}

void SystemView::CalculateShipPositionAtTime(const Ship *s, Orbit o, double t, vector3d &pos)
{
	pos = vector3d(0., 0., 0.);
	FrameId shipFrameId = s->GetFrame();
	FrameId shipNonRotFrameId = Frame::GetFrame(shipFrameId)->GetNonRotFrame();
	// if the ship is in a rotating frame, we will rotate it with the frame
	if (Frame::GetFrame(shipFrameId)->IsRotFrame()) {
		vector3d rpos(0.0);
		Frame *rotframe = Frame::GetFrame(shipFrameId);
		if (t == m_game->GetTime()) {
			pos = s->GetPositionRelTo(m_game->GetSpace()->GetRootFrame());
			return;
		} else
			rpos = s->GetPositionRelTo(shipNonRotFrameId) * rotframe->GetOrient() * matrix3x3d::RotateY(rotframe->GetAngSpeed() * (t - m_game->GetTime())) * rotframe->GetOrient().Transpose();
		vector3d fpos(0.0);
		CalculateFramePositionAtTime(shipNonRotFrameId, t, fpos);
		pos += fpos + rpos;
	} else {
		vector3d fpos(0.0);
		CalculateFramePositionAtTime(shipNonRotFrameId, t, fpos);
		pos += (fpos + o.OrbitalPosAtTime(t - m_game->GetTime()));
	}
}

//frame must be nonrotating
void SystemView::CalculateFramePositionAtTime(FrameId frameId, double t, vector3d &pos)
{
	if (frameId == m_game->GetSpace()->GetRootFrame())
		pos = vector3d(0., 0., 0.);
	else {
		Frame *frame = Frame::GetFrame(frameId);
		CalculateFramePositionAtTime(frame->GetParent(), t, pos);
		pos += frame->GetSystemBody()->GetOrbit().OrbitalPosAtTime(t);
	}
}

void SystemView::Draw3D()
{
	PROFILE_SCOPED()
	m_renderer->ClearScreen();
	m_projected.clear();

	if (!m_bodyIcon) {
		Graphics::MaterialDescriptor desc;
		Graphics::RenderStateDesc rsd;
		rsd.primitiveType = Graphics::TRIANGLE_FAN;

		m_bodyMat.reset(m_renderer->CreateMaterial("unlit", desc, rsd));
		m_bodyIcon.reset(new Graphics::Drawables::Disk(m_renderer));
	}

	if (!m_atlasMat) {
		Graphics::MaterialDescriptor desc;
		desc.textures = 1;
		Graphics::RenderStateDesc rsd;
		rsd.blendMode = Graphics::BLEND_ALPHA;
		rsd.primitiveType = Graphics::TRIANGLE_FAN;
		rsd.depthTest = false;

		m_atlasMat.reset(m_renderer->CreateMaterial("unlit", desc, rsd));
		m_atlasMat->SetTexture(Graphics::Renderer::GetName("texture0"), TextureBuilder::GetWhiteTexture(m_renderer));
	}

	SystemPath path = m_game->GetSectorView()->GetSelected().SystemOnly();
	if (m_system) {
		if (m_system->GetUnexplored() != m_unexplored || !m_system->GetPath().IsSameSystem(path)) {
			m_system.Reset();
		}
	}

	m_viewingCurrentSystem = m_game->IsNormalSpace() && m_game->GetSpace()->GetStarSystem()->GetPath().IsSameSystem(path);

	if (m_realtime) {
		m_time = m_game->GetTime();
	} else {
		m_time += m_timeStep * Pi::GetFrameTime();
	}
	std::string t = Lang::TIME_POINT + format_date(m_time);

	if (!m_system) {
		ClearSelectedObject();

		m_system = m_game->GetGalaxy()->GetStarSystem(path);
		m_unexplored = m_system->GetUnexplored();

		SystemBody *body = m_system->GetRootBody().Get();
		m_atlasLayout = {};
		m_atlasLayout.isVertical = body->GetType() == SystemBody::TYPE_GRAVPOINT;
		LayoutSystemBody(body, m_atlasLayout);

		ResetViewpoint();
	}

	if (m_displayMode == Mode::Orrery) {
		DrawOrreryView();
	} else if (m_displayMode == Mode::Atlas) {
		DrawAtlasView();
	}
}

void SystemView::DrawOrreryView()
{
	// Set up the perspective projection for the background stars
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, m_renderer->GetDisplayAspect(), 1.f, 1500.f);

	// Background is rotated around (0,0,0) and drawn
	matrix4x4d trans2bg = matrix4x4d::Identity();
	trans2bg.RotateX(DEG2RAD(-m_rot_x));
	trans2bg.RotateY(DEG2RAD(-m_rot_y));

	auto *background = m_game->GetSpace()->GetBackground();
	background->SetIntensity(0.6);
	if (!m_game->IsNormalSpace() || !m_viewingCurrentSystem) {
		Uint32 cachedFlags = background->GetDrawFlags();
		background->SetDrawFlags(Background::Container::DRAW_SKYBOX);
		background->Draw(trans2bg);
		background->SetDrawFlags(cachedFlags);
	} else {
		background->Draw(trans2bg);
	}

	m_renderer->ClearDepthBuffer();

	// We need to adjust the "far" cutoff plane, so that at high magnifications you can see
	// distant objects in the background.
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, m_renderer->GetDisplayAspect(), 1.f, 1000.f * m_zoom * float(AU) + DEFAULT_VIEW_DISTANCE * 2);
	//TODO add reserve

	// The matrix is shifted from the (0,0,0) by DEFAULT_VIEW_DISTANCE
	// and then rotated (around 0,0,0) and scaled by m_zoom, shift doesn't scale.
	// m_zoom default value is 1/AU.
	// { src/gameconsts.h:static const double AU = 149598000000.0; // m }
	// The coordinates of the objects are given in meters; they are multiplied by m_zoom,
	// therefore dy default, 1.0 AU (in meters) in the coordinate turns into 1.0 in camera space.
	// Since the *shift doesn't scale*, it always equals to DEFAULT_VIEW_DISTANCE,
	// in camera space.
	// So, the "apparent" view distance, is DEFAULT_VIEW_DISTANCE / m_zoom * AU (AU)
	// Therefore the default "apparent" view distance is DEFAULT_VIEW_DISTANCE, in AU
	// When we change m_zoom, we actually change the "apparent" view distance, because
	// the coordinates of the objects are scaled, but the shift is not.
	m_cameraSpace = matrix4x4f::Identity();
	m_cameraSpace.Translate(0, 0, -DEFAULT_VIEW_DISTANCE);
	m_cameraSpace.Rotate(DEG2RAD(m_rot_x), 1, 0, 0);
	m_cameraSpace.Rotate(DEG2RAD(m_rot_y), 0, 1, 0);
	m_cameraSpace.Scale(m_zoom);
	m_renderer->SetTransform(m_cameraSpace);

	// smooth transition animation
	if (m_animateTransition) {
		// since the object being approached can move, we need to compensate for its movement
		// making an imprint of the old value (from previous frame)
		m_trans -= m_transTo;
		// calculate the new value
		GetTransformTo(m_viewedObject, m_transTo);
		// now the difference between the new and the old value is added to m_trans
		m_trans += m_transTo;
		const float ft = Pi::GetFrameTime();
		m_animateTransition--;
		AnimationCurves::Approach(m_trans.x, m_transTo.x, ft);
		AnimationCurves::Approach(m_trans.y, m_transTo.y, ft);
		AnimationCurves::Approach(m_trans.z, m_transTo.z, ft);
	} else {
		GetTransformTo(m_viewedObject, m_trans);
	}

	vector3d pos = m_trans;

	// glLineWidth(2);
	if (!m_system->GetUnexplored() && m_system->GetRootBody()) {
		// all systembodies draws here
		PutBody(m_system->GetRootBody().Get(), pos, m_cameraSpace);
	}
	// glLineWidth(1);

	if (m_game->IsNormalSpace() && m_viewingCurrentSystem) {
		// draw ships
		if (m_shipDrawing != OFF) {
			DrawShips(m_time, pos);
		}
		// draw player and planner
		vector3d ppos(0.0);
		Orbit playerOrbit = Pi::player->ComputeOrbit();
		Body *PlayerBody = static_cast<Body *>(Pi::player);
		FrameId playerNonRotFrameId = Frame::GetFrame(PlayerBody->GetFrame())->GetNonRotFrame();
		Frame *playerNonRotFrame = Frame::GetFrame(playerNonRotFrameId);
		SystemBody *playerAround = playerNonRotFrame->GetSystemBody();
		CalculateShipPositionAtTime(static_cast<Ship *>(Pi::player), playerOrbit, m_time, ppos);
		AddProjected<Body>(Projectable::OBJECT, Projectable::PLAYER, PlayerBody, ppos + pos);

		vector3d offset(0.0);
		CalculateFramePositionAtTime(playerNonRotFrameId, m_time, offset);
		offset = offset + pos;

		if (Pi::player->GetFlightState() == Ship::FlightState::FLYING) {
			PutOrbit<Body>(Projectable::PLAYER, PlayerBody, &playerOrbit, offset, svColor[PLAYER_ORBIT], playerAround->GetRadius());
			const double plannerStartTime = m_planner->GetStartTime();
			if (!m_planner->GetPosition().ExactlyEqual(vector3d(0, 0, 0))) {
				Orbit plannedOrbit = Orbit::FromBodyState(m_planner->GetPosition(),
					m_planner->GetVel(),
					playerAround->GetMass());
				PutOrbit<Body>(Projectable::PLANNER, PlayerBody, &plannedOrbit, offset, svColor[PLANNER_ORBIT], playerAround->GetRadius());
				if (std::fabs(m_time - m_game->GetTime()) > 1. && (m_time - plannerStartTime) > 0.)
					AddProjected<Body>(Projectable::OBJECT, Projectable::PLANNER, PlayerBody, offset + plannedOrbit.OrbitalPosAtTime(m_time - plannerStartTime));
				else
					AddProjected<Body>(Projectable::OBJECT, Projectable::PLANNER, PlayerBody, offset + m_planner->GetPosition());
			}
		}
	}

	if (m_gridDrawing != GridDrawing::OFF) {
		// calculate lines for this system:
		DrawGrid(std::floor(m_system->GetRootBody()->GetMaxChildOrbitalDistance() * 1.2 / AU));
	}
}

static constexpr double SCALE_EXPONENT = 1.0 / 1.9;
static constexpr double STAR_SCALE_EXPONENT = 1.0 / 3.8;

// Return a unit-scale relative size for a body; this is not a 'real' size, but one that preserves relative scale
double get_body_radius(SystemBody *b)
{
	if (b->GetType() == SystemBody::TYPE_GRAVPOINT)
		return 0.0;

	double x = b->GetRadius() / EARTH_RADIUS;
	if (b->GetSuperType() == SystemBody::SUPERTYPE_STAR)
		// Stars are bigger at smaller sizes, but grow much slower.
		// A star has the same perceptual size as a planet at roughly 32 earth radiuses
		return pow(x, STAR_SCALE_EXPONENT) * 2.5;
	else
		// small objects have a min. radius of 0.25, earth has radius of 1, larger objects grow logarithmically
		return pow(x, SCALE_EXPONENT) + std::max(0.25 * (1.0 - pow(x, 4.0)), 0.0);
}

// LayoutSystemBody accumulates the size of all children bodies into the passed
// layout parameter. The size of the layout is measured from the center of the
// root body to the trailing edge of the furthest child body in both directions.
// The offset and isVertical layout fields are reserved for the caller to set.
void SystemView::LayoutSystemBody(SystemBody *body, AtlasBodyLayout &layout)
{
	layout.body = body;
	layout.radius = get_body_radius(body);
	layout.size = vector2f(layout.radius);

	const int orient = layout.isVertical ? 1 : 0;
	const int crossdir = layout.isVertical ? 0 : 1;

	if (body->GetType() == SystemBody::TYPE_GRAVPOINT) {
		layout.isBinary = true;
		// layout.offset[crossdir] += 10.0f; // gravpoint debugging
	}

	if (!body->GetNumChildren())
		return;

	float maxRadius = layout.radius;
	for (auto *child : body->GetChildren()) {
		if (child->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
			continue;

		AtlasBodyLayout child_layout{};

		// if this is a binary host gravpoint, don't alter the layout direction
		if (child->GetType() == SystemBody::TYPE_GRAVPOINT)
			child_layout.isVertical = layout.isVertical;
		else
			child_layout.isVertical = !layout.isVertical;

		LayoutSystemBody(child, child_layout);

		// layout.size measures to the edge of the last encountered body
		// so we add the radius of the current child body plus a gap
		float offset = layout.size[orient];
		if (offset > 0)
			offset += child_layout.radius + AtlasViewPlanetGap(child_layout.radius);
		child_layout.offset[orient] = offset;

		layout.size[orient] = offset + child_layout.size[orient];
		layout.size[crossdir] = std::max(layout.size[crossdir], child_layout.size[crossdir]);

		maxRadius = std::max(child_layout.radius, maxRadius);
		layout.children.push_back(child_layout);
	}

	if (layout.isBinary) {
		// if we're a gravpoint, set the "radius" from the maximum radius of our direct children
		// (used to determine how much back-fill space is reserved to avoid overlap)
		layout.radius = maxRadius;
	}
}

void SystemView::RenderAtlasBody(const AtlasBodyLayout &layout, vector3f pos, const matrix4x4f &cameraTrans)
{
	pos += vector3f(layout.offset.x, -layout.offset.y, 0.f);

	if (layout.body->GetType() != SystemBody::TYPE_GRAVPOINT) {
		matrix4x4f bodyTrans = matrix4x4f::Identity();
		bodyTrans.Translate(pos);
		bodyTrans.Scale(layout.radius);
		m_renderer->SetTransform(cameraTrans * bodyTrans);

		// Drawables::GetAxes3DDrawable(m_renderer)->Draw(m_renderer);
		Graphics::Texture *bodyTex = TextureBuilder::Model(layout.body->GetIcon()).GetOrCreateTexture(m_renderer, "bodyicons");
		m_atlasMat->SetTexture(Graphics::Renderer::GetName("texture0"), bodyTex);
		m_bodyIcon->Draw(m_renderer, m_atlasMat.get());

		float pixPerUnit = AtlasViewPixelPerUnit();
		AddProjected<SystemBody>(Projectable::OBJECT, Projectable::SYSTEMBODY, layout.body, vector3d(), layout.radius * pixPerUnit);
	}
	/* else { // gravpoint debugging
		matrix4x4f bodyTrans = matrix4x4f::Identity();
		bodyTrans.Translate(pos - vector3f(layout.offset.x, -layout.offset.y, 0.f));
		m_renderer->SetTransform(cameraTrans * bodyTrans);

		Drawables::GetAxes3DDrawable(m_renderer)->Draw(m_renderer);
	} */

	for (const auto &child : layout.children) {
		RenderAtlasBody(child, pos, cameraTrans);
	}
}

void SystemView::DrawAtlasView()
{
	// Set up the perspective projection for the background stars
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, m_renderer->GetDisplayAspect(), 1.f, 1500.f);

	// Background is rotated around (0,0,0), adjusted for parallax effect, and drawn
	matrix4x4d trans2bg = matrix4x4d::Identity();

	// parallax effect
	trans2bg.Translate(m_atlasPos.x, -m_atlasPos.y, 0.0);
	trans2bg.RotateY(DEG2RAD(m_atlasPos.x * 0.05));

	// rotate, tilt, tilt
	trans2bg.RotateZ(DEG2RAD(35.f));
	trans2bg.RotateX(DEG2RAD(-5.f));
	trans2bg.RotateY(DEG2RAD(-45.f));

	auto *background = m_game->GetSpace()->GetBackground();
	background->SetIntensity(0.6);
	if (!m_game->IsNormalSpace() || !m_viewingCurrentSystem) {
		Uint32 cachedFlags = background->GetDrawFlags();
		background->SetDrawFlags(Background::Container::DRAW_SKYBOX);
		background->Draw(trans2bg);
		background->SetDrawFlags(cachedFlags);
	} else {
		background->Draw(trans2bg);
	}

	m_renderer->ClearDepthBuffer();

	// Pick an orthographic projection scale that has the same apparent size as a perspective projection at 30m.
	m_renderer->SetProjection(matrix4x4f::OrthoMatrix(m_atlasViewW * m_atlasZoom, m_atlasViewH * m_atlasZoom, 1.0, 1000.0));

	matrix4x4f cameraTrans = matrix4x4f::Identity();
	cameraTrans.Translate(vector3f(m_atlasPos.x, -m_atlasPos.y, -10.0));

	matrix4x4f gridTransform = matrix4x4f::Identity();
	gridTransform.Translate(vector3f(0, 0, -1.0f));
	gridTransform.RotateX(M_PI / 2.0);
	gridTransform.Scale(4.0 / float(AU)); // one grid square = one earth diameter = two units
	m_renderer->SetTransform(cameraTrans * gridTransform);
	DrawGrid(64.0);

	// Don't draw bodies in unexplored systems
	if (m_unexplored)
		return;

	// Draw the system atlas layout, offsetting the position to ensure it's roughly centered on the grid
	RenderAtlasBody(m_atlasLayout, vector3f{ -m_atlasPosDefault.x, m_atlasPosDefault.y, 0.0f }, cameraTrans);
}

void SystemView::Update()
{
	const float ft = Pi::GetFrameTime();

	// TODO: add "true" lower/upper bounds to m_zoomTo / m_zoom
	m_zoomTo = Clamp(m_zoomTo, MIN_ZOOM, MAX_ZOOM);
	m_zoom = Clamp(m_zoom, MIN_ZOOM, MAX_ZOOM);
	m_atlasZoomTo = Clamp(m_atlasZoomTo, MIN_ATLAS_ZOOM, MAX_ATLAS_ZOOM);

	auto prevAtlasZoom = m_atlasZoom;
	// Since m_zoom changes over multiple orders of magnitude, any fixed linear factor will not be appropriate
	// at some of them.
	AnimationCurves::Approach(m_zoom, m_zoomTo, ft, 10.f, m_zoomTo / 60.f);
	AnimationCurves::Approach(m_atlasZoom, m_atlasZoomTo, ft, 10.f, m_atlasZoomTo / 60.f);

	AnimationCurves::Approach(m_rot_x, m_rot_x_to, ft);
	AnimationCurves::Approach(m_rot_y, m_rot_y_to, ft);

	AnimationCurves::Approach(m_atlasPos.x, m_atlasPosTo.x, ft);
	AnimationCurves::Approach(m_atlasPos.y, m_atlasPosTo.y, ft);

	// make panning so that the zoom occurs on the mouse cursor
	if (prevAtlasZoom != m_atlasZoom) {
		// FIXME The ImGui method one frame out of date
		// either add the appropriate method to Input or start the pigui frame earlier
		auto mpos = ImGui::GetMousePos();
		mpos.x = Clamp(mpos.x, 0.f, float(Graphics::GetScreenWidth()));
		mpos.y = Clamp(mpos.y, 0.f, float(Graphics::GetScreenHeight()));
		auto cpos = vector2f(mpos.x, mpos.y) - vector2f(Graphics::GetScreenWidth() / 2, Graphics::GetScreenHeight() / 2);
		auto shift = cpos * (m_atlasZoom - prevAtlasZoom) * m_atlasViewH / Graphics::GetScreenHeight();
		m_atlasPosTo += shift;
		m_atlasPos = m_atlasPosTo;
	}

	// to capture mouse when button was pressed and release when released
	if (Pi::input->MouseButtonState(SDL_BUTTON_MIDDLE) != m_rotateWithMouseButton) {
		m_rotateWithMouseButton = !m_rotateWithMouseButton;
		Pi::input->SetCapturingMouse(m_rotateWithMouseButton);
	}

	if (m_rotateWithMouseButton || m_rotateView) {
		int motion[2];
		Pi::input->GetMouseMotion(motion);
		if (m_displayMode == Mode::Orrery) {
			m_rot_x_to += motion[1] * 20 * ft;
			m_rot_y_to += motion[0] * 20 * ft;
		} else {
			const double pixToUnits = Graphics::GetScreenHeight() / m_atlasViewH;
			constexpr float mouseAcceleration = 1.5f;
			m_atlasPosTo.x += motion[0] * m_atlasZoom / pixToUnits * mouseAcceleration;
			m_atlasPosTo.y += motion[1] * m_atlasZoom / pixToUnits * mouseAcceleration;
		}
	} else if (m_zoomView) {
		Pi::input->SetCapturingMouse(true);
		int motion[2];
		Pi::input->GetMouseMotion(motion);
		if (m_displayMode == Mode::Orrery)
			m_zoomTo *= pow(ZOOM_IN_SPEED * 0.003 + 1, -motion[1]);
		else
			m_atlasZoomTo += motion[1] * 0.005 * Clamp(m_atlasZoomTo, MIN_ATLAS_ZOOM, MAX_ATLAS_ZOOM);
	}

	// camera control signals from devices, sent to the SectorView
	if (m_input.mapViewZoom->IsActive()) {
		if (m_displayMode == Mode::Orrery)
			m_zoomTo *= pow(ZOOM_IN_SPEED * 0.006 + 1, m_input.mapViewZoom->GetValue());
		else
			m_atlasZoomTo -= m_input.mapViewZoom->GetValue() * ATLAS_SCROLL_SENS;
	}
	if (m_input.mapViewYaw->IsActive())
		m_rot_y_to += m_input.mapViewYaw->GetValue() * ft * 60;
	if (m_input.mapViewPitch->IsActive())
		m_rot_x_to += m_input.mapViewPitch->GetValue() * ft * 60;

	m_rot_x_to = Clamp(m_rot_x_to, -80.0f, 80.0f);

	if (m_shipDrawing != OFF) {
		RefreshShips();
		// if we are attached to the ship, check if we not deleted it in the previous frame
		if (m_viewedObject.type != Projectable::NONE && m_viewedObject.base == Projectable::SHIP) {
			auto bs = m_game->GetSpace()->GetBodies();
			if (std::find(bs.begin(), bs.end(), m_viewedObject.ref.body) == bs.end())
				ResetViewpoint();
		}
		if (m_selectedObject.type != Projectable::NONE && m_selectedObject.base == Projectable::SHIP) {
			auto bs = m_game->GetSpace()->GetBodies();
			if (std::find(bs.begin(), bs.end(), m_selectedObject.ref.body) == bs.end())
				m_selectedObject.type = Projectable::NONE;
		}
	}
}

void SystemView::MouseWheel(bool up)
{
	if (this != Pi::GetView())
		return;

	if (m_displayMode == Mode::Orrery) {
		if (!up)
			m_zoomTo *= 1 / ((ZOOM_OUT_SPEED - 1) * WHEEL_SENSITIVITY + 1) / Pi::GetMoveSpeedShiftModifier();
		else
			m_zoomTo *= ((ZOOM_IN_SPEED - 1) * WHEEL_SENSITIVITY + 1) * Pi::GetMoveSpeedShiftModifier();
	} else {
		m_atlasZoomTo -= (up ? 1.0 : -1.0) * ATLAS_SCROLL_SENS;
	}
}

void SystemView::RefreshShips(void)
{
	m_contacts.clear();
	auto bs = m_game->GetSpace()->GetBodies();
	for (auto s = bs.begin(); s != bs.end(); s++) {
		if ((*s) != Pi::player &&
			(*s)->GetType() == ObjectType::SHIP) {

			const auto c = static_cast<Ship *>(*s);
			m_contacts.push_back(std::make_pair(c, c->ComputeOrbit()));
		}
	}
}

void SystemView::DrawShips(const double t, const vector3d &offset)
{
	// offset - translate vector to selected object, scaled to camera scale
	for (auto s = m_contacts.begin(); s != m_contacts.end(); s++) {
		vector3d pos(0.0);
		CalculateShipPositionAtTime((*s).first, (*s).second, t, pos);
		pos = pos + offset;
		//draw highlighted orbit for selected ship
		const bool isSelected = m_selectedObject.type == Projectable::OBJECT && m_selectedObject.base != Projectable::SYSTEMBODY && m_selectedObject.ref.body == (*s).first;
		AddProjected<Body>(Projectable::OBJECT, Projectable::SHIP, static_cast<Body *>((*s).first), pos);
		if (m_shipDrawing == ORBITS && (*s).first->GetFlightState() == Ship::FlightState::FLYING) {
			vector3d framepos(0.0);
			CalculateFramePositionAtTime((*s).first->GetFrame(), m_time, framepos);
			PutOrbit<Body>(Projectable::SHIP, static_cast<Body *>((*s).first), &(*s).second, offset + framepos, isSelected ? svColor[SELECTED_SHIP_ORBIT] : svColor[SHIP_ORBIT], 0);
		}
	}
}

void SystemView::DrawGrid(uint32_t radius)
{
	int half_lines = radius + 1;

	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, half_lines * 4 + (m_gridDrawing == GridDrawing::GRID_AND_LEGS ? m_projected.size() * 2 : 0)));

	float zoom = float(AU);
	vector3d pos = m_trans;

	for (int i = -half_lines; i < half_lines + 1; i++) {
		float z = float(i) * zoom;
		m_lineVerts->Add(vector3f(-half_lines * zoom, 0.0f, z) + vector3f(pos), svColor[GRID]);
		m_lineVerts->Add(vector3f(+half_lines * zoom, 0.0f, z) + vector3f(pos), svColor[GRID]);
	}

	for (int i = -half_lines; i < half_lines + 1; i++) {
		float x = float(i) * zoom;
		m_lineVerts->Add(vector3f(x, 0.0f, -half_lines * zoom) + vector3f(pos), svColor[GRID]);
		m_lineVerts->Add(vector3f(x, 0.0f, +half_lines * zoom) + vector3f(pos), svColor[GRID]);
	}

	if (m_gridDrawing == GridDrawing::GRID_AND_LEGS)
		for (Projectable &p : m_projected) {
			vector3d offset(p.worldpos);
			offset.y = m_trans.y;
			m_lineVerts->Add(vector3f(p.worldpos), svColor[GRID_LEG] * 0.5);
			m_lineVerts->Add(vector3f(offset), svColor[GRID_LEG] * 0.5);
		}

	m_lines.SetData(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);
	m_lines.Draw(Pi::renderer, m_gridMat.get());
}

template <typename T>
void SystemView::AddProjected(Projectable::types type, Projectable::bases base, T *ref, const vector3d &worldpos, float screensize)
{
	vector3d pos = Graphics::ProjectToScreen(m_renderer, worldpos);
	if (pos.z > 0.0) return; // reject back-projected objects
	pos.y = m_renderer->GetViewport().h - pos.y;

	Projectable p(type, base, ref);
	p.screenpos = pos;
	p.screensize = screensize;
	p.worldpos = worldpos;
	m_projected.push_back(p);
}

void SystemView::SetVisibility(std::string param)
{
	if (param == "RESET_VIEW")
		ResetViewpoint();
	else if (param == "GRID_OFF")
		m_gridDrawing = GridDrawing::OFF;
	else if (param == "GRID_ON")
		m_gridDrawing = GridDrawing::GRID;
	else if (param == "GRID_AND_LEGS")
		m_gridDrawing = GridDrawing::GRID_AND_LEGS;
	else if (param == "LAG_OFF")
		m_showL4L5 = LAG_OFF;
	else if (param == "LAG_ICON")
		m_showL4L5 = LAG_ICON;
	else if (param == "LAG_ICONTEXT")
		m_showL4L5 = LAG_ICONTEXT;
	else if (param == "SHIPS_OFF") {
		m_shipDrawing = OFF;
		// if we are attached to the ship, reset view, since the ship was hidden
		if (m_selectedObject.type != Projectable::NONE && m_selectedObject.base == Projectable::SHIP)
			m_selectedObject.type = Projectable::NONE;
		if (m_viewedObject.type != Projectable::NONE && m_viewedObject.base == Projectable::SHIP)
			ResetViewpoint();
	} else if (param == "SHIPS_ON")
		m_shipDrawing = BOXES;
	else if (param == "SHIPS_ORBITS")
		m_shipDrawing = ORBITS;
	else
		Output("Unknown visibility: %s\n", param.c_str());
}

void SystemView::SetZoomMode(bool enable)
{
	if (enable != m_zoomView) {
		Pi::input->SetCapturingMouse(enable);
		m_zoomView = enable;
		if (m_zoomView) m_rotateView = false;
	}
}

void SystemView::SetRotateMode(bool enable)
{
	if (enable != m_rotateView) {
		Pi::input->SetCapturingMouse(enable);
		m_rotateView = enable;
		if (m_rotateView) m_zoomView = false;
	}
}

Projectable *SystemView::GetSelectedObject()
{
	return &m_selectedObject;
}

void SystemView::SetSelectedObject(Projectable::types type, Projectable::bases base, SystemBody *sb)
{
	m_selectedObject.type = type;
	m_selectedObject.base = base;
	m_selectedObject.ref.sbody = sb;
}

void SystemView::SetSelectedObject(Projectable::types type, Projectable::bases base, Body *b)
{
	m_selectedObject.type = type;
	m_selectedObject.base = base;
	m_selectedObject.ref.body = b;
}

void SystemView::ClearSelectedObject()
{
	m_selectedObject.type = Projectable::NONE;
}

void SystemView::ViewSelectedObject()
{
	// we will immediately determine the coordinates of the viewed body so that
	// there is a correct starting point of the transition animation, otherwise
	// there may be an unwanted shift in the next frame
	m_viewedObject = m_selectedObject;
	GetTransformTo(m_viewedObject, m_transTo);
	m_animateTransition = MAX_TRANSITION_FRAMES;
}

double SystemView::ProjectedSize(double size, vector3d pos)
{
	matrix4x4d dtrans = matrix4x4d(m_cameraSpace);
	pos = dtrans * pos; //position in camera space to know distance
	double result = size / pos.Length() / CAMERA_FOV_RADIANS;
	return result;
}

float SystemView::GetZoom() const
{
	return m_displayMode == Mode::Orrery ? 1.0 / m_zoom : m_atlasZoom;
}

double SystemView::GetOrbitTime(double t, const SystemBody *b) { return t; }
double SystemView::GetOrbitTime(double t, const Body *b) { return t - m_game->GetTime(); }
void SystemView::OnSwitchFrom() { m_projected.clear(); } // because ships from the previous system may remain after last update
float SystemView::AtlasViewPixelPerUnit()
{
	return Graphics::GetScreenHeight() / (m_atlasViewH * m_atlasZoom);
}
