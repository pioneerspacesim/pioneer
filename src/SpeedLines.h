// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);

	Ship *m_ship;

	std::vector<vector3f> m_points;
	
	Graphics::RenderState *m_renderState;
	RefCountedPtr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexArray> m_varray;
	std::unique_ptr<Graphics::VertexBuffer> m_vbuffer;

	matrix4x4d m_transform;
	
	bool m_visible;
	float m_lineLength;
	vector3f m_dir;
};

#endif
