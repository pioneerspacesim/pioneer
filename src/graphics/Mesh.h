#ifndef _MESH_H
#define _MESH_H

#include "Renderer.h"
#include "VertexArray.h"
#include <vector>

namespace Graphics {

enum UsageHint {
	USAGE_STATIC, // used lots, good candidate for GPU buffering
	USAGE_DYNAMIC // used a few (one) times and then thrown away
};

class Surface;

/*
 * Mesh can hold multiple surfaces and is intended for complex,
 * unchanging geometry. Renderers can buffer the contents into VBOs or
 * whatever they prefer. In theory the original vertex data could be
 * thrown away... but perhaps it is better not to optimize that yet.
 */
class Mesh : public Renderable {
public:
	Mesh(PrimitiveType t, UsageHint usageHint = USAGE_STATIC);
	~Mesh();

	PrimitiveType GetPrimtiveType() const { return m_primitiveType; }
	UsageHint GetUsageHint() const { return m_usageHint; }

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
	UsageHint m_usageHint;
	std::vector<Surface*> m_surfaces;
};

}

#endif
