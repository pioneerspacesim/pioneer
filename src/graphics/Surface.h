// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SURFACE_H
#define _SURFACE_H

#include "Material.h"
#include "RefCounted.h"
#include "Renderer.h"
#include "VertexArray.h"
#include <vector>

namespace Graphics {

/*
 * Surface is a container for a vertex array, a material
 * and an index array. Intended for indexed triangle drawing.
 */
class Surface : public Renderable {
public:
	Surface(PrimitiveType primitiveType, VertexArray *vertices, RefCountedPtr<Material> material = RefCountedPtr<Material>(0)):
		m_primitiveType(primitiveType), m_vertices(vertices), m_material(material) {}
	virtual ~Surface() {}

	PrimitiveType GetPrimtiveType() const { return m_primitiveType; }
	VertexArray *GetVertices() const { return m_vertices.get(); }
	RefCountedPtr<Material> GetMaterial() const { return m_material; }
	void SetMaterial(RefCountedPtr<Material> m) { m_material = m; }

	int GetNumVerts() const { return m_vertices ? m_vertices->position.size() : 0; }
	int GetNumIndices() const { return m_indices.size(); }
	std::vector<unsigned short> &GetIndices() { return m_indices; }
	const unsigned short *GetIndexPointer() const { return &m_indices[0]; }

	bool IsIndexed() const { return !m_indices.empty(); }

private:
	PrimitiveType m_primitiveType;
	std::unique_ptr<VertexArray> m_vertices;
	RefCountedPtr<Material> m_material;

	std::vector<unsigned short> m_indices;
};

}

#endif
