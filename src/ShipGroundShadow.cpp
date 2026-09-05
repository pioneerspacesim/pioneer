// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipGroundShadow.h"

#include "Body.h"
#include "Camera.h"
#include "Frame.h"
#include "Game.h"
#include "MathUtil.h"
#include "ModelBody.h"
#include "Pi.h"
#include "Planet.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "SpaceStationType.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"

#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

#include "profiler/Profiler.h"

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

using namespace Graphics;

namespace {
	static const float SHADOW_ALPHA = 0.5f;
	static const float SHADOW_DEPTH_BIAS = 1e-7f;	  // Depth bias for shadows (positive = closer). Composite pass only.
	static const double SHADOW_MAX_ALTITUDE = 2000.0; // m above terrain
	static const double SHADOW_MAX_ANGLE = 80.0;      // Degrees from terrain normal - no shadow when greater (i.e. at sunset)
	static const double SHADOW_FADE_ANGLE = 70.0;     // Degrees from terrain normal - full strength at or below this
	static const double SHADOW_TERRAIN_SAMPLE_WIDTH = 256.0; // m, starting terrain sample size for shadow plane calc

	std::unique_ptr<RenderTarget> s_maskRT;
	std::unique_ptr<Material> s_shadowMat;
	std::unique_ptr<Material> s_compositeMat;
	std::unique_ptr<VertexArray> s_fullscreenVerts;
	int s_rtWidth = 0;
	int s_rtHeight = 0;

	static size_t s_planeNormal = "planeNormal"_hash;
	static size_t s_planeD = "planeD"_hash;
	static size_t s_projectDir = "projectDir"_hash;
	static size_t s_shadowStrength = "shadowStrength"_hash;
	static size_t s_shadowMask = "shadowMask"_hash;
	static size_t s_shadowAlpha = "shadowAlpha"_hash;
	static size_t s_depthBias = "depthBias"_hash;

	// When docked at a ground station, use the artist-authored dock locator plane
	// instead of collision geometry, which can sit above the visible pad surface.
	static bool TryBuildDockShadowPlane(
		const Ship *ship,
		const Planet *planet,
		const FrameId planetFrame,
		const vector3d &shipPos,
		const vector3d &projectDir,
		const vector3d &radial,
		vector3d &groundPointOut,
		vector3d &planeNormalOut)
	{
		if (!ship->IsDocked())
			return false;

		const SpaceStation *station = ship->GetDockedWith();
		if (!station || !station->IsGroundStation())
			return false;

		const Body *stationParent = Frame::GetFrame(station->GetFrame())->GetBody();
		if (stationParent != static_cast<const Body *>(planet))
			return false;

		const int bay = ship->GetDockingPort();
		if (bay < 0 || bay >= int(station->GetDockingPortCount()))
			return false;

		const matrix4x4f stageLocal = station->GetStationType()->GetStageTransform(bay, DockStage::DOCKED);
		const matrix4x4d stationMatrix(station->GetInterpOrientRelTo(planetFrame), station->GetInterpPositionRelTo(planetFrame));
		const matrix4x4d bayTrans = stationMatrix * matrix4x4d(stageLocal);

		const vector3d padPoint = bayTrans.GetTranslate();
		vector3d padNormal = bayTrans.GetOrient().VectorY().NormalizedSafe();
		if (padNormal.LengthSqr() < 1e-12)
			return false;

		if (padNormal.Dot(radial) < 0.0)
			padNormal = -padNormal;

		const double padD = padNormal.Dot(padPoint);
		const double denom = projectDir.Dot(padNormal);
		if (std::abs(denom) < 1e-8)
			return false;

		const double t = (padD - padNormal.Dot(shipPos)) / denom;
		if (t <= 0.0)
			return false;

		groundPointOut = shipPos + projectDir * t;
		planeNormalOut = padNormal;
		return true;
	}

	static void RenderProjectedShipShadows(
		Graphics::Renderer *renderer,
		const Camera *camera,
		const FrameId camFrame,
		const std::vector<const Ship *> &ships,
		Material *material)
	{
		Renderer::MatrixTicket matrixTicket(renderer);
		camera->GetContext()->ApplyDrawTransforms(renderer);

		for (const Ship *ship : ships) {
			Frame *frame = Frame::GetFrame(ship->GetFrame());
			if (!frame)
				continue;
			Body *parentBody = frame->GetBody();
			if (!parentBody || !parentBody->IsType(ObjectType::PLANET))
				continue;

			auto *planet = static_cast<Planet *>(parentBody);
			if (!ShipGroundShadow::ShouldCastGroundShadow(ship, planet))
				continue;

			ShipGroundShadowPlane plane;
			if (!ShipGroundShadow::TryBuildShadowPlane(camera, ship, planet, plane))
				continue;

			matrix4x4d planetToView;
			Frame::GetFrameTransform(planet->GetFrame(), camFrame, planetToView);

			const vector3f planeNormalView = vector3f(planetToView.ApplyRotationOnly(plane.normal));
			const vector3f planePointView = vector3f(planetToView * plane.point);
			const vector3f projectDirView = vector3f(planetToView.ApplyRotationOnly(plane.projectDir));
			const float planeD = planeNormalView.Dot(planePointView);

			material->SetPushConstant(s_planeNormal, planeNormalView);
			material->SetPushConstant(s_planeD, planeD);
			material->SetPushConstant(s_projectDir, projectDirView);
			material->SetPushConstant(s_shadowStrength, plane.strength);

			Frame *shipFrame = Frame::GetFrame(ship->GetFrame());
			matrix4x4d viewTransform = shipFrame->GetInterpOrientRelTo(camFrame);
			viewTransform.SetTranslate(shipFrame->GetInterpPositionRelTo(camFrame));

			const auto trans = matrix4x4f(viewTransform * ship->GetInterpMatrix());
			const auto model = ship->GetModel();

			const double renderTime = Pi::game ? Pi::game->GetTime() + (Pi::GetGameTickAlpha() * Pi::game->GetTimeStep()) : Pi::GetApp()->GetTime();
			model->SetRenderTime(renderTime);

			SceneGraph::RenderData params;
			params.overrideMaterial = material;
			params.boundingRadius = model->GetDrawClipRadius();
			params.nodemask = SceneGraph::NODE_SOLID;
			model->GetRoot()->Render(trans, &params);
		}
	}

	static double LightAngleFromNormal(const vector3d &toLight, const vector3d &surfaceNormal)
	{
		return RAD2DEG(acos(Clamp(toLight.Dot(surfaceNormal), -1.0, 1.0)));
	}

	static float ShadowAngleFade(double angleDeg)
	{
		if (angleDeg >= SHADOW_MAX_ANGLE)
			return 0.f;
		if (angleDeg <= SHADOW_FADE_ANGLE)
			return 1.f;
		return float((SHADOW_MAX_ANGLE - angleDeg) / (SHADOW_MAX_ANGLE - SHADOW_FADE_ANGLE));
	}

	// Shadow strength from the angle between surface normal and light at surfacePoint.
	// 0 = no shadow (at or beyond SHADOW_MAX_ANGLE), 1 = full strength (at or below SHADOW_FADE_ANGLE).
	static float SurfaceNormalShadowStrength(const vector3d &surfaceNormal, const vector3d &lightPos, const vector3d &surfacePoint)
	{
		const vector3d toLight = (lightPos - surfacePoint).NormalizedSafe();
		return ShadowAngleFade(LightAngleFromNormal(toLight, surfaceNormal));
	}


	// Find a terrain shadow plane by iterative refinement of the terrain height
	//
	// Starting from the point under the ship, each iteration:
	//   1. Sample a terrain normal at the current centre
	//   2. Intersect the light ray (through the ship, along projectDir) with that tangent plane.
	//   3. Snap the intersection to the terrain surface and use that as the next centre.
	//
	// Footprint halves each iteration down to minFootprintMeters so early steps smooth over
	// ridges and valley walls before narrowing to the ship footprint.
	static void GetTerrainShadowPlane(
		const Planet *planet,
		const vector3d &shipPos,
		const vector3d &projectDir,
		const vector3d &startRadial,
		double minFootprintMeters,
		vector3d &shadowCentrePoint,
		vector3d &shadowPlaneNormal,
		double &shadowBestDistance)
	{
		const double minFootprint = std::max(minFootprintMeters, 1.0);
		const double startFootprint = std::max(SHADOW_TERRAIN_SAMPLE_WIDTH, minFootprint);

		vector3d centre = planet->GetTerrainSurfacePoint(startRadial);

		shadowCentrePoint = centre;
		shadowPlaneNormal = planet->GetTerrainSurfaceNormal(centre, startFootprint);
		shadowBestDistance = std::numeric_limits<double>::infinity();

		double hitDistance = 0.0;
		double footprint = startFootprint;

		while (true) {
			shadowPlaneNormal = planet->GetTerrainSurfaceNormal(centre, footprint);

			const double planeD = shadowPlaneNormal.Dot(centre);
			const double denom = projectDir.Dot(shadowPlaneNormal);
			// Light rays are parallel to the sampled plane — no meaningful intersection.
			if (std::abs(denom) < 1e-8)
				break;

			const double t = (planeD - shadowPlaneNormal.Dot(shipPos)) / denom;
			// Intersection is closer to the light than the ship is, which makes no sense for shadows
			if (t <= 0.0)
				break;

			centre = planet->GetTerrainSurfacePoint((shipPos + projectDir * t).NormalizedSafe());
			shadowCentrePoint = centre;
			shadowPlaneNormal = planet->GetTerrainSurfaceNormal(centre, footprint);
			hitDistance = t;

			if (footprint <= minFootprint)
				break;

			footprint *= 0.5;
			if (footprint < minFootprint)
				footprint = minFootprint;
		}

		if (hitDistance > 0.0)
			shadowBestDistance = hitDistance;
	}

	static bool SelectDominantLight(const Camera *camera, const Ship *ship, const Planet *planet, size_t &outIdx)
	{
		outIdx = 0;

		const std::vector<Camera::LightSource> &lights = camera->GetLightSources();
		if (lights.empty())
			return false;

		const FrameId planetFrame = planet->GetFrame();
		const vector3d shipPos = ship->GetInterpPositionRelTo(planetFrame);
		const vector3d radial = shipPos.NormalizedSafe();
		const double footprintDiameter = ship->GetRoughFootprintDiameter();
		const vector3d surfaceNormal = planet->GetTerrainSurfaceNormal(radial, footprintDiameter);

		size_t best = lights.size();
		float bestScore = -1.f;
		for (size_t i = 0; i < lights.size(); ++i) {
			const Camera::LightSource &source = lights[i];
			if (!source.GetBody())
				continue;

			const vector3d lightPos = source.GetBody()->GetInterpPositionRelTo(planetFrame);
			const vector3d toLight = (lightPos - shipPos).Normalized();
			const double angleDeg = LightAngleFromNormal(toLight, surfaceNormal);
			if (angleDeg > SHADOW_MAX_ANGLE)
				continue;

			const float angleFade = ShadowAngleFade(angleDeg);
			const float facing = std::max(0.0, toLight.Dot(surfaceNormal));
			const float score = camera->ShadowedIntensity(int(i), ship) * (source.GetLight().GetDiffuse().GetLuminance() / 255.f) * facing * angleFade;
			if (score > bestScore) {
				bestScore = score;
				best = i;
			}
		}

		if (best >= lights.size())
			return false;

		outIdx = best;
		return true;
	}

} // namespace

void ShipGroundShadow::Init(Graphics::Renderer *renderer)
{
	if (!s_shadowMat) {
		MaterialDescriptor shadowDesc;
		Graphics::RenderStateDesc shadowRsd;
		shadowRsd.blendMode = BLEND_SOLID;
		shadowRsd.depthTest = true;
		shadowRsd.depthWrite = true;
		shadowRsd.cullMode = CULL_NONE;

		const VertexFormatDesc shadowVfmt = VertexFormatDesc::FromAttribSet(ATTRIB_POSITION);
		s_shadowMat.reset(renderer->CreateMaterial("ground_shadow", shadowDesc, shadowRsd, shadowVfmt));
	}

	if (!s_compositeMat) {
		MaterialDescriptor compositeDesc;
		compositeDesc.textures = 1;
		Graphics::RenderStateDesc compositeRsd;
		compositeRsd.blendMode = BLEND_ALPHA;
		compositeRsd.depthTest = true;
		compositeRsd.depthWrite = false;
		compositeRsd.cullMode = CULL_NONE;
		compositeRsd.primitiveType = TRIANGLE_FAN;

		const VertexFormatDesc compositeVfmt = VertexFormatDesc::FromAttribSet(ATTRIB_POSITION | ATTRIB_UV0);
		s_compositeMat.reset(renderer->CreateMaterial("ground_shadow_composite", compositeDesc, compositeRsd, compositeVfmt));
		s_compositeMat->SetPushConstant(s_shadowAlpha, SHADOW_ALPHA);
		s_compositeMat->SetPushConstant(s_depthBias, SHADOW_DEPTH_BIAS);
	}

	if (!s_fullscreenVerts) {
		s_fullscreenVerts.reset(new VertexArray(ATTRIB_POSITION | ATTRIB_UV0));
		s_fullscreenVerts->Add(vector3f(-1.f, -1.f, 0.f), vector2f(0.f, 0.f));
		s_fullscreenVerts->Add(vector3f(1.f, -1.f, 0.f), vector2f(1.f, 0.f));
		s_fullscreenVerts->Add(vector3f(1.f, 1.f, 0.f), vector2f(1.f, 1.f));
		s_fullscreenVerts->Add(vector3f(-1.f, 1.f, 0.f), vector2f(0.f, 1.f));
	}
}

void ShipGroundShadow::Uninit()
{
	s_maskRT.reset();
	s_shadowMat.reset();
	s_compositeMat.reset();
	s_fullscreenVerts.reset();
	s_rtWidth = 0;
	s_rtHeight = 0;
}

void ShipGroundShadow::EnsureResources(Graphics::Renderer *renderer, int width, int height)
{
	Init(renderer);

	if (!s_maskRT || width != s_rtWidth || height != s_rtHeight) {
		s_maskRT.reset();
		s_shadowMat.reset();
		s_compositeMat.reset();
		RenderTargetDesc rtDesc(
			uint16_t(width), uint16_t(height),
			TEXTURE_RG32F,
			TEXTURE_DEPTH,
			false);
		s_maskRT.reset(renderer->CreateRenderTarget(rtDesc));

		s_rtWidth = width;
		s_rtHeight = height;
	}
}

bool ShipGroundShadow::ShouldCastGroundShadow(const Ship *ship, const Planet *planet)
{
	Frame *shipFrame = Frame::GetFrame(ship->GetFrame());
	if (!shipFrame)
		return false;

	Frame *planetFrame = Frame::GetFrame(planet->GetFrame());
	if (!planetFrame)
		return false;

	// Planet bodies live in the rotating frame; compare orbital (non-rot) frames.
	if (shipFrame->GetNonRotFrame() != planetFrame->GetNonRotFrame())
		return false;

	// Terrain height is defined in the planet's rotating frame, not the inertial orbital frame.
	const FrameId planetRotFrame = planetFrame->GetId();
	const vector3d shipPos = ship->GetInterpPositionRelTo(planetRotFrame);
	if (shipPos.Length() < 1.0)
		return false;

	const vector3d radial = shipPos.NormalizedSafe();
	const double footprintDiameter = ship->GetRoughFootprintDiameter();
	const vector3d surfacePoint = planet->GetTerrainSurfacePoint(radial);
	const vector3d surfaceNormal = planet->GetTerrainSurfaceNormal(radial, footprintDiameter);
	const double altitude = (shipPos - surfacePoint).Dot(surfaceNormal);
	return altitude < SHADOW_MAX_ALTITUDE;
}

bool ShipGroundShadow::TryBuildShadowPlane(const Camera *camera, const Ship *ship, const Planet *planet, ShipGroundShadowPlane &planeOut)
{
	if (!ShouldCastGroundShadow(ship, planet))
		return false;

	const FrameId planetFrame = planet->GetFrame();
	const vector3d shipPos = ship->GetInterpPositionRelTo(planetFrame);
	const vector3d radial = shipPos.NormalizedSafe();
	const double footprintDiameter = ship->GetRoughFootprintDiameter();

	size_t lightIdx = 0;
	if (!SelectDominantLight(camera, ship, planet, lightIdx))
		return false;

	const Camera::LightSource &light = camera->GetLightSources()[lightIdx];
	if (!light.GetBody())
		return false;

	vector3d lightPos = light.GetBody()->GetInterpPositionRelTo(planetFrame);

	// Send a ray from the light through the ship origin, then we're going to see what is the first thing it hits
	vector3d projectDir = shipPos - lightPos;
	if (projectDir.LengthSqr() < 1e-18)
		return false;
	projectDir = projectDir.Normalized();

	// Shadow plane candidate A: dock locator when docked at a ground station on this planet. The landing pad itself
	// can be at a slightly different height from the collision mesh, so we prefer to get the exact height when docked.
	vector3d dockPoint;
	vector3d dockNormal;
	if (TryBuildDockShadowPlane(ship, planet, planetFrame, shipPos, projectDir, radial, dockPoint, dockNormal)) {
		const float shadowStrength = SurfaceNormalShadowStrength(dockNormal, lightPos, dockPoint);
		if (shadowStrength <= 0.f)
			return false;
		planeOut.point = dockPoint;
		planeOut.normal = dockNormal;
		planeOut.projectDir = projectDir;
		planeOut.strength = shadowStrength;
		return true;	// If the ship is docked, then always use the landing pad as the shadow plane
	}

	// Shadow plane candidate B: multi-scale terrain plane refinement along the light ray
	vector3d shadowCentrePoint;
	vector3d shadowPlaneNormal;
	double shadowBestDistance = 0.0;
	GetTerrainShadowPlane(planet, shipPos, projectDir, radial, footprintDiameter, shadowCentrePoint, shadowPlaneNormal, shadowBestDistance);

	// Shadow plane candidate C: raycast against collision geometry, e.g. buildings and docking pads (when not docked)
	Frame *planetFrameObj = Frame::GetFrame(planetFrame);
	CollisionSpace *collisionSpace = planetFrameObj ? planetFrameObj->GetCollisionSpace() : nullptr;
	if (collisionSpace) {
		const Geom *ignoreGeom = static_cast<const ModelBody *>(ship)->GetGeom();
		CollisionContact hit = {};
		const double rayEpsilon = 0.5;
		const vector3d rayStart = shipPos + projectDir * rayEpsilon;
		const double rayLen = std::max<double>(SHADOW_MAX_ALTITUDE * 8.0, 20000.0);
		collisionSpace->TraceRay(rayStart, projectDir, rayLen, &hit, ignoreGeom);

		if (hit.distance < rayLen) {
			const double hitDist = hit.distance + rayEpsilon;
			if (hitDist > 0.0 && hitDist < shadowBestDistance) {
				vector3d hitNormal = hit.normal.NormalizedSafe();
				if (hitNormal.LengthSqr() > 0.0) {
					// Keep normals oriented outward
					if (hitNormal.Dot(radial) < 0.0)
						hitNormal = -hitNormal;
					if (SurfaceNormalShadowStrength(hitNormal, lightPos, hit.pos) > 0.f) {
						shadowCentrePoint = hit.pos;
						shadowPlaneNormal = hitNormal;
						shadowBestDistance = hitDist;
					}
				}
			}
		}
	}

	const float shadowStrength = SurfaceNormalShadowStrength(shadowPlaneNormal, lightPos, shadowCentrePoint);
	if (shadowStrength <= 0.f)
		return false;

	planeOut.point = shadowCentrePoint;
	planeOut.normal = shadowPlaneNormal;
	planeOut.projectDir = projectDir;
	planeOut.strength = shadowStrength;
	return true;
}

void ShipGroundShadow::RenderPass(
	Graphics::Renderer *renderer,
	const Camera *camera,
	Graphics::RenderTarget *mainRenderTarget,
	const std::vector<const Ship *> &ships)
{
	if (ships.empty() || !mainRenderTarget)
		return;

	PROFILE_SCOPED()

	const int width = renderer->GetWindowWidth();
	const int height = renderer->GetWindowHeight();
	EnsureResources(renderer, width, height);
	if (!s_maskRT || !s_shadowMat || !s_compositeMat || !s_fullscreenVerts)
		return;

	const FrameId camFrame = camera->GetContext()->GetTempFrame();

	// We do two passes, the first calculates the shadows, the second adds them to the scene.
	// This is so that overlapping shadows (e.g. shadow cast by the hull and shadow cast by
	// the landing gear) merge into a single shadow in the output.

	Renderer::StateTicket ticket1(renderer);
	renderer->SetRenderTarget(s_maskRT.get());
	renderer->SetViewport({ 0, 0, width, height });
	renderer->ClearScreen(Color(0, 0, 0, 0), true);

	RenderProjectedShipShadows(renderer, camera, camFrame, ships, s_shadowMat.get());

	Renderer::StateTicket ticket2(renderer);
	renderer->SetRenderTarget(mainRenderTarget);
	renderer->SetViewport({ 0, 0, width, height });

	s_compositeMat->SetTexture(s_shadowMask, s_maskRT->GetColorTexture());

	{
		Renderer::MatrixTicket mt(renderer);
		renderer->SetProjection(matrix4x4f::Identity);
		renderer->SetTransform(matrix4x4f::Identity);
		renderer->DrawBuffer(s_fullscreenVerts.get(), s_compositeMat.get());
	}
}
