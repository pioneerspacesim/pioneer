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
	int GetNumIndices() const { return indices.size(); }
	std::vector<unsigned short> indices;

	//deletes existing, takes ownership
	void SetVertices(VertexArray *v);
	VertexArray *GetVertices() const { return m_vertices; }

	void SetMaterial(RefCountedPtr<Material> m) { m_material = m; }
	RefCountedPtr<Material> GetMaterial() const { return m_material; }

private:
	friend class StaticMesh;
	friend class Renderer;
	friend class RendererLegacy;
	friend class RendererGL2;
	PrimitiveType m_primitiveType;
	RefCountedPtr<Material> m_material;
	VertexArray *m_vertices;
	// multiple surfaces can be buffered in one vbo so need to
	// save starting offset + amount to draw
	//XXX temporary
	int glOffset; //index start OR vertex start
	int glAmount; //index count OR vertex amount
};

typedef std::vector<Surface*> SurfaceList;

#endif
