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

#include <SDL_stdinc.h>

using namespace Graphics;

Camera::Camera(float width, float height, float fovY, float znear, float zfar) :
	m_width(width),
	m_height(height),
	m_fovAng(fovY),
	m_zNear(znear),
	m_zFar(zfar),
	m_frustum(m_width, m_height, m_fovAng, znear, zfar),
	m_pos(0.0),
	m_orient(matrix3x3d::Identity()),
	m_frame(0),
	m_camFrame(0),
	m_renderer(0)
{
}

Camera::~Camera()
{
	if (m_camFrame) {
		m_frame->RemoveChild(m_camFrame);
		delete m_camFrame;
	}
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

		const Uint8 *col = StarSystem::starRealColors[body->type];

		const Color lightCol(col[0], col[1], col[2], 0);
		vector3f lightpos(lpos.x, lpos.y, lpos.z);
		lights.push_back(Camera::LightSource(frame->GetBody(), Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, lightpos, lightCol, lightCol)));
	}

	for (Frame::ChildIterator it = frame->BeginChildren(); it != frame->EndChildren(); ++it) {
		position_system_lights(camFrame, *it, lights);
	}
}

void Camera::Update()
{
	if (!m_frame) return;

	// make temporary camera frame
	m_camFrame = new Frame(m_frame, "camera", Frame::FLAG_ROTATING);

	// move and orient it to the camera position
	m_camFrame->SetOrient(m_orient, Pi::game ? Pi::game->GetTime() : 0.0);
	m_camFrame->SetPosition(m_pos);

	// make sure old orient and interpolated orient (rendering orient) are not rubbish
	m_camFrame->ClearMovement();
	m_camFrame->UpdateInterpTransform(1.0);			// update root-relative pos/orient

	// evaluate each body and determine if/where/how to draw it
	m_sortedBodies.clear();
	for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i) {
		Body *b = *i;

		// prepare attrs for sorting and drawing
		BodyAttrs attrs;
		attrs.body = b;
		Frame::GetFrameRenderTransform(b->GetFrame(), m_camFrame, attrs.viewTransform);
		attrs.viewCoords = attrs.viewTransform * b->GetInterpPosition();
		attrs.camDist = attrs.viewCoords.Length();
		attrs.bodyFlags = b->GetFlags();
		m_sortedBodies.push_back(attrs);
	}

	// depth sort
	m_sortedBodies.sort();
}

void Camera::Draw(Graphics::Renderer *renderer, const Body *excludeBody, ShipCockpit* cockpit)
{
	PROFILE_SCOPED()
	if (!m_camFrame) return;
	if (!renderer) return;

	m_renderer = renderer;

	m_renderer->SetDepthWrite(true);
	m_renderer->SetDepthTest(true);

	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));

	m_renderer->SetPerspectiveProjection(m_fovAng, m_width/m_height, m_zNear, m_zFar);
	m_renderer->SetTransform(matrix4x4f::Identity());
	m_renderer->ClearScreen();

	matrix4x4d trans2bg;
	Frame::GetFrameRenderTransform(Pi::game->GetSpace()->GetRootFrame(), m_camFrame, trans2bg);
	trans2bg.ClearToRotOnly();

	// Pick up to four suitable system light sources (stars)
	m_lightSources.clear();
	m_lightSources.reserve(4);
	position_system_lights(m_camFrame, Pi::game->GetSpace()->GetRootFrame(), m_lightSources);

	if (m_lightSources.empty()) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		const Color col(255);
		m_lightSources.push_back(LightSource(0, Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f), col, col)));
	}

	//fade space background based on atmosphere thickness and light angle
	float bgIntensity = 1.f;
	if (m_camFrame->GetParent() && m_camFrame->GetParent()->IsRotFrame()) {
		//check if camera is near a planet
		Body *camParentBody = m_camFrame->GetParent()->GetBody();
		if (camParentBody && camParentBody->IsType(Object::PLANET)) {
			Planet *planet = static_cast<Planet*>(camParentBody);
			const vector3f relpos(planet->GetInterpPositionRelTo(m_camFrame));
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
		renderer->SetLights(rendererLights.size(), &rendererLights[0]);
	}

	for (std::list<BodyAttrs>::iterator i = m_sortedBodies.begin(); i != m_sortedBodies.end(); ++i) {
		BodyAttrs *attrs = &(*i);

		// explicitly exclude a single body if specified (eg player)
		if (attrs->body == excludeBody)
			continue;

		double rad = attrs->body->GetClipRadius();
		if (!m_frustum.TestPointInfinite((*i).viewCoords, rad))
			continue;

		// draw spikes for far objects
		double screenrad = 500 * rad / attrs->camDist;      // approximate pixel size
		if (attrs->body->IsType(Object::PLANET) && screenrad < 2) {
			// absolute bullshit
			double spikerad = (7 + 1.5*log10(screenrad)) * rad / screenrad;
			DrawSpike(spikerad, attrs->viewCoords, attrs->viewTransform);
		}
		else if (screenrad >= 2 || attrs->body->IsType(Object::STAR) ||
					(attrs->body->IsType(Object::PROJECTILE) && screenrad > 0.25))
			attrs->body->Render(renderer, this, attrs->viewCoords, attrs->viewTransform);
	}

	Sfx::RenderAll(renderer, Pi::game->GetSpace()->GetRootFrame(), m_camFrame);

	// NB: Do any screen space rendering after here:
	// Things like the cockpit and AR features like hudtrails, space dust etc.

	// Render cockpit
	// XXX only here because it needs a frame for lighting calc
	// should really be in WorldView, immediately after camera draw
	if(cockpit)
		cockpit->RenderCockpit(renderer, this, m_camFrame);


	m_frame->RemoveChild(m_camFrame);
	delete m_camFrame;
	m_camFrame = 0;

	glPopAttrib();
}

void Camera::DrawSpike(double rad, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	PROFILE_SCOPED()
	// draw twinkly star-thing on faraway objects
	// XXX this seems like a good case for drawing in 2D - use projected position, then the
	// "face the camera dammit" bits can be skipped
	if (!m_renderer) return;

	const double newdist = m_zNear + 0.5f * (m_zFar - m_zNear);
	const double scale = newdist / viewCoords.Length();

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(scale*viewCoords.x, scale*viewCoords.y, scale*viewCoords.z);

	// face the camera dammit
	vector3d zaxis = viewCoords.Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	trans = trans * rot;

	m_renderer->SetDepthTest(false);
	m_renderer->SetBlendMode(BLEND_ALPHA_ONE);

	// XXX this is supposed to pick a correct light colour for the object twinkle.
	// Not quite correct, since it always uses the first light
	GLfloat col[4];
	glGetLightfv(GL_LIGHT0, GL_DIFFUSE, col);

	static VertexArray va(ATTRIB_POSITION | ATTRIB_DIFFUSE);
	va.Clear();

	const Color center(col[0]*255, col[1]*255, col[2]*255, 255);
	const Color edges(col[0]*255, col[1]*255, col[2]*255, 0);

	//center
	va.Add(vector3f(0.f), center);

	const float spikerad = float(scale*rad);

	// bezier with (0,0,0) control points
	{
		const vector3f p0(0,spikerad,0), p1(spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(spikerad,0,0), p1(0,-spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(0,-spikerad,0), p1(-spikerad,0,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}
	{
		const vector3f p0(-spikerad,0,0), p1(0,spikerad,0);
		float t=0.1f; for (int i=1; i<10; i++, t+= 0.1f) {
			const vector3f p = (1-t)*(1-t)*p0 + t*t*p1;
			va.Add(p, edges);
		}
	}

	m_renderer->SetTransform(trans);
	m_renderer->DrawTriangles(&va, Graphics::vtxColorMaterial, TRIANGLE_FAN);
	m_renderer->SetBlendMode(BLEND_SOLID);
	m_renderer->SetDepthTest(true);
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
	for (Space::BodyIterator ib2 = Pi::game->GetSpace()->BodiesBegin(); ib2 != Pi::game->GetSpace()->BodiesEnd(); ++ib2) {
		Body *b2 = *ib2;
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
