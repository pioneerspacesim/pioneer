#ifndef _STATICMESH_H
#define _STATICMESH_H

#include "Renderer.h"
#include "VertexArray.h"
#include "Surface.h"
#include <vector>

class Surface;

// subclass to store renderer specific information
struct RenderInfo {
	RenderInfo() { }
	virtual ~RenderInfo() { }
};

// Geometry that changes rarely or never
// May be cached by the renderer
// Can hold multiple surfaces
class StaticMesh {
public:
	StaticMesh(PrimitiveType t);
	~StaticMesh();

	Surface *AddSurface();
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
	RenderInfo *m_renderInfo;
	PrimitiveType m_primitiveType;
	SurfaceList m_surfaces;
};

#endif
