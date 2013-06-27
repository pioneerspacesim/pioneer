// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_THRUSTER_H
#define _SCENEGRAPH_THRUSTER_H
/*
 * Spaceship thruster
 */
#include "libs.h"
#include "Node.h"

namespace Graphics {
	class Renderer;
	class VertexArray;
	class Material;
}

namespace SceneGraph {

class Thruster : public Node {
public:
	Thruster(Graphics::Renderer *, bool linear, const vector3f &pos, const vector3f &dir);
	Thruster(const Thruster&, NodeCopyCache *cache = 0);
	Node *Clone(NodeCopyCache *cache = 0);
	virtual void Accept(NodeVisitor &v);
	virtual const char *GetTypeName() const { return "Thruster"; }
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);

private:
	static Graphics::VertexArray* CreateGeometry();
	RefCountedPtr<Graphics::Material> m_tMat;
	ScopedPtr<Graphics::VertexArray> m_tVerts;
	bool linearOnly;
	vector3f dir;
	vector3f pos;
};

}

#endif
