// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_BILLBOARD_H
#define _SCENEGRAPH_BILLBOARD_H
/*
 * One or more billboard sprites, meant for lights mostly
 */
#include "Node.h"
#include "graphics/Material.h"

namespace SceneGraph {

class Billboard : public Node {
public:
	Billboard(RefCountedPtr<Graphics::Material>, float size);
	Billboard(const std::vector<vector3f>&, RefCountedPtr<Graphics::Material>, float size);
	virtual const char *GetTypeName() { return "Billboard"; }
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);

	void AddPoint(const vector3f &pt);

private:
	float m_size;
	RefCountedPtr<Graphics::Material> m_material;
	std::vector<vector3f> m_points;
};

}

#endif
