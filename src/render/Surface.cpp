#include "Surface.h"
#include "Material.h"
#include "VertexArray.h"

Surface::Surface() :
	m_material(0),
	m_primitiveType(TRIANGLES),
	m_vertices(0)
{
}

Surface::Surface(PrimitiveType t) :
	m_material(0),
	m_primitiveType(t),
	m_vertices(0)
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
