#ifndef _SPEEDLINES_H
#define _SPEEDLINES_H

// virtual space dust to give a sense of movement

#include "libs.h"
#include "graphics/Renderer.h"

class Ship;

class SpeedLines
{
public:
	SpeedLines(Ship*);

	void Update(float time);
	void Render(Graphics::Renderer*);

	void SetTransform(const matrix4x4d &t) { m_transform = t; }

	Ship *GetShip() const { return m_ship; }

private:
	Ship *m_ship;

	std::vector<vector3f> m_points;

	std::vector<vector3f> m_vertices;
	std::vector<Color> m_vtxColors;

	matrix4x4d m_transform;

	bool m_visible;
	float m_lineLength;
	vector3f m_dir;
	Color m_color;
};

#endif
