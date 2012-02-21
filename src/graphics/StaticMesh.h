#ifndef _STATICMESH_H
#define _STATICMESH_H

#include "Renderer.h"
#include "VertexArray.h"
#include <vector>

namespace Graphics {

class Surface;

/*
 * StaticMesh can hold multiple surfaces and is intended for complex,
 * unchanging geometry. Renderers can buffer the contents into VBOs or
 * whatever they prefer. In theory the original vertex data could be
 * thrown away... but perhaps it is better not to optimize that yet.
 */
class StaticMesh : public Renderable {
public:
	StaticMesh(PrimitiveType t);
	~StaticMesh();

	PrimitiveType GetPrimtiveType() const { return m_primitiveType; }

	void AddSurface(Surface *s);
	Surface *GetSurface(int idx) const { return m_surfaces.at(idx); }

	//useful to know for buffers
	int GetNumVerts() const;
	int GetNumIndices() const;

	//blarf
	AttributeSet GetAttributeSet() const;

	typedef std::vector<Surface*>::const_iterator SurfaceIterator;
	const SurfaceIterator SurfacesBegin() const { return m_surfaces.begin(); }
	const SurfaceIterator SurfacesEnd() const { return m_surfaces.end(); }

	bool cached;

private:
	PrimitiveType m_primitiveType;
	std::vector<Surface*> m_surfaces;
};

}

#endif
