// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "HudTrail.h"
#include "Pi.h"
#include "graphics/RenderState.h"

const float UPDATE_INTERVAL = 0.1f;
const Uint16 MAX_POINTS = 100;

HudTrail::HudTrail(Body *b, const Color& c)
: m_body(b)
, m_updateTime(0.f)
, m_color(c)
{
	m_currentFrame = b->GetFrame();

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	m_renderState = Pi::renderer->CreateRenderState(rsd);
}

void HudTrail::Update(float time)
{
	//record position
	m_updateTime += time;
	if (m_updateTime > UPDATE_INTERVAL) {
		m_updateTime = 0.f;
		const Frame *bodyFrame = m_body->GetFrame();
		if( bodyFrame==m_currentFrame )
			m_trailPoints.push_back(m_body->GetInterpPosition());
	}

	while (m_trailPoints.size() > MAX_POINTS)
		m_trailPoints.pop_front();
}

void HudTrail::Render(Graphics::Renderer *r)
{
	//render trail
	if (m_trailPoints.size() > 1) {
		const vector3d vpos = m_transform * m_body->GetInterpPosition();
		m_transform[12] = vpos.x;
		m_transform[13] = vpos.y;
		m_transform[14] = vpos.z;
		m_transform[15] = 1.0;

		static std::vector<vector3f> tvts;
		static std::vector<Color> colors;
		tvts.clear();
		colors.clear();
		const vector3d curpos = m_body->GetInterpPosition();
		tvts.push_back(vector3f(0.f));
		colors.push_back(Color(0.f));
		float alpha = 1.f;
		const float decrement = 1.f / m_trailPoints.size();
		const Color tcolor = m_color;
		for (Uint16 i = m_trailPoints.size()-1; i > 0; i--) {
			tvts.push_back(-vector3f(curpos - m_trailPoints[i]));
			alpha -= decrement;
			colors.push_back(tcolor);
			colors.back().a = Uint8(alpha * 255);
		}

		r->SetTransform(m_transform);
		r->DrawLines(tvts.size(), &tvts[0], &colors[0], m_renderState, Graphics::LINE_STRIP);
	}
}

void HudTrail::Reset(const Frame *newFrame)
{
	m_currentFrame = newFrame;
	m_trailPoints.clear();
}
