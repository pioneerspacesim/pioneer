// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StaticMesh.h"
#include "Surface.h"

namespace Graphics {

StaticMesh::StaticMesh(PrimitiveType t) :
	Renderable(),
	cached(false),
	m_primitiveType(t)
{
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::AddSurface(RefCountedPtr<Surface> s)
{
	m_surfaces.push_back(s);
}

int StaticMesh::GetNumVerts() const
{
	int numvertices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numvertices += (*surface)->GetNumVerts();
	return numvertices;
}

int StaticMesh::GetNumIndices() const
{
	int numIndices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numIndices += (*surface)->GetNumIndices();
	return numIndices;
}

int StaticMesh::GetAvailableVertexSpace() const
{
	return MAX_VERTICES - GetNumVerts();
}

AttributeSet StaticMesh::GetAttributeSet() const
{
	//all vertices should match
	AttributeSet set = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		if ((*surface)->GetVertices())
			set = (*surface)->GetVertices()->GetAttributeSet();
	return set;
}

}
