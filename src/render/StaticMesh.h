#ifndef _STATICMESH_H
#define _STATICMESH_H

#include <vector>

class Surface;

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
	// XXX gl specific hack (stores vbo id)
	unsigned int buffy;
};

#endif
