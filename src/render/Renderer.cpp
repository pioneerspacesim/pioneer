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

void VertexArray::Add(const vector3f &v, const Color &c, const vector3f n)
{
	position.push_back(v);
	diffuse.push_back(c);
	normal.push_back(n);
}

Renderer::Renderer(int w, int h) :
	m_width(w), m_height(h)
{

}

Renderer::~Renderer()
{

}