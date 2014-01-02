// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATICGEOMETRY_H
#define _STATICGEOMETRY_H
/*
 * Geometry node containing one StaticMesh. Nothing fancy.
 */
#include "Node.h"
#include "Aabb.h"
#include "graphics/Renderer.h"
#include "graphics/StaticMesh.h"
#include "SmartPtr.h"

namespace SceneGraph {

class NodeVisitor;

class StaticGeometry : public Node
{
public:
	StaticGeometry(Graphics::Renderer *r);
	StaticGeometry(const StaticGeometry&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "StaticGeometry"; }
	virtual void Accept(NodeVisitor &nv);
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);

	void AddMesh(RefCountedPtr<Graphics::StaticMesh>);
	unsigned int GetNumMeshes() const { return m_meshes.size(); }
	RefCountedPtr<Graphics::StaticMesh> GetMesh(unsigned int i) { return m_meshes.at(i); }

	Aabb m_boundingBox;
	Graphics::BlendMode m_blendMode;

protected:
	~StaticGeometry();
	void DrawBoundingBox(const Aabb &bb);
	std::vector<RefCountedPtr<Graphics::StaticMesh> > m_meshes;
	typedef std::vector<RefCountedPtr<Graphics::StaticMesh> > MeshContainer;
};

}
#endif
