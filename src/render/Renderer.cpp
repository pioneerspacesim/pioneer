#include "Renderer.h"
#include "Render.h"
#include "Texture.h"

VertexArray::VertexArray()
{
}

VertexArray::VertexArray(int size, bool c, bool n)
{
	position.reserve(size);

	if (c)
		diffuse.reserve(size);
	if (n)
		normal.reserve(size);
}

VertexArray::~VertexArray()
{

}

unsigned int VertexArray::GetNumVerts() const
{
	return position.size();
}

void VertexArray::Clear()
{
	position.clear();
	diffuse.clear();
	normal.clear();
	uv0.clear();
}

void VertexArray::Add(const vector3f &v)
{
	position.push_back(v);
}

void VertexArray::Add(const vector3f &v, const Color &c)
{
	position.push_back(v);
	diffuse.push_back(c);
}

void VertexArray::Add(const vector3f &v, const Color &c, const vector3f &n)
{
	position.push_back(v);
	diffuse.push_back(c);
	normal.push_back(n);
}

void VertexArray::Add(const vector3f &v, const Color &c, const vector2f &uv)
{
	position.push_back(v);
	diffuse.push_back(c);
	uv0.push_back(uv);
}

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