// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemView.h"

#include "AnimationCurves.h"
#include "Background.h"
#include "Game.h"
#include "Input.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Space.h"
#include "gameconsts.h"
#include "matrix4x4.h"
#include "core/Log.h"

#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "galaxy/SystemPath.h"
#include "galaxy/SystemBody.h"

#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"

#include "imgui/imgui.h"
#include "SDL_keycode.h"

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

namespace {
	static bool too_near(const vector3f &a, const vector3f &b, const vector2f &gain, float width)
	{
		return std::abs(a.x - b.x) < gain.x && std::abs(a.y - b.y) < gain.y
			// Gliese852:
			// we don’t want to group objects that simply overlap and are located at different distances
			// therefore, depth is also taken into account, we have z_NDC (normalized device coordinates)
			// in order to make a strict translation of delta z_NDC into delta "pixels", one also needs to know
			// the z coordinates in the camera space. I plan to implement this later
			// at the moment I just picked up a number that works well (6.0)
			&& std::abs(a.z - b.z) * width * 6.0 < gain.x;
	}
}

// ─── System View ─────────────────────────────────────────────────────────────

SystemView::SystemView(Game *game) :
	PiGuiView("system-view"),
	m_game(game),
	m_displayMode(Mode::Orrery),
	m_viewingCurrentSystem(false),
	m_unexplored(true)
{
	m_map.reset(new SystemMapViewport(Pi::GetApp()));
	m_map->GetStarportHeightAboveTerrain = sigc::mem_fun(this, &SystemView::CalculateStarportHeight);

	RefreshShips();
	m_planner = Pi::planner;
}

SystemView::~SystemView()
{
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

double SystemView::CalculateStarportHeight(const SystemBody *body)
{
	if (m_viewingCurrentSystem)
		// if we look at the current system, the relief is known, we take the height from the physical body
		return m_game->GetSpace()->FindBodyForPath(&(body->GetPath()))->GetPosition().Length();
	else
		// if the remote system - take the radius of the planet
		return body->GetParent()->GetRadius();
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

void SystemView::AddShipTracks(double time)
{
	using Col = SystemMapViewport::ColorIndex;

	// offset - translate vector to selected object, scaled to camera scale
	for (auto s = m_contacts.begin(); s != m_contacts.end(); s++) {
		vector3d pos(0.0);
		CalculateShipPositionAtTime((*s).first, (*s).second, time, pos);
		//draw highlighted orbit for selected ship
		Projectable p = { Projectable::OBJECT, Projectable::SHIP, s->first, pos };
		const bool isSelected = *m_map->GetSelectedObject() == p;
		m_map->AddObjectTrack(p);

		if (m_map->GetShipDrawing() == ORBITS && s->first->GetFlightState() == Ship::FlightState::FLYING) {
			const Color orbitColor = isSelected ? m_map->svColor[Col::SELECTED_SHIP_ORBIT] : m_map->svColor[Col::SHIP_ORBIT];
			vector3d framepos(0.0);
			CalculateFramePositionAtTime(s->first->GetFrame(), time, framepos);
			m_map->AddOrbitTrack({ Projectable::ORBIT, Projectable::SHIP, s->first, framepos }, &s->second, orbitColor, 0);
		}
	}
}

void SystemView::Update()
{
	const float ft = Pi::GetFrameTime();
	m_map->SetReferenceTime(m_game->GetTime());

	SystemPath path = m_game->GetSectorView()->GetSelected().SystemOnly();
	m_viewingCurrentSystem = m_game->IsNormalSpace() && m_game->GetSpace()->GetStarSystem()->GetPath().IsSameSystem(path);

	RefCountedPtr<StarSystem> system = m_map->GetCurrentSystem();
	if (!system || (system->GetUnexplored() != m_unexplored || !system->GetPath().IsSameSystem(path))) {
		system = m_game->GetGalaxy()->GetStarSystem(path);
		m_unexplored = system->GetUnexplored();
		m_map->SetCurrentSystem(system);
	}

	m_map->Update(ft);
	m_map->HandleInput(ft);

	if (m_map->GetShipDrawing() != OFF) {
		RefreshShips();

		Projectable *viewed = m_map->GetViewedObject();
		// if we are attached to the ship, check if we not deleted it in the previous frame
		if (viewed->type != Projectable::NONE && viewed->base == Projectable::SHIP) {
			auto bs = m_game->GetSpace()->GetBodies();
			if (std::find(bs.begin(), bs.end(), viewed->ref.body) == bs.end())
				m_map->ResetViewpoint();
		}

		Projectable *selected = m_map->GetSelectedObject();
		if (selected->type != Projectable::NONE && selected->base == Projectable::SHIP) {
			auto bs = m_game->GetSpace()->GetBodies();
			if (std::find(bs.begin(), bs.end(), selected->ref.body) == bs.end())
				selected->type = Projectable::NONE;
		}
	}

	if (m_game->IsNormalSpace() && m_viewingCurrentSystem) {
		using Col = SystemMapViewport::ColorIndex;

		double time = m_map->GetTime();

		// draw ships
		if (m_map->GetShipDrawing() != OFF) {
			AddShipTracks(time);
		}

		// draw player and planner
		vector3d ppos(0.0);
		Orbit playerOrbit = Pi::player->ComputeOrbit();
		Body *PlayerBody = static_cast<Body *>(Pi::player);

		CalculateShipPositionAtTime(static_cast<Ship *>(Pi::player), playerOrbit, time, ppos);
		m_map->AddObjectTrack({ Projectable::OBJECT, Projectable::PLAYER, PlayerBody, ppos });

		FrameId playerNonRotFrameId = Frame::GetFrame(PlayerBody->GetFrame())->GetNonRotFrame();
		Frame *playerNonRotFrame = Frame::GetFrame(playerNonRotFrameId);
		SystemBody *playerAround = playerNonRotFrame->GetSystemBody();

		vector3d offset(0.0);
		CalculateFramePositionAtTime(playerNonRotFrameId, time, offset);

		if (Pi::player->GetFlightState() == Ship::FlightState::FLYING) {
			const double planetRadius = playerAround->GetRadius();
			m_map->AddOrbitTrack({ Projectable::ORBIT, Projectable::PLAYER, PlayerBody, offset }, &playerOrbit, m_map->svColor[Col::PLAYER_ORBIT], planetRadius);

			const double plannerStartTime = m_planner->GetStartTime();
			if (!m_planner->GetPosition().ExactlyEqual(vector3d(0, 0, 0))) {
				Orbit plannedOrbit = Orbit::FromBodyState(m_planner->GetPosition(),
					m_planner->GetVel(),
					playerAround->GetMass());

				m_map->AddOrbitTrack({ Projectable::ORBIT, Projectable::PLANNER, PlayerBody, offset }, &plannedOrbit, m_map->svColor[Col::PLANNER_ORBIT], planetRadius);
				if (std::fabs(time - m_game->GetTime()) > 1. && (time - plannerStartTime) > 0.)
					m_map->AddObjectTrack({ Projectable::OBJECT, Projectable::PLANNER, PlayerBody, offset + plannedOrbit.OrbitalPosAtTime(time - plannerStartTime) });
				else
					m_map->AddObjectTrack({ Projectable::OBJECT, Projectable::PLANNER, PlayerBody, offset + m_planner->GetPosition() });
			}
		}
	}
}

void SystemView::Draw3D()
{
	PROFILE_SCOPED()
	m_renderer->ClearScreen();

	auto *background = m_game->GetSpace()->GetBackground();
	Uint32 cachedFlags = background->GetDrawFlags();

	if (!m_game->IsNormalSpace() || !m_viewingCurrentSystem)
		background->SetDrawFlags(Background::Container::DRAW_SKYBOX);

	m_map->SetBackground(background);

	m_map->SetDisplayMode(m_displayMode);
	m_map->Draw3D();

	background->SetDrawFlags(cachedFlags);
}

void SystemView::OnSwitchFrom()
{
	// because ships from the previous system may remain after last update
	// m_projected.clear();
}

// ─── System Map Input ────────────────────────────────────────────────────────

void SystemMapViewport::InputBindings::RegisterBindings()
{
	mapViewPitch = AddAxis("BindMapViewPitch");
	mapViewYaw = AddAxis("BindMapViewYaw");
	mapViewZoom = AddAxis("BindMapViewZoom");
}

// ─── System Map Display ──────────────────────────────────────────────────────

SystemMapViewport::SystemMapViewport(GuiApplication *app) :
	m_input(app->GetInput()),
	m_app(app),
	m_renderer(app->GetRenderer()),
	m_displayMode(SystemView::Mode::Orrery),
	m_showGravpoints(false),
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

	m_lineMat.reset(m_renderer->CreateMaterial("vtxColor", lineMatDesc, rsd)); //m_renderer not set yet

	rsd.primitiveType = Graphics::LINE_SINGLE;
	m_gridMat.reset(m_renderer->CreateMaterial("vtxColor", lineMatDesc, rsd));

	m_realtime = true;

	ResetViewpoint();

	m_orbitVts.reset(new vector3f[N_VERTICES_MAX + 1]);
	m_orbitColors.reset(new Color[N_VERTICES_MAX + 1]);
}

SystemMapViewport::~SystemMapViewport()
{
}

void SystemMapViewport::AccelerateTime(float step)
{
	m_realtime = false;
	m_timeStep = step;
}

void SystemMapViewport::SetRealTime()
{
	m_realtime = true;
}

void SystemMapViewport::ResetViewpoint()
{
	m_viewedObject.type = Projectable::NONE;
	m_rot_y_to = 0;
	m_rot_x_to = 50;
	m_zoomTo = 1.0f / float(AU);
	m_timeStep = 1.0f;
	m_time = m_refTime;
	m_transTo *= 0.0;
	m_animateTransition = MAX_TRANSITION_FRAMES;

	m_viewedObject = {};

	if (!m_renderer) return;

	float height = tan(DEG2RAD(CAMERA_FOV)) * 30.0f;
	m_atlasViewW = height * m_renderer->GetDisplayAspect();
	m_atlasViewH = height;

	bool hasChildren = !m_atlasLayout.children.empty();
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

RefCountedPtr<StarSystem> SystemMapViewport::GetCurrentSystem()
{
	return m_system;
}

void SystemMapViewport::SetCurrentSystem(RefCountedPtr<StarSystem> system)
{
	ClearSelectedObject();

	m_system = system;

	if (m_system) {
		SystemBody *body = m_system->GetRootBody().Get();
		m_atlasLayout = {};
		m_atlasLayout.isVertical = body->GetType() == SystemBody::TYPE_GRAVPOINT;
		LayoutSystemBody(body, m_atlasLayout);
	}

	ResetViewpoint();
}

void SystemMapViewport::RenderOrbit(Projectable p, const ProjectedOrbit *orbitData, const vector3d &offset)
{
	PROFILE_SCOPED()

	double ecc = orbitData->orbit.GetEccentricity();
	double timeshift = ecc > 0.6 ? 0.0 : 0.5;
	double maxT = 1.;
	unsigned short num_vertices = 0;
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = (double(i) + timeshift) / double(N_VERTICES_MAX);
		const vector3d pos = orbitData->orbit.EvenSpacedPosTrajectory(t);
		if (pos.Length() < orbitData->planetRadius) {
			maxT = t;
			break;
		}
	}

	static const float startTrailPercent = 0.85;
	static const float fadedColorParameter = 0.8;

	Uint16 fadingColors = 0;
	const double tMinust0 = p.base == Projectable::SYSTEMBODY ? m_time : m_time - m_refTime;
	for (unsigned short i = 0; i < N_VERTICES_MAX; ++i) {
		const double t = (double(i) + timeshift) / double(N_VERTICES_MAX) * maxT;
		if (fadingColors == 0 && t >= startTrailPercent * maxT)
			fadingColors = i;
		const vector3d pos = orbitData->orbit.EvenSpacedPosTrajectory(t, tMinust0);
		m_orbitVts[i] = vector3f(offset + pos);
		++num_vertices;
		if (pos.Length() < orbitData->planetRadius)
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
		const Color fadedColor = orbitData->color * fadedColorParameter;
		std::fill_n(m_orbitColors.get(), num_vertices, fadedColor);
		const Uint16 trailLength = num_vertices - fadingColors;
		for (Uint16 currentColor = 0; currentColor < trailLength; ++currentColor) {
			float scalingParameter = (1.f - static_cast<float>(currentColor) / (trailLength - 1));
			m_orbitColors[currentColor + fadingColors] = fadedColor * scalingParameter;
		}

		m_orbits.SetData(num_vertices, m_orbitVts.get(), m_orbitColors.get());
		m_orbits.Draw(m_renderer, m_lineMat.get());
	}

	AddProjected(p, Projectable::PERIAPSIS, offset + orbitData->orbit.Perigeum());
	AddProjected(p, Projectable::APOAPSIS, offset + orbitData->orbit.Apogeum());

	if (p.base != Projectable::SYSTEMBODY || !p.getRef())
		return;

	const SystemBody::BodySuperType bst = p.ref.sbody->GetSuperType();
	const bool showLagrange = (bst == SystemBody::SUPERTYPE_ROCKY_PLANET || bst == SystemBody::SUPERTYPE_GAS_GIANT);

	if (showLagrange && m_showL4L5 != LAG_OFF) {
		const vector3d posL4 = orbitData->orbit.EvenSpacedPosTrajectory((1.0 / 360.0) * 60.0, tMinust0);
		AddProjected(p, Projectable::L4, offset + posL4);

		const vector3d posL5 = orbitData->orbit.EvenSpacedPosTrajectory((1.0 / 360.0) * 300.0, tMinust0);
		AddProjected(p, Projectable::L5, offset + posL5);
	}
}

// returns the position of the ground spaceport relative to the center of the planet at the specified time
static vector3d position_of_surface_starport_relative_to_parent(const SystemBody *starport, double time, double radius)
{
	const SystemBody *parent = starport->GetParent();
	// planet axis tilt
	return matrix3x3d::RotateX(parent->GetAxialTilt()) *
		// the angle the planet has turned since the beginning of time
		matrix3x3d::RotateY(-2 * M_PI / parent->GetRotationPeriod() * time + parent->GetRotationPhaseAtStart()) *
		// the original coordinates of the starport are saved as a 3x3 matrix,
		starport->GetOrbit().GetPlane() *
		// to get the direction to the station, you need to multiply them by 0.0, 1.0, 0.0
		vector3d(0.0, 1.0, 0.0) * radius;
}

void SystemMapViewport::AddBodyTrack(const SystemBody *b, const vector3d &offset)
{
	if (b->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
		return;

	if (b->GetType() != SystemBody::TYPE_GRAVPOINT || m_showGravpoints)
		AddObjectTrack({ Projectable::OBJECT, Projectable::SYSTEMBODY, b, offset });

	// perfect-knowledge abstraction: track all child bodies
	if (b->HasChildren()) {
		for (const SystemBody *kid : b->GetChildren()) {
			if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
				// we need the distance to the center of the planet
				double radius = GetStarportHeightAboveTerrain ? GetStarportHeightAboveTerrain(kid) : b->GetRadius();

				AddObjectTrack({ Projectable::OBJECT, Projectable::SYSTEMBODY, kid,
					offset + position_of_surface_starport_relative_to_parent(kid, m_time, radius) });
				continue;
			}

			if (!is_zero_general(kid->GetOrbit().GetSemiMajorAxis())) {
				// Add the body's orbit
				Projectable p(Projectable::ORBIT, Projectable::SYSTEMBODY, kid);
				p.worldpos = offset;
				AddOrbitTrack(p, &kid->GetOrbit(), svColor[SYSTEMBODY_ORBIT], 0.0);
			}

			// not using current time yet
			AddBodyTrack(kid, offset + kid->GetOrbit().OrbitalPosAtTime(m_time));
		}
	}
}

void SystemMapViewport::RenderBody(const SystemBody *b, const vector3d &position, const matrix4x4f &trans)
{
	if (b->GetSuperType() == SystemBody::SUPERTYPE_STARPORT)
		return;

	const double radius = b->GetRadius();

	matrix4x4f invRot = trans;
	invRot.ClearToRotOnly();
	invRot.Renormalize();
	invRot = invRot.Inverse();

	// Bodies are pre-transformed in double precision to use 0,0,0 as view-space center
	matrix4x4f bodyTrans = trans;
	bodyTrans.Translate(vector3f(position));
	bodyTrans.Scale(radius);

	m_renderer->SetTransform(bodyTrans * invRot);
	m_bodyMat->diffuse = svColor[SYSTEMBODY];
	m_bodyIcon->Draw(m_renderer, m_bodyMat.get());
}

void SystemMapViewport::Draw3D()
{
	PROFILE_SCOPED()
	m_viewportSize = m_renderer->GetViewport();

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

	if (m_displayMode == SystemView::Mode::Orrery)
		DrawOrreryView();
	else
		DrawAtlasView();
}

void SystemMapViewport::DrawOrreryView()
{
	PROFILE_SCOPED()

	// Set up the perspective projection for the background stars
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, float(m_viewportSize.w) / m_viewportSize.h, 1.f, 1500.f);

	// Background is rotated around (0,0,0) and drawn
	matrix4x4d trans2bg = matrix4x4d::Identity();
	trans2bg.RotateX(DEG2RAD(-m_rot_x));
	trans2bg.RotateY(DEG2RAD(-m_rot_y));

	m_background->SetIntensity(0.6);
	m_background->Draw(trans2bg);

	if (!m_system)
		return;

	m_renderer->ClearDepthBuffer();

	// We need to adjust the "far" cutoff plane, so that at high magnifications you can see
	// distant objects in the background.
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, float(m_viewportSize.w) / m_viewportSize.h, 1.f, 1000.f * m_zoom * float(AU) + DEFAULT_VIEW_DISTANCE * 2);
	//TODO add reserve

	double surfaceDistance = 0.0;
	if (m_viewedObject.type == Projectable::OBJECT && m_viewedObject.base == Projectable::SYSTEMBODY && m_viewedObject.getRef())
		surfaceDistance += m_viewedObject.ref.sbody->GetRadius();

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
	// The surface distance translation ensures the view does not go inside of a planet.
	// This matrix operation is read in reverse order (bottom-to-top).
	m_cameraSpace = matrix4x4f::Identity();
	m_cameraSpace.Translate(0, 0, -DEFAULT_VIEW_DISTANCE);
	m_cameraSpace.Translate(0, 0, -surfaceDistance * m_zoom); // apply scale from m_zoom
	m_cameraSpace.Rotate(DEG2RAD(m_rot_x), 1, 0, 0);
	m_cameraSpace.Rotate(DEG2RAD(m_rot_y), 0, 1, 0);
	m_cameraSpace.Scale(m_zoom);
	m_renderer->SetTransform(m_cameraSpace);

	// smooth transition animation
	if (m_animateTransition) {
		// initialize animation target position - m_viewedObject is not guaranteed
		// to have position data until after object tracks are generated
		if (m_animateTransition == MAX_TRANSITION_FRAMES)
			m_transTo = m_viewedObject.worldpos;

		// since the object being approached can move, we need to compensate for its movement
		// making an imprint of the old value (from previous frame)
		m_trans -= m_transTo;
		// calculate the new value
		// GetTransformTo(m_viewedObject, m_transTo);
		m_transTo = m_viewedObject.worldpos;
		// now the difference between the new and the old value is added to m_trans
		m_trans += m_transTo;
		const float ft = m_app->DeltaTime();
		m_animateTransition--;
		AnimationCurves::Approach(m_trans.x, m_transTo.x, ft);
		AnimationCurves::Approach(m_trans.y, m_transTo.y, ft);
		AnimationCurves::Approach(m_trans.z, m_transTo.z, ft);
	} else {
		m_trans = m_viewedObject.worldpos;
	}

	// Transform all tracks + render system bodies
	for (auto &track : m_objectTracks) {
		m_renderer->SetTransform(m_cameraSpace);
		const vector3d viewpos = track.worldpos - m_trans;

		if (track.type == Projectable::ORBIT) {
			ProjectedOrbit *orbitData = &m_orbitTracks[track.orbitIdx];
			const double axisZoom = orbitData->orbit.GetSemiMajorAxis();

			//semimajor axis radius should be at least 1% of screen width to show the orbit
			//FIXME: this has never worked, the returned size is not in screen %
			if (ProjectedSize(axisZoom, viewpos) > 0.01) {
				RenderOrbit(track, orbitData, viewpos);
			}
		} else {
			AddProjected(track, track.type, viewpos);

			if (track.type == Projectable::OBJECT && track.base == Projectable::SYSTEMBODY) {
				RenderBody(track.ref.sbody, viewpos, m_cameraSpace);
			}
		}
	}

	m_renderer->SetTransform(m_cameraSpace);

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
void SystemMapViewport::LayoutSystemBody(SystemBody *body, AtlasBodyLayout &layout)
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

void SystemMapViewport::RenderAtlasBody(const AtlasBodyLayout &layout, vector3f pos, const matrix4x4f &cameraTrans)
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
		AddProjected({ Projectable::OBJECT, Projectable::SYSTEMBODY, layout.body }, Projectable::OBJECT, vector3d(), layout.radius * pixPerUnit);
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

void SystemMapViewport::DrawAtlasView()
{
	// Set up the perspective projection for the background stars
	m_renderer->SetPerspectiveProjection(CAMERA_FOV, float(m_viewportSize.w) / m_viewportSize.h, 1.f, 1500.f);

	// Background is rotated around (0,0,0), adjusted for parallax effect, and drawn
	matrix4x4d trans2bg = matrix4x4d::Identity();

	// parallax effect
	trans2bg.Translate(m_atlasPos.x, -m_atlasPos.y, 0.0);
	trans2bg.RotateY(DEG2RAD(m_atlasPos.x * 0.05));

	// rotate, tilt, tilt
	trans2bg.RotateZ(DEG2RAD(35.f));
	trans2bg.RotateX(DEG2RAD(-5.f));
	trans2bg.RotateY(DEG2RAD(-45.f));

	m_background->SetIntensity(0.6);
	m_background->Draw(trans2bg);

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
	if (m_system->GetUnexplored())
		return;

	// Draw the system atlas layout, offsetting the position to ensure it's roughly centered on the grid
	RenderAtlasBody(m_atlasLayout, vector3f{ -m_atlasPosDefault.x, m_atlasPosDefault.y, 0.0f }, cameraTrans);
}

void SystemMapViewport::HandleInput(float ft)
{
	Input::Manager *inputMgr = m_app->GetInput();

	// to capture mouse when button was pressed and release when released
	if (inputMgr->MouseButtonState(SDL_BUTTON_MIDDLE) != m_rotateWithMouseButton) {
		m_rotateWithMouseButton = !m_rotateWithMouseButton;
		inputMgr->SetCapturingMouse(m_rotateWithMouseButton);
	}

	float speedMod = inputMgr->KeyState(SDLK_LSHIFT) ? 10.f : (inputMgr->KeyState(SDLK_LCTRL) ? 0.1f : 1.f);

	if (m_rotateWithMouseButton || m_rotateView) {
		int motion[2];
		inputMgr->GetMouseMotion(motion);
		if (m_displayMode == SystemView::Mode::Orrery) {
			m_rot_x_to += motion[1] * 20 * ft * speedMod;
			m_rot_y_to += motion[0] * 20 * ft * speedMod;
		} else {
			const double pixToUnits = m_viewportSize.h / m_atlasViewH;
			const float mouseAcceleration = m_rotateWithMouseButton ? 1.0f : 1.5f;
			m_atlasPosTo.x += motion[0] * m_atlasZoom / pixToUnits * mouseAcceleration;
			m_atlasPosTo.y += motion[1] * m_atlasZoom / pixToUnits * mouseAcceleration;
		}
	} else if (m_zoomView) {
		inputMgr->SetCapturingMouse(true);
		int motion[2];
		inputMgr->GetMouseMotion(motion);
		if (m_displayMode == SystemView::Mode::Orrery)
			m_zoomTo *= pow(ZOOM_IN_SPEED * 0.003 * speedMod + 1, -motion[1]);
		else
			m_atlasZoomTo += motion[1] * 0.005 * speedMod * Clamp(m_atlasZoomTo, MIN_ATLAS_ZOOM, MAX_ATLAS_ZOOM);
	}

	// camera control signals from devices, sent to the SectorView
	if (m_input.mapViewZoom->IsActive()) {
		if (m_displayMode == SystemView::Mode::Orrery)
			m_zoomTo *= pow(ZOOM_IN_SPEED * 0.006 * speedMod + 1, m_input.mapViewZoom->GetValue());
		else
			m_atlasZoomTo -= m_input.mapViewZoom->GetValue() * ATLAS_SCROLL_SENS;
	}
	if (m_input.mapViewYaw->IsActive())
		m_rot_y_to += m_input.mapViewYaw->GetValue() * ft * 60;
	if (m_input.mapViewPitch->IsActive())
		m_rot_x_to += m_input.mapViewPitch->GetValue() * ft * 60;

	m_rot_x_to = Clamp(m_rot_x_to, -80.0f, 80.0f);

	int scroll = inputMgr->GetMouseWheel();

	if (scroll && m_displayMode == SystemView::Mode::Orrery) {
		if (scroll < 0)
			m_zoomTo *= 1 / ((ZOOM_OUT_SPEED - 1) * WHEEL_SENSITIVITY * speedMod + 1);
		else
			m_zoomTo *= ((ZOOM_IN_SPEED - 1) * WHEEL_SENSITIVITY * speedMod + 1);
	} else if (scroll) {
		m_atlasZoomTo -= (scroll > 0 ? 1.0 : -1.0) * ATLAS_SCROLL_SENS;
	}
}

void SystemMapViewport::Update(float ft)
{
	m_objectTracks.clear();
	m_orbitTracks.clear();
	m_projected.clear();

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
	// NOTE: if viewportSize is not yet set, divide-by-zero occurs
	if (prevAtlasZoom != m_atlasZoom && m_viewportSize.h > 0) {
		// FIXME The ImGui method one frame out of date
		// either add the appropriate method to Input or start the pigui frame earlier
		// FIXME: atlas code needs to be separated from orrery code
		auto mpos = ImGui::GetMousePos();
		mpos.x = Clamp(mpos.x, 0.f, float(m_viewportSize.w));
		mpos.y = Clamp(mpos.y, 0.f, float(m_viewportSize.h));
		auto cpos = vector2f(mpos.x, mpos.y) - vector2f(m_viewportSize.w / 2.f, m_viewportSize.h / 2.f);
		auto shift = cpos * (m_atlasZoom - prevAtlasZoom) * m_atlasViewH / m_viewportSize.h;
		m_atlasPosTo += shift;
		m_atlasPos = m_atlasPosTo;
	}

	if (m_realtime) {
		m_time = m_refTime;
	} else {
		m_time += m_timeStep * ft;
	}

	if (m_displayMode == SystemView::Mode::Orrery) {
		if (m_system && !m_system->GetUnexplored() && m_system->GetRootBody()) {
			// all systembodies draws here
			AddBodyTrack(m_system->GetRootBody().Get(), vector3d(0, 0, 0));
		}
	}
}

void SystemMapViewport::DrawGrid(uint32_t radius)
{
	int half_lines = radius + 1;

	m_lineVerts.reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION, half_lines * 4 + (m_gridDrawing == GridDrawing::GRID_AND_LEGS ? m_projected.size() * 2 : 0)));

	float zoom = float(AU);
	vector3f pos = vector3f(m_trans);

	for (int i = -half_lines; i < half_lines + 1; i++) {
		float z = float(i) * zoom;
		m_lineVerts->Add(vector3f(-half_lines * zoom, 0.0f, z) - pos, svColor[GRID]);
		m_lineVerts->Add(vector3f(+half_lines * zoom, 0.0f, z) - pos, svColor[GRID]);
	}

	for (int i = -half_lines; i < half_lines + 1; i++) {
		float x = float(i) * zoom;
		m_lineVerts->Add(vector3f(x, 0.0f, -half_lines * zoom) - pos, svColor[GRID]);
		m_lineVerts->Add(vector3f(x, 0.0f, +half_lines * zoom) - pos, svColor[GRID]);
	}

	if (m_gridDrawing == GridDrawing::GRID_AND_LEGS)
		for (Projectable &p : m_objectTracks) {
			vector3d offset(p.worldpos - m_trans);
			offset.y = 0; //-m_trans.y;
			m_lineVerts->Add(vector3f(p.worldpos - m_trans), svColor[GRID_LEG] * 0.5);
			m_lineVerts->Add(vector3f(offset), svColor[GRID_LEG] * 0.5);
		}

	m_lines.SetData(m_lineVerts->GetNumVerts(), &m_lineVerts->position[0], &m_lineVerts->diffuse[0]);
	m_lines.Draw(m_renderer, m_gridMat.get());
}

std::vector<Projectable::GroupInfo> SystemMapViewport::GroupProjectables(vector2f groupThreshold, const std::vector<Projectable> &specialObjects)
{
	PROFILE_SCOPED()

	std::vector<Projectable::GroupInfo> outGroups;
	std::vector<std::pair<int, int>> overlappingSpecial;

	// Loop over all projectables and assemble groups
	for (size_t idx = 0; idx < m_projected.size(); idx++) {
		Projectable &p = m_projected[idx];

		bool intersect = false;
		std::pair<int, int> specialIdx = { -1, -1 };
		for (size_t sidx = 0; sidx < specialObjects.size(); sidx++) {
			if (p == specialObjects[sidx]) {
				// Special objects are evaluated in a second pass to determine
				// which group they should be added to
				specialIdx = { sidx, idx };
				break;
			}
		}

		for (auto &group : outGroups) {
			if (group.type == p.type && too_near(p.screenpos, group.screenpos, groupThreshold, m_viewportSize.w)) {
				// Regular objects are added to a group if intersecting,
				// special objects are reserved for later
				if (specialIdx.first == -1)
					group.tracks.push_back(idx);
				else
					overlappingSpecial.push_back(specialIdx);
				intersect = true;
				break;
			}
		}

		if (!intersect) {
			// object is not intersecting with any group, so it starts a new group
			outGroups.emplace_back(idx, p.screenpos, p.type);
			if (specialIdx.first != -1)
				outGroups.back().setSpecial(specialIdx.first);
		}
	}

	// Insert special objects into groups if overlapping or add new groups
	for (auto [sidx, idx] : overlappingSpecial) {
		Projectable &p = m_projected[idx];
		Projectable::GroupInfo *nearest = nullptr;
		double min_length = 1e64;

		// Find any groups this special object might be overlapping
		for (auto &group : outGroups) {
			double screendist = (group.screenpos - p.screenpos).LengthSqr();
			if (screendist < min_length && group.type == p.type && too_near(p.screenpos, group.screenpos, groupThreshold, m_viewportSize.w)) {
				nearest = &group;
				min_length = screendist;
			}
		}

		if (nearest) {
			// Overlapping a group, mark the group as special and add the object
			nearest->setSpecial(sidx);
			nearest->tracks.push_back(idx);
		} else {
			// Make a new group
			outGroups.emplace_back(idx, p.screenpos, p.type);
			outGroups.back().setSpecial(sidx);
		}
	}

	//shuffle groups so Object groups are drawn after other groups
	std::stable_sort(outGroups.begin(), outGroups.end(), [](const auto &a, const auto &b){ return a.type > b.type; });

	return outGroups;
}

void SystemMapViewport::AddObjectTrack(Projectable p)
{
	m_objectTracks.push_back(p);

	// Projectables are ephemeral, but we need to know about the position of
	// our current selected/viewed track each frame - copy the position from
	// a matching submitted object track

	if (m_selectedObject == p)
		m_selectedObject.worldpos = p.worldpos;

	if (m_viewedObject == p)
		m_viewedObject.worldpos = p.worldpos;
}

void SystemMapViewport::AddOrbitTrack(Projectable p, const Orbit *orbit, Color color, double planetRadius)
{
	p.type = Projectable::ORBIT;
	p.orbitIdx = int(m_orbitTracks.size());
	m_objectTracks.push_back(p);

	m_orbitTracks.push_back({ *orbit, color, planetRadius });
}

void SystemMapViewport::AddProjected(Projectable p, Projectable::types type, const vector3d &transformedPos, float screensize)
{
	vector3d pos = Graphics::ProjectToScreen(m_renderer, transformedPos);
	if (pos.z > 0.0) return; // reject back-projected objects
	pos.y = m_renderer->GetViewport().h - pos.y;

	p.type = type;
	p.screenpos = vector3f(pos);
	p.screensize = screensize;
	m_projected.push_back(p);
}

void SystemMapViewport::SetVisibility(std::string param)
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

void SystemMapViewport::SetZoomMode(bool enable)
{
	if (enable != m_zoomView) {
		m_app->GetInput()->SetCapturingMouse(enable);
		m_zoomView = enable;
		if (m_zoomView) m_rotateView = false;
	}
}

void SystemMapViewport::SetRotateMode(bool enable)
{
	if (enable != m_rotateView) {
		m_app->GetInput()->SetCapturingMouse(enable);
		m_rotateView = enable;
		if (m_rotateView) m_zoomView = false;
	}
}

void SystemMapViewport::ClearSelectedObject()
{
	m_selectedObject.type = Projectable::NONE;
}

void SystemMapViewport::SetViewedObject(Projectable p)
{
	// we will immediately determine the coordinates of the viewed body so that
	// there is a correct starting point of the transition animation, otherwise
	// there may be an unwanted shift in the next frame
	m_viewedObject = p;
	m_animateTransition = MAX_TRANSITION_FRAMES;
}

double SystemMapViewport::ProjectedSize(double size, vector3d pos)
{
	matrix4x4d dtrans = matrix4x4d(m_cameraSpace);
	pos = dtrans * pos; //position in camera space to know distance
	double result = size / pos.Length() / CAMERA_FOV_RADIANS;
	return result;
}

float SystemMapViewport::GetZoom() const
{
	return m_displayMode == SystemView::Mode::Orrery ? 1.0 / m_zoom : m_atlasZoom;
}

float SystemMapViewport::AtlasViewPixelPerUnit()
{
	return m_viewportSize.h / (m_atlasViewH * m_atlasZoom);
}
