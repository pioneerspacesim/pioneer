#include "Renderer.h"
#include "Render.h"
#include "Texture.h"
#include "VertexArray.h"

int Surface::GetNumVerts() const
{
	if (vertices)
		return vertices->position.size();
	else
		return 0;
}

StaticMesh::StaticMesh()
{
	buffy = 0;
	numSurfaces = 0;
	surfaces = 0;
	cached = false;
}

StaticMesh::StaticMesh(int n)
{
	buffy = 0;
	numSurfaces = 0;
	surfaces = 0;
	cached = false;

	if (n > 0) {
		numSurfaces = n;
		surfaces = new Surface[n];
	}
}

StaticMesh::~StaticMesh()
{
	// does not delete vertex arrays or
	// materials, could solve with sharedptr
	delete[] surfaces;
}

int StaticMesh::GetNumVerts() const
{
	int numvertices = 0;
	for (int i=0; i < numSurfaces; i++) {
		numvertices += surfaces[i].GetNumVerts();
	}
	return numvertices;
}

Renderer::Renderer(int w, int h) :
	m_width(w), m_height(h)
{

}

Renderer::~Renderer()
{

}
