// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SpeedLines.h"
#include "Ship.h"
#include "graphics/RenderState.h"
#include "Pi.h"

const float BOUNDS     = 2000.f;
const int   DEPTH      = 8;
const float SPACING    = 500.f;
const float MAX_VEL    = 100.f;

SpeedLines::SpeedLines(Ship *s)
: m_ship(s)
, m_visible(false)
, m_dir(0.f)
{
	m_points.reserve(DEPTH * DEPTH * DEPTH);
	for (int x = -DEPTH/2; x < DEPTH/2; x++) {
		for (int y = -DEPTH/2; y < DEPTH/2; y++) {
			for (int z = -DEPTH/2; z < DEPTH/2; z++) {
				m_points.push_back(vector3f(x * SPACING, y * SPACING, z * SPACING));
			}
		}
	}

	m_vertices.resize(m_points.size() * 2);
	m_vtxColors.resize(m_vertices.size());

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	m_renderState = Pi::renderer->CreateRenderState(rsd);
}

void SpeedLines::Update(float time)
{
	vector3f vel = vector3f(m_ship->GetVelocity());
	const float absVel = vel.Length();

	// don't show if
	//   vel < 100m/s
	//   in rotating frame (near station or planet surface)
	if (absVel < 100.f || m_ship->GetFrame()->IsRotFrame()) {
		m_visible = false;
		return;
	}
	m_visible = true;

	//slow lines down at higher speeds
	float mult;
	if (absVel > 100000.f)
		mult = 0.001f;
	else if (absVel > 10000.f)
		mult = 0.01f;
	else if (absVel > 5000.f)
		mult = 0.1f;
	else
		mult = 1.f;

	//rate of change (incl. time acceleration)
	float d = absVel * time * mult;

	m_lineLength = Clamp(absVel * 0.1f, 2.f, 100.f);
	m_dir = vel.Normalized();

	vel = vel * time * mult;

	//too fast to draw - cap
	if (d > MAX_VEL)
		vel = m_dir * MAX_VEL;

	for (Uint16 i = 0; i < m_points.size(); i++) {

		vector3f &pt = m_points[i];

		pt -= vel;

		//wrap around
		if (pt.x > BOUNDS)
			pt.x -= BOUNDS * 2.f;
		if (pt.x < -BOUNDS)
			pt.x += BOUNDS * 2.f;
		if (pt.y > BOUNDS)
			pt.y -= BOUNDS * 2.f;
		if (pt.y < -BOUNDS)
			pt.y += BOUNDS * 2.f;
		if (pt.z > BOUNDS)
			pt.z -= BOUNDS * 2.f;
		if (pt.z < -BOUNDS)
			pt.z += BOUNDS * 2.f;
	}
}

void SpeedLines::Render(Graphics::Renderer *r)
{
	if (!m_visible) return;

	const vector3f dir = m_dir * m_lineLength;

	Uint16 vtx = 0;
	for (auto it = m_points.begin(); it != m_points.end(); ++it) {
		m_vertices[vtx]   = *it - dir;
		m_vertices[vtx+1] = *it + dir;

		//distance fade
		const Color col = Color(Color::GRAY.r, Color::GRAY.g, Color::GRAY.b,
				Clamp((1.f - it->Length() / BOUNDS),0.f,1.f) * 255);
		m_vtxColors[vtx]   = col;
		m_vtxColors[vtx+1] = col;

		vtx += 2;
	}

	r->SetTransform(m_transform);
	r->DrawLines(m_vertices.size(), &m_vertices[0], &m_vtxColors[0], m_renderState);
}
