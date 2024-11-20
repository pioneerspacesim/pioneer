// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SceneRenderer.h"

#include "Body.h"
#include "Camera.h"
#include "Frame.h"
#include "Planet.h"
#include "Sfx.h"
#include "Space.h"
#include "SpaceStation.h"

#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "render/RenderSetup.h"
#include "render/ResourceManager.h"

#include "render/generators/BackgroundGenerator.h"
#include "render/generators/BillboardGenerator.h"
#include "render/generators/LightingGenerator.h"
#include "render/generators/FullscreenGenerator.h"

#include "profiler/Profiler.h"

using namespace Render;

// if a body would render smaller than this many pixels, just ignore it
static const float OBJECT_HIDDEN_PIXEL_THRESHOLD = 2.0f;

// if a terrain object would render smaller than this many pixels, draw a billboard instead
static const float BILLBOARD_PIXEL_THRESHOLD = 8.0f;

bool compare_body_attrs(const SceneRenderer::BodyAttrs &a, const SceneRenderer::BodyAttrs &b)
{
	// both drawing last; distance order
	if (a.bodyFlags & Body::FLAG_DRAW_LAST && b.bodyFlags & Body::FLAG_DRAW_LAST)
		return a.camDist > b.camDist;

	// a drawing last; draw b first
	if (a.bodyFlags & Body::FLAG_DRAW_LAST)
		return false;

	// b drawing last; draw a first
	if (b.bodyFlags & Body::FLAG_DRAW_LAST)
		return true;

	// both in normal draw; distance order
	return a.camDist > b.camDist;
}

void RenderGenerator::CacheShadersForPass(RenderPass *pass)
{
	if (!pass->shader.empty()) {
		// TODO: this is more viable when the RenderStateDesc parameters can be specified in a shaderdef file
		Graphics::RenderStateDesc rsd = {};
		rsd.depthTest = false;
		rsd.depthWrite = false;

		Graphics::Material *mat = m_sceneRenderer->GetRenderer()->CreateMaterial(pass->shader, {}, rsd);
		m_sceneRenderer->CacheShaderForPass(pass, pass->shader, mat);
	}
}

SceneRenderer::SceneRenderer(Graphics::Renderer *r, ResourceManager *rm) :
	m_renderer(r),
	m_resManager(rm)
{
	m_generators.try_emplace("background", new BackgroundGenerator(this));
	m_generators.try_emplace("billboards", new BillboardGenerator(this));
	m_generators.try_emplace("lighting", new LightingGenerator(this));
	m_generators.try_emplace("fullscreen", new FullscreenGenerator(this));
	m_generators.try_emplace("fullscreen_resolve", new FullscreenResolveGenerator(this));
	m_generators.try_emplace("fullscreen_downsample", new FullscreenDownsampleGenerator(this));
}

SceneRenderer::~SceneRenderer()
{
}

void SceneRenderer::LoadRenderSetup(RenderSetup *renderSetup)
{
	PROFILE_SCOPED()

	m_renderSetup.reset(renderSetup);
	m_resManager->LoadRenderSetup(renderSetup);
	// TODO: validate all layer configurations to ensure they reference valid
	// resource objects and can be created
}

void SceneRenderer::BeforeRendering(Space *space, Camera *camera)
{
	PROFILE_SCOPED()

	// Build a list of sorted bodies, presuming that one of the layers we're going to draw will use them
	// NOTE: this is comparatively quite inefficient - bodies should be sorted into buckets for each render pass
	// based on what the render pass wants to consume (i.e. billboards, opaque, transparent)
	// This would work better with an abstraction based around submitting RenderObjects to a RenderScene rather
	// than traversing the list of bodies to call the Render() virtual method
	BuildSortedBodies(space, camera->GetContext());
}

void SceneRenderer::RenderScene(Space *space, Camera *camera, std::string_view layerName)
{
	PROFILE_SCOPED()

	const CameraContext *context = camera->GetContext();

	// Get the list of layers enabled for this draw
	auto layerConfig = m_renderSetup->GetRenderLayerConfig(layerName);

	// Execute each render pass in this layer
	for (auto &pass : layerConfig) {
		PROFILE_SCOPED_RAW(pass->name.c_str());

		SetupRenderPass(pass);

		if (!pass->generator.empty() && m_generators.count(pass->generator)) {
			m_generators.at(pass->generator)->Run(pass, space, context);
		} else if (pass->generator.empty()) {
			// implicit "main objects" render pass
			RenderLitObjects(pass, space, camera);
		}
	}

	// HACK: SfxManager should be moved to a particle system render generator
	SfxManager::RenderAll(m_renderer, space->GetRootFrame(), context->GetTempFrame());
}

void SceneRenderer::SetupRenderPass(RenderPass *pass)
{
	PROFILE_SCOPED()

	if (!pass->render_target.empty()) {
		Graphics::RenderTarget *rt = m_resManager->GetRenderTarget(pass->render_target);
		assert(rt);

		if (m_renderer->GetRenderTarget() != rt)
			m_renderer->SetRenderTarget(rt);
	}

	// TODO: handle multiple render target textures / depth texture

	// Cache materials for the render pass
	// We assume a RenderPass pointer will be the same for subsequent
	// invocations of the same RenderLayer
	if (!pass->generator.empty()) {
		auto iter = m_generators.find(pass->generator);

		if (iter != m_generators.end() && m_passMaterials[pass].empty()) {

			// TODO: determine when this needs to be called to regenerate materials
			// When the backbuffer changes? When shaders get hot-reloaded? When it's explicitly requested?
			iter->second->CacheShadersForPass(pass);

		}
	}
}

void SceneRenderer::CacheShaderForPass(RenderPass *pass, std::string_view name, Graphics::Material *mat)
{
	m_passMaterials[pass].emplace_back(name, mat);
}

Graphics::Material *SceneRenderer::GetShaderForPass(RenderPass *pass, std::string_view name)
{
	auto iter = m_passMaterials.find(pass);
	if (iter != m_passMaterials.end()) {

		for (auto &cached : iter->second) {
			if (cached.name == name) {
				return cached.mat.get();
			}
		}

	}

	return nullptr;
}

void SceneRenderer::BuildSortedBodies(Space *space, const CameraContext *camera)
{
	PROFILE_SCOPED()

	FrameId camFrame = camera->GetTempFrame();

	// evaluate each body and determine if/where/how to draw it
	m_sortedBodies.clear();
	m_spaceStations.clear();

	for (Body *b : space->GetBodies()) {
		BodyAttrs attrs = {};
		attrs.body = b;
		attrs.exclude = false; // TODO unused
		attrs.billboard = false; // false by default
		attrs.calcAtmosphericLighting = false; // false by default
		attrs.calcInteriorLighting = false;

		// If the body wishes to be excluded from the draw, skip it.
		if (b->GetFlags() & Body::FLAG_DRAW_EXCLUDE)
			continue;

		// determine position and transform for draw
		//		Frame::GetFrameTransform(b->GetFrame(), camFrame, attrs.viewTransform);		// doesn't use interp coords, so breaks in some cases
		Frame *f = Frame::GetFrame(b->GetFrame());
		attrs.viewTransform = f->GetInterpOrientRelTo(camFrame);
		attrs.viewTransform.SetTranslate(f->GetInterpPositionRelTo(camFrame));
		attrs.viewCoords = attrs.viewTransform * b->GetInterpPosition();

		// cull off-screen objects
		double rad = b->GetClipRadius();
		if (!camera->GetFrustum().TestPointInfinite(attrs.viewCoords, rad))
			continue;

		attrs.camDist = attrs.viewCoords.Length();
		attrs.bodyFlags = b->GetFlags();

		// approximate pixel width (disc diameter) of body on screen
		// FIXME: this should reference a property set on the camera instead of querying the window size
		const float pixSize = m_renderer->GetWindowHeight() * 2.0 * rad / (attrs.camDist * Graphics::GetFovFactor());

		// terrain objects are visible from distance but might not have any discernable features
		if (b->IsType(ObjectType::TERRAINBODY)) {
			if (pixSize < BILLBOARD_PIXEL_THRESHOLD) {
				attrs.billboard = true;

				// scale the position of the terrain body until it fits within the far plane for its billboard to be rendered
				// Note that with an infinite projection matrix there is no far plane and this is somewhat unnecessary
				double zFar = camera->GetZFar();
				double scaleFactor = zFar / attrs.viewCoords.Length() - 0.000001; // tiny nudge closer from the far plane

				// Set viewCoords to billboard position
				attrs.viewCoords *= std::min(scaleFactor, 1.0);

				// limit the minimum billboard size for planets so they're always a little visible
				attrs.billboardSize = std::max(1.0f, pixSize);
				if (b->IsType(ObjectType::STAR)) {
					attrs.billboardColor = StarSystem::starRealColors[b->GetSystemBody()->GetType()];
				} else if (b->IsType(ObjectType::PLANET)) {
					// XXX this should incorporate some lighting effect
					// (ie, colour of the illuminating star(s))
					attrs.billboardColor = b->GetSystemBody()->GetAlbedo();
				} else {
					attrs.billboardColor = Color::WHITE;
				}

				// this should always be the main star in the system - except for the star itself!
				if (!m_lightSources.empty() && !b->IsType(ObjectType::STAR)) {
					attrs.billboardColor *= m_lightSources[0].light.GetDiffuse(); // colour the billboard a little with the Starlight
				}

				attrs.billboardColor.a = 255; // no alpha, these things are hard enough to see as it is
			}
		} else if (pixSize < OBJECT_HIDDEN_PIXEL_THRESHOLD) {
			continue;
		}

		Body *parentBody = f->GetBody();
		if (parentBody && parentBody->GetType() == ObjectType::PLANET) {
			auto *planet = static_cast<Planet *>(parentBody);

			double atmo_rad_sqr = planet->GetAtmosphereRadius() * planet->GetAtmosphereRadius();
			if (b->IsType(ObjectType::MODELBODY) && b->GetPosition().LengthSqr() <= atmo_rad_sqr)
				attrs.calcAtmosphericLighting = true;
		}

		if(b->IsType(ObjectType::SHIP)) {
			attrs.calcInteriorLighting = true;
		}

		m_sortedBodies.push_back(attrs);

		if(b->IsType(ObjectType::SPACESTATION)) {
			m_spaceStations.push_back(static_cast<SpaceStation *>(b));
		}
	}

	// depth sort
	std::sort(m_sortedBodies.begin(), m_sortedBodies.end(), compare_body_attrs);
}

void SceneRenderer::RenderLitObjects(RenderPass *pass, Space *space, Camera *camera)
{
	PROFILE_SCOPED()

	// Save lights for later restoring
	std::vector<float> oldLightIntensities = {};
	std::vector<float> lightIntensities;

	m_camLights.reserve(m_lightSources.size());

	for (size_t i = 0; i < m_lightSources.size(); i++) {
		oldLightIntensities.push_back(m_renderer->GetLight(i).GetIntensity());
		lightIntensities.push_back(1.0f);

		m_camLights.emplace_back(m_lightSources[i].body, m_lightSources[i].light);
	}

	for (auto &attrs : GetSortedBodies()) {
		if (attrs.exclude || attrs.billboard)
			continue;

		PrepareLighting(&attrs);
		attrs.body->Render(m_renderer, camera, attrs.viewCoords, attrs.viewTransform);
	}

	// Reset lighting back to "normal" at the end of the draw
	// TODO: state must die
	m_renderer->SetAmbientColor(Color::WHITE);
	m_renderer->SetLightIntensity(m_lightSources.size(), oldLightIntensities.data());
}

void SceneRenderer::PrepareLighting(BodyAttrs *attrs)
{
	std::vector<float> lightIntensities;

	double ambient = 0.05, direct = 1.0;
	if (attrs->calcAtmosphericLighting)
		Camera::CalcLighting(attrs->body, m_camLights, ambient, direct);

	Color4ub ambientLightColor = Color::WHITE;
	Color4ub stationLightColor = Color::WHITE;
	double stationFactor = 0.0;

	if (attrs->calcInteriorLighting)
		Camera::CalcInteriorLighting(attrs->body, m_spaceStations, stationLightColor, stationFactor);

	direct = direct * (1.0 - stationFactor);
	ambient = ambient * (1.0 - stationFactor) + stationFactor;

	for (size_t i = 0; i < m_lightSources.size(); i++)
		lightIntensities.push_back(direct * Camera::ShadowedIntensity(m_lightSources[i].body, attrs->body));

	// Setup dynamic lighting parameters
	Color4ub ambientMix = (ambientLightColor.Shade((float)stationFactor) + stationLightColor.Shade(1.0f - stationFactor)).Shade(1.0 - ambient);
	ambientMix.a = 255;

	// Set lighting data for this draw
	// TODO: state must die
	m_renderer->SetAmbientColor(ambientMix);
	m_renderer->SetLightIntensity(m_lightSources.size(), lightIntensities.data());
}
