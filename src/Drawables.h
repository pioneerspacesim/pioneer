#ifndef _DRAWABLES_H
#define _DRAWABLES_H

#include "libs.h"
#include "render/VertexArray.h"
#include "render/Renderer.h"

// A thing that can draw itself using renderer
// (circles, disks, polylines etc)
class Drawable {
public:
	virtual void Draw(Renderer *r) = 0;
};

class Circle {
public:
	Circle(float radius, const Color &c) : m_color(c) {
		for (float theta=0; theta < 2*M_PI; theta += 0.05*M_PI) {
			m_verts.push_back(vector3f(radius*sin(theta), radius*cos(theta), 0));
		}
	}
	virtual void Draw(Renderer *renderer) {
		renderer->DrawLines(m_verts.size(), &m_verts[0], m_color, LINE_LOOP);
	}

private:
	std::vector<vector3f> m_verts;
	Color m_color;
};

#endif
