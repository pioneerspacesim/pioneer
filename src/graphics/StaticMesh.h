// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATICMESH_H
#define _STATICMESH_H

#include "Renderer.h"
#include "VertexArray.h"
#include "Surface.h"
#include <vector>

namespace Graphics {

/*
 * StaticMesh can hold multiple surfaces and is intended for complex,
 * unchanging geometry. Renderers can buffer the contents into VBOs or
 * whatever they prefer. The original vertex data is kept for reloading
 * on context switch.
 */
class StaticMesh : public Renderable {
public:
	StaticMesh(PrimitiveType t);
	~StaticMesh();

	PrimitiveType GetPrimtiveType() const { return m_primitiveType; }

	void AddSurface(RefCountedPtr<Surface>);
	RefCountedPtr<Surface> GetSurface(int idx) const { return m_surfaces.at(idx); }

	//useful to know for buffers
	int GetNumVerts() const;
	int GetNumIndices() const;
	int GetAvailableVertexSpace() const;

	AttributeSet GetAttributeSet() const;

	typedef std::vector<RefCountedPtr<Surface> >::const_iterator SurfaceIterator;
	const SurfaceIterator SurfacesBegin() const { return m_surfaces.begin(); }
	const SurfaceIterator SurfacesEnd() const { return m_surfaces.end(); }

	bool cached;

	static const int MAX_VERTICES = 65536;

private:
	PrimitiveType m_primitiveType;
	std::vector<RefCountedPtr<Surface> > m_surfaces;
};

}

#endif
