#ifndef _SURFACE_H
#define _SURFACE_H

#include "Material.h"
#include "RefCounted.h"
#include "Renderer.h"
#include "VertexArray.h"
#include <vector>

// surface with a material
// can have indices
class Surface {
public:
	Surface(PrimitiveType t);
	~Surface();
	int GetNumVerts() const;
	std::vector<unsigned short> indices;

	//deletes existing, takes ownership
	void SetVertices(VertexArray *v);
	VertexArray *GetVertices() const { return m_vertices; }

	void SetMaterial(RefCountedPtr<Material> m) { m_material = m; }
	RefCountedPtr<Material> GetMaterial() const { return m_material; }

private:
	Surface();
	friend class StaticMesh;
	friend class Renderer;
	friend class RendererLegacy;
	friend class RendererGL2;
	PrimitiveType m_primitiveType;
	RefCountedPtr<Material> m_material;
	VertexArray *m_vertices;
	// multiple surfaces can be buffered in one vbo so need to
	// save starting offset + amount to draw
	//int startVertex;
	//int numVertices; should be samme as vertices->GetNumVerts()
	//int startIndex;
	//int numIndices; should be same as indices.size()
};

#endif
