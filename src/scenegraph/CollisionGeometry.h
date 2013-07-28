// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_COLLISIONGEOMETRY_H
#define _SCENEGRAPH_COLLISIONGEOMETRY_H

/*
 * Non-renderable geometry node which CollisionVisitor can use
 * to build a collision mesh.
 */

#include "Node.h"

namespace Graphics { class Surface; }

namespace SceneGraph {

class CollisionGeometry : public Node {
public:
	CollisionGeometry(Graphics::Renderer *r, Graphics::Surface*, unsigned int flag);
	CollisionGeometry(Graphics::Renderer *r, const std::vector<vector3f>&, const std::vector<unsigned short>&, unsigned int flag);
	CollisionGeometry(const CollisionGeometry&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "CollisionGeometry"; }
	virtual void Accept(NodeVisitor &nv);

	const std::vector<vector3f> &GetVertices() const { return m_vertices; }
	const std::vector<int> &GetIndices() const { return m_indices; }
	unsigned int GetTriFlag() const { return m_triFlag; }

protected:
	~CollisionGeometry();

private:
	void CopyData(const std::vector<vector3f>&, const std::vector<unsigned short>&);
	std::vector<vector3f> m_vertices;
	std::vector<int> m_indices; //geomtree uses int
	unsigned int m_triFlag; //only one per node
};

}

#endif
