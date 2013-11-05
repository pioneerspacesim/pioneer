#ifndef _HUDTRAIL_H
#define _HUDTRAIL_H

// trail drawn after an object to track motion

#include "libs.h"
#include "Body.h"
#include "graphics/Renderer.h"

class HudTrail
{
public:
	HudTrail(Body *b, const Color&);
	void Update(float time);
	void Render(Graphics::Renderer *r);

	void SetColor(const Color &c) { m_color = c; }
	void SetTransform(const matrix4x4d &t) { m_transform = t; }

private:
	Body *m_body;
	float m_updateTime;
	Color m_color;
	matrix4x4d m_transform;
	std::deque<vector3d> m_trailPoints;
};

#endif
