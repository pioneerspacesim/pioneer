// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "HudTrail.h"

#include "Body.h"
#include "Frame.h"
#include "Pi.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"

const float UPDATE_INTERVAL = 0.1f;
const Uint16 MAX_POINTS = 100;

HudTrail::HudTrail(Body *b, const Color &c) :
	m_body(b),
	m_updateTime(0.f),
	m_color(c)
{
	m_currentFrame = b->GetFrame();

	Graphics::MaterialDescriptor desc;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.primitiveType = Graphics::LINE_STRIP;
	m_lineMat.reset(Pi::renderer->CreateMaterial("vtxColor", desc, rsd));
}

void HudTrail::Update(float time)
{
	PROFILE_SCOPED();
	//record position
	m_updateTime += time;
	if (m_updateTime > UPDATE_INTERVAL) {
		m_updateTime = 0.f;
		FrameId bodyFrameId = m_body->GetFrame();

		if (!m_currentFrame) {
			m_currentFrame = bodyFrameId;
			m_trailPoints.clear();
		}

		if (bodyFrameId == m_currentFrame)
			m_trailPoints.push_back(m_body->GetInterpPosition());
	}

	while (m_trailPoints.size() > MAX_POINTS)
		m_trailPoints.pop_front();
}

void HudTrail::Render(Graphics::Renderer *r)
{
	PROFILE_SCOPED();
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
		tvts.reserve(MAX_POINTS);
		colors.reserve(MAX_POINTS);

		tvts.push_back(vector3f(0.f));
		colors.push_back(Color::BLANK);
		float alpha = 1.f;
		const float decrement = 1.f / m_trailPoints.size();
		const Color tcolor = m_color;
		for (size_t i = m_trailPoints.size() - 1; i > 0; i--) {
			tvts.push_back(-vector3f(curpos - m_trailPoints[i]));
			alpha -= decrement;
			colors.push_back(tcolor);
			colors.back().a = Uint8(alpha * 255);
		}

		r->SetTransform(matrix4x4f(m_transform));
		m_lines.SetData(tvts.size(), &tvts[0], &colors[0]);
		m_lines.Draw(r, m_lineMat.get());
	}
}

void HudTrail::Reset(FrameId newFrame)
{
	m_currentFrame = newFrame;
	m_trailPoints.clear();
}
