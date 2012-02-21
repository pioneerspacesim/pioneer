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
	VertexArray *GetVertices() const { return m_vertices.Get(); }
	RefCountedPtr<Material> GetMaterial() const { return m_material; }

	int GetNumVerts() const { return m_vertices ? m_vertices->position.size() : 0; }
	int GetNumIndices() const { return m_indices.size(); }
	const unsigned short *GetIndexPointer() const { return &m_indices[0]; }

	bool IsIndexed() { return !m_indices.empty(); }

private:
	PrimitiveType m_primitiveType;
	ScopedPtr<VertexArray> m_vertices;
	RefCountedPtr<Material> m_material;

	std::vector<unsigned short> m_indices;
};

}

#endif
