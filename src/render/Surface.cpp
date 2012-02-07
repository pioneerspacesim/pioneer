#include "Surface.h"
#include "Material.h"
#include "VertexArray.h"

Surface::Surface() :
	m_vertices(0), m_material(0), primitiveType(TRIANGLES)
{
}

Surface::~Surface()
{
	delete m_vertices;
}

void Surface::SetVertices(VertexArray *v)
{
	if (m_vertices) delete m_vertices;

	m_vertices = v;
}

int Surface::GetNumVerts() const
{
	if (m_vertices)
		return m_vertices->position.size();
	else
		return 0;
}
