#include "StaticMesh.h"
#include "Surface.h"

StaticMesh::StaticMesh(PrimitiveType t) :
	cached(false),
	m_renderInfo(0),
	m_primitiveType(t)
{
}

StaticMesh::~StaticMesh()
{
	while (!m_surfaces.empty()) delete m_surfaces.back(), m_surfaces.pop_back();
	delete m_renderInfo;
}

Surface *StaticMesh::AddSurface()
{
	Surface *s = new Surface(m_primitiveType);
	m_surfaces.push_back(s);
	return s;
}

int StaticMesh::GetNumVerts() const
{
	int numvertices = 0;
	for (SurfaceList::const_iterator surface = m_surfaces.begin();
		surface != m_surfaces.end(); ++surface)
	{
		numvertices += (*surface)->GetNumVerts();
	}
	return numvertices;
}

int StaticMesh::GetNumIndices() const
{
	int numIndices = 0;
	for (SurfaceList::const_iterator surface = m_surfaces.begin();
		surface != m_surfaces.end(); ++surface)
	{
		numIndices += (*surface)->GetNumIndices();
	}
	return numIndices;
}

AttributeSet StaticMesh::GetAttributeSet() const
{
	//all vertices should match
	AttributeSet set = 0;
	for (SurfaceList::const_iterator surface = m_surfaces.begin();
		surface != m_surfaces.end(); ++surface)
	{
		set = (*surface)->GetVertices()->GetAttributeSet();
	}
	return set;
}
