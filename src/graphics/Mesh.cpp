#include "Mesh.h"
#include "Surface.h"

namespace Graphics {

Mesh::Mesh(PrimitiveType t) :
	Renderable(),
	cached(false),
	m_primitiveType(t)
{
}

Mesh::~Mesh()
{
	while (!m_surfaces.empty()) delete m_surfaces.back(), m_surfaces.pop_back();
}

void Mesh::AddSurface(Surface *s)
{
	m_surfaces.push_back(s);
}

int Mesh::GetNumVerts() const
{
	int numvertices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numvertices += (*surface)->GetNumVerts();
	return numvertices;
}

int Mesh::GetNumIndices() const
{
	int numIndices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numIndices += (*surface)->GetNumIndices();
	return numIndices;
}

AttributeSet Mesh::GetAttributeSet() const
{
	//all vertices should match
	AttributeSet set = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		if ((*surface)->GetVertices())
			set = (*surface)->GetVertices()->GetAttributeSet();
	return set;
}

}
