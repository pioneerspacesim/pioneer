// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Camera.h"
#include "Frame.h"
#include "galaxy/StarSystem.h"
#include "Space.h"
#include "Player.h"
#include "Pi.h"
#include "Sfx.h"
#include "Game.h"
#include "Planet.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"

#include <SDL_stdinc.h>

using namespace Graphics;

// if a body would render smaller than this many pixels, just ignore it
static const float OBJECT_HIDDEN_PIXEL_THRESHOLD = 2.0f;

// if a terrain object would render smaller than this many pixels, draw a billboard instead
static const float BILLBOARD_PIXEL_THRESHOLD = 15.0f;

CameraContext::CameraContext(float width, float height, float fovAng, float zNear, float zFar) :
	m_width(width),
	m_height(height),
	m_fovAng(fovAng),
	m_zNear(zNear),
	m_zFar(zFar),
	m_frustum(m_width, m_height, m_fovAng, m_zNear, m_zFar),
	m_frame(nullptr),
	m_pos(0.0),
	m_orient(matrix3x3d::Identity()),
	m_camFrame(nullptr)
{
}

CameraContext::~CameraContext()
{
	if (m_camFrame)
		EndFrame();
}

void CameraContext::BeginFrame()
{
	assert(m_frame);
	assert(!m_camFrame);

	// make temporary camera frame
	m_camFrame = new Frame(m_frame, "camera", Frame::FLAG_ROTATING);

	// move and orient it to the camera position
	m_camFrame->SetOrient(m_orient, Pi::game ? Pi::game->GetTime() : 0.0);
	m_camFrame->SetPosition(m_pos);

	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	m_camFrame->ClearMovement();
	m_camFrame->UpdateInterpTransform(1.0);			// update root-relative pos/orient
}

void CameraContext::EndFrame()
{
	assert(m_frame);
	assert(m_camFrame);

	m_frame->RemoveChild(m_camFrame);
	delete m_camFrame;
	m_camFrame = nullptr;
}

void CameraContext::ApplyDrawTransforms(Graphics::Renderer *r)
{
	r->SetPerspectiveProjection(m_fovAng, m_width/m_height, m_zNear, m_zFar);
	r->SetTransform(matrix4x4f::Identity());
}


Camera::Camera(RefCountedPtr<CameraContext> context, Graphics::Renderer *renderer) :
	m_context(context),
	m_renderer(renderer)
{
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;

	m_billboardMaterial.reset(m_renderer->CreateMaterial(desc));
	m_billboardMaterial->texture0 = Graphics::TextureBuilder::Billboard("textures/planet_billboard.png").GetOrCreateTexture(m_renderer, "billboard");
}

static void position_system_lights(Frame *camFrame, Frame *frame, std::vector<Camera::LightSource> &lights)
{
	PROFILE_SCOPED()
	if (lights.size() > 3) return;

	SystemBody *body = frame->GetSystemBody();
	// IsRotFrame check prevents double counting
	if (body && !frame->IsRotFrame() && (body->GetSuperType() == SystemBody::SUPERTYPE_STAR)) {
		vector3d lpos = frame->GetPositionRelTo(camFrame);
		const double dist = lpos.Length() / AU;
		lpos *= 1.0/dist; // normalize

		const Uint8 *col = StarSystem::starRealColors[body->GetType()];

		const Color lightCol(col[0], col[1], col[2], 0);
		vector3f lightpos(lpos.x, lpos.y, lpos.z);
		lights.push_back(Camera::LightSource(frame->GetBody(), Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, lightpos, lightCol, lightCol)));
	}

	for (Frame* kid : frame->GetChildren()) {
		position_system_lights(camFrame, kid, lights);
	}
}

void Camera::Update()
{
	Frame *camFrame = m_context->GetCamFrame();

	// evaluate each body and determine if/where/how to draw it
	m_sortedBodies.clear();
	for (Body* b : Pi::game->GetSpace()->GetBodies()) {
		BodyAttrs attrs;
		attrs.body = b;

		// determine position and transform for draw
		Frame::GetFrameTransform(b->GetFrame(), camFrame, attrs.viewTransform);
		attrs.viewCoords = attrs.viewTransform * b->GetInterpPosition();

		// cull off-screen objects
		double rad = b->GetClipRadius();
		if (!m_context->GetFrustum().TestPointInfinite(attrs.viewCoords, rad))
			continue;

		attrs.camDist = attrs.viewCoords.Length();
		attrs.bodyFlags = b->GetFlags();

		// approximate pixel width (disc diameter) of body on screen
		float pixSize = (Graphics::GetScreenWidth() * rad / attrs.camDist);
		if (pixSize < OBJECT_HIDDEN_PIXEL_THRESHOLD)
			continue;

		// terrain objects are visible from distance but might not have any discernable features
		attrs.billboard = false;
		if (b->IsType(Object::TERRAINBODY)) {
			if (pixSize < BILLBOARD_PIXEL_THRESHOLD) {
				attrs.billboard = true;
				vector3d pos;
				double size = rad * 2.0 * m_context->GetFrustum().TranslatePoint(attrs.viewCoords, pos);
				attrs.billboardPos = vector3f(&pos.x);
				attrs.billboardSize = float(size);
				if (b->IsType(Object::STAR)) {
					const Uint8 *col = StarSystem::starRealColors[b->GetSystemBody()->GetType()];
					attrs.billboardColor = Color(col[0], col[1], col[2], 255);
				}
				else if (b->IsType(Object::PLANET)) {
					double surfaceDensity; // unused
					// XXX this is pretty crap because its not always right
					// (gas giants are always white) and because it should have
					// some star colour mixed in to simulate lighting
					b->GetSystemBody()->GetAtmosphereFlavor(&attrs.billboardColor, &surfaceDensity);
					attrs.billboardColor.a = 255; // no alpha, these things are hard enough to see as it is
				}
				else
					attrs.billboardColor = Color::WHITE;
			}
		}

		m_sortedBodies.push_back(attrs);
	}

	// depth sort
	m_sortedBodies.sort();
}

void Camera::Draw(const Body *excludeBody, ShipCockpit* cockpit)
{
	PROFILE_SCOPED()

	Frame *camFrame = m_context->GetCamFrame();

	m_renderer->ClearScreen();

	matrix4x4d trans2bg;
	Frame::GetFrameTransform(Pi::game->GetSpace()->GetRootFrame(), camFrame, trans2bg);
	trans2bg.ClearToRotOnly();

	// Pick up to four suitable system light sources (stars)
	m_lightSources.clear();
	m_lightSources.reserve(4);
	position_system_lights(camFrame, Pi::game->GetSpace()->GetRootFrame(), m_lightSources);

	if (m_lightSources.empty()) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		const Color col(255);
		m_lightSources.push_back(LightSource(0, Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f), col, col)));
	}

	//fade space background based on atmosphere thickness and light angle
	float bgIntensity = 1.f;
	if (camFrame->GetParent() && camFrame->GetParent()->IsRotFrame()) {
		//check if camera is near a planet
		Body *camParentBody = camFrame->GetParent()->GetBody();
		if (camParentBody && camParentBody->IsType(Object::PLANET)) {
			Planet *planet = static_cast<Planet*>(camParentBody);
			const vector3f relpos(planet->GetInterpPositionRelTo(camFrame));
			double altitude(relpos.Length());
			double pressure, density;
			planet->GetAtmosphericState(altitude, &pressure, &density);
			if (pressure >= 0.001)
			{
				//go through all lights to calculate something resembling light intensity
				float angle = 0.f;
				for(std::vector<LightSource>::const_iterator it = m_lightSources.begin();
					it != m_lightSources.end(); ++it) {
					const vector3f lightDir(it->GetLight().GetPosition().Normalized());
					angle += std::max(0.f, lightDir.Dot(-relpos.Normalized())) * (it->GetLight().GetDiffuse().GetLuminance() / 255.0f);
				}
				//calculate background intensity with some hand-tweaked fuzz applied
				bgIntensity = Clamp(1.f - std::min(1.f, powf(density, 0.25f)) * (0.3f + powf(angle, 0.25f)), 0.f, 1.f);
			}
		}
	}

	Pi::game->GetSpace()->GetBackground()->SetIntensity(bgIntensity);
	Pi::game->GetSpace()->GetBackground()->Draw(trans2bg);

	{
		std::vector<Graphics::Light> rendererLights;
		rendererLights.reserve(m_lightSources.size());
		for (size_t i = 0; i < m_lightSources.size(); i++)
			rendererLights.push_back(m_lightSources[i].GetLight());
		m_renderer->SetLights(rendererLights.size(), &rendererLights[0]);
	}

	for (std::list<BodyAttrs>::iterator i = m_sortedBodies.begin(); i != m_sortedBodies.end(); ++i) {
		BodyAttrs *attrs = &(*i);

		// explicitly exclude a single body if specified (eg player)
		if (attrs->body == excludeBody)
			continue;

		// draw something!
		if (attrs->billboard) {
			Graphics::Renderer::MatrixTicket mt(m_renderer, Graphics::MatrixMode::MODELVIEW);
			m_renderer->SetTransform(matrix4x4d::Identity());
			m_billboardMaterial->diffuse = attrs->billboardColor;
			m_renderer->DrawPointSprites(1, &attrs->billboardPos, Sfx::additiveAlphaState, m_billboardMaterial.get(), attrs->billboardSize);
		}
		else
			attrs->body->Render(m_renderer, this, attrs->viewCoords, attrs->viewTransform);
	}

	Sfx::RenderAll(m_renderer, Pi::game->GetSpace()->GetRootFrame(), camFrame);

	// NB: Do any screen space rendering after here:
	// Things like the cockpit and AR features like hudtrails, space dust etc.

	// Render cockpit
	// XXX only here because it needs a frame for lighting calc
	// should really be in WorldView, immediately after camera draw
	if(cockpit)
		cockpit->RenderCockpit(m_renderer, this, camFrame);
}

void Camera::CalcShadows(const int lightNum, const Body *b, std::vector<Shadow> &shadowsOut) const {
	// Set up data for eclipses. All bodies are assumed to be spheres.
	const Body *lightBody = m_lightSources[lightNum].GetBody();
	if (!lightBody)
		return;

	const double lightRadius = lightBody->GetPhysRadius();
	const vector3d bLightPos = lightBody->GetPositionRelTo(b);
	const vector3d lightDir = bLightPos.Normalized();

	double bRadius;
	if (b->IsType(Object::TERRAINBODY)) bRadius = b->GetSystemBody()->GetRadius();
	else bRadius = b->GetPhysRadius();

	// Look for eclipsing third bodies:
	for (const Body *b2 : Pi::game->GetSpace()->GetBodies()) {
		if ( b2 == b || b2 == lightBody || !(b2->IsType(Object::PLANET) || b2->IsType(Object::STAR)))
			continue;

		double b2Radius = b2->GetSystemBody()->GetRadius();
		vector3d b2pos = b2->GetPositionRelTo(b);
		const double perpDist = lightDir.Dot(b2pos);

		if ( perpDist <= 0 || perpDist > bLightPos.Length())
			// b2 isn't between b and lightBody; no eclipse
			continue;

		// Project to the plane perpendicular to lightDir, taking the line between the shadowed sphere
		// (b) and the light source as zero. Our calculations assume that the light source is at
		// infinity. All lengths are normalised such that b has radius 1. srad is then the radius of the
		// occulting sphere (b2), and lrad is the apparent radius of the light disc when considered to
		// be at the distance of b2, and projectedCentre is the normalised projected position of the
		// centre of b2 relative to the centre of b. The upshot is that from a point on b, with
		// normalised projected position p, the picture is of a disc of radius lrad being occulted by a
		// disc of radius srad centred at projectedCentre-p. To determine the light intensity at p, we
		// then just need to estimate the proportion of the light disc being occulted.
		const double srad = b2Radius / bRadius;
		const double lrad = (lightRadius/bLightPos.Length())*perpDist / bRadius;
		if (srad / lrad < 0.01) {
			// any eclipse would have negligible effect - ignore
			continue;
		}
		const vector3d projectedCentre = ( b2pos - perpDist*lightDir ) / bRadius;
		if (projectedCentre.Length() < 1 + srad + lrad) {
			// some part of b is (partially) eclipsed
			Camera::Shadow shadow = { lightNum, projectedCentre, static_cast<float>(srad), static_cast<float>(lrad) };
			shadowsOut.push_back(shadow);
		}
	}
}

float discCovered(const float dist, const float rad) {
	// proportion of unit disc covered by a second disc of radius rad placed
	// dist from centre of first disc.
	//
	// WLOG, the second disc is displaced horizontally to the right.
	// xl = rightwards distance to intersection of the two circles.
	// xs = normalised leftwards distance from centre of second disc to intersection.
	// d = vertical distance to an intersection point
	// The clampings handle the cases where one disc contains the other.
	const float radsq = rad*rad;
	const float xl = Clamp((dist*dist + 1.f - radsq) / (2.f*std::max(0.001f,dist)), -1.f, 1.f);
	const float xs = Clamp((dist - xl)/std::max(0.001f,rad), -1.f, 1.f);
	const float d = sqrt(std::max(0.f, 1.f - xl*xl));

	const float th = Clamp(acosf(xl), 0.f, float(M_PI));
	const float th2 = Clamp(acosf(xs), 0.f, float(M_PI));

	assert(!is_nan(d) && !is_nan(th) && !is_nan(th2));

	// covered area can be calculated as the sum of segments from the two
	// discs plus/minus some triangles, and it works out as follows:
	return Clamp((th + radsq*th2 - dist*d)/float(M_PI), 0.f, 1.f);
}

static std::vector<Camera::Shadow> shadows;

float Camera::ShadowedIntensity(const int lightNum, const Body *b) const {
	shadows.clear();
	shadows.reserve(16);
	CalcShadows(lightNum, b, shadows);
	float product = 1.0;
	for (std::vector<Camera::Shadow>::const_iterator it = shadows.begin(), itEnd = shadows.end(); it!=itEnd; it++)
		product *= 1.0 - discCovered(it->centre.Length() / it->lrad, it->srad / it->lrad);
	return product;
}

// PrincipalShadows(b,n): returns the n biggest shadows on b in order of size
void Camera::PrincipalShadows(const Body *b, const int n, std::vector<Shadow> &shadowsOut) const {
	shadows.clear();
	shadows.reserve(16);
	for (size_t i = 0; i < 4 && i < m_lightSources.size(); i++) {
		CalcShadows(i, b, shadows);
	}
	shadowsOut.reserve(shadows.size());
	std::sort(shadows.begin(), shadows.end());
	std::vector<Shadow>::reverse_iterator it = shadows.rbegin(), itREnd = shadows.rend();
	for (int i = 0; i < n; i++) {
		if (it == itREnd) break;
		shadowsOut.push_back(*(it++));
	}
}
