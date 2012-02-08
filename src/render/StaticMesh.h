#ifndef _STATICMESH_H
#define _STATICMESH_H

#include <vector>
#include "Renderer.h"

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
	StaticMesh();
	StaticMesh(int num_surfaces, PrimitiveType t);
	~StaticMesh();
	int GetNumVerts() const;

	int numSurfaces;
	Surface *surfaces;
	bool cached;

private:
	friend class Renderer;
	friend class RendererLegacy;
	friend class RendererGL2;
	RenderInfo *m_renderInfo;
};

#endif
