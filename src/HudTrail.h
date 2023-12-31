// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _HUDTRAIL_H
#define _HUDTRAIL_H

#include "Color.h"
#include "FrameId.h"
#include "graphics/Drawables.h"
#include "matrix4x4.h"

#include <deque>
// trail drawn after an object to track motion

namespace Graphics {
	class Renderer;
} // namespace Graphics

class Body;
class Frame;

class HudTrail {
public:
	HudTrail(Body *b, const Color &);
	void Update(float time);
	void Render(Graphics::Renderer *r);
	void Reset(const FrameId newFrame);

	void SetColor(const Color &c) { m_color = c; }
	void SetTransform(const matrix4x4d &t) { m_transform = t; }

private:
	Body *m_body;
	FrameId m_currentFrame;
	float m_updateTime;
	Color m_color;
	matrix4x4d m_transform;
	std::deque<vector3d> m_trailPoints;
	std::unique_ptr<Graphics::Material> m_lineMat;
	Graphics::Drawables::Lines m_lines;
};

#endif
