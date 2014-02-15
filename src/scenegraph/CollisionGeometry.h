// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_COLLISIONGEOMETRY_H
#define _SCENEGRAPH_COLLISIONGEOMETRY_H

/*
 * Non-renderable geometry node which CollisionVisitor can use
 * to build a collision mesh.
 */

#include "Node.h"

namespace Graphics { class Surface; }

class GeomTree;
class Geom;

namespace SceneGraph {
class CollisionGeometry : public Node {
public:
	CollisionGeometry(Graphics::Renderer *r, const std::vector<vector3f>&, const std::vector<unsigned short>&, unsigned int flag);
	CollisionGeometry(const CollisionGeometry&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "CollisionGeometry"; }
	virtual void Accept(NodeVisitor &nv);
	virtual void Save(NodeDatabase&) override;
	static CollisionGeometry *Load(NodeDatabase&);

	const std::vector<vector3f> &GetVertices() const { return m_vertices; }
	const std::vector<Uint16> &GetIndices() const { return m_indices; }
	unsigned int GetTriFlag() const { return m_triFlag; }

	bool IsDynamic() const { return m_dynamic; }
	void SetDynamic(bool b) { m_dynamic = b; }

	//for linking game collision objects with these nodes
	GeomTree *GetGeomTree() const { return m_geomTree; }
	void SetGeomTree(GeomTree *c) { m_geomTree = c; }

	Geom *GetGeom() const { return m_geom; }
	void SetGeom(Geom *g) { m_geom = g; }

protected:
	~CollisionGeometry();

private:
	void CopyData(const std::vector<vector3f>&, const std::vector<unsigned short>&);
	std::vector<vector3f> m_vertices;
	std::vector<Uint16> m_indices;
	unsigned int m_triFlag; //only one per node
	bool m_dynamic;

	//for dynamic collisions
	GeomTree *m_geomTree;
	Geom *m_geom;
};
}

#endif
