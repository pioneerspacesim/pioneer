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

BufferThing::BufferThing()
{
	buffy = 0;
	numSurfaces = 0;
	surfaces = 0;
	cached = false;
	primitiveType = TRIANGLES;
}

BufferThing::BufferThing(int n)
{
	buffy = 0;
	numSurfaces = 0;
	surfaces = 0;
	cached = false;
	primitiveType = TRIANGLES;

	if (n > 0) {
		numSurfaces = n;
		surfaces = new Surface[n];
	}
}

BufferThing::~BufferThing()
{
	/*if (surfaces)
		delete[] surfaces;*/
}

Renderer::Renderer(int w, int h) :
	m_width(w), m_height(h)
{

}

Renderer::~Renderer()
{

}