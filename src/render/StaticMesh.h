#ifndef _STATICMESH_H
#define _STATICMESH_H

#include "Renderer.h"
#include "VertexArray.h"
#include "Surface.h"
#include <vector>

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

	void AddSurface(Surface *s);
	Surface *GetSurface(int idx) const { return m_surfaces.at(idx); }

	//useful to know for buffers
	int GetNumVerts() const;
	int GetNumIndices() const;

	//blarf
	AttributeSet GetAttributeSet() const;

	bool cached;

private:
	friend class Renderer;
	friend class RendererLegacy;
	friend class RendererGL2;
	PrimitiveType m_primitiveType;
	SurfaceList m_surfaces;
};

#endif
