// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPEEDLINES_H
#define _SPEEDLINES_H

#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "matrix4x4.h"
#include "vector3.h"

#include <memory>
#include <vector>

namespace Graphics {
	class Renderer;
} // namespace Graphics

class Ship;

// virtual space dust to give a sense of movement

class SpeedLines {
public:
	SpeedLines(Ship *);

	void Update(float time);
	void Render(Graphics::Renderer *);

	void SetTransform(const matrix4x4d &t) { m_transform = t; }

	Ship *GetShip() const { return m_ship; }

private:
	static void Init();
	static float BOUNDS;
	static int DEPTH;
	static float SPACING;
	static float MAX_VEL;

	void CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size);

	Ship *m_ship;

	std::vector<vector3f> m_points;

	RefCountedPtr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexArray> m_varray;
	std::unique_ptr<Graphics::MeshObject> m_mesh;

	matrix4x4d m_transform;

	bool m_visible;
	float m_lineLength;
	vector3f m_dir;
};

#endif
