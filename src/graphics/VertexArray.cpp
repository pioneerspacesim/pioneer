// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VertexArray.h"

namespace Graphics {

VertexArray::VertexArray(AttributeSet attribs, int size)
{
	PROFILE_SCOPED()
	m_attribs = attribs;

	if (size > 0) {
		//would be rather weird without positions!
		if (attribs & ATTRIB_POSITION)
			position.reserve(size);
		if (attribs & ATTRIB_DIFFUSE)
			diffuse.reserve(size);
		if (attribs & ATTRIB_NORMAL)
			normal.reserve(size);
		if (attribs & ATTRIB_UV0)
			uv0.reserve(size);
	}
}

VertexArray::~VertexArray()
{

}

bool VertexArray::HasAttrib(VertexAttrib v) const
{
	return (m_attribs & v) != 0;
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

void VertexArray::Add(const vector3f &v, const vector2f &uv)
{
	position.push_back(v);
	uv0.push_back(uv);
}

void VertexArray::Add(const vector3f &v, const vector3f &n, const vector2f &uv)
{
	position.push_back(v);
	normal.push_back(n);
	uv0.push_back(uv);
}

void VertexArray::Set(const Uint32 idx, const vector3f &v)
{
	position[idx] = v;
}

void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c)
{
	position[idx] = v;
	diffuse[idx] = c;
}

void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c, const vector3f &n)
{
	position[idx] = v;
	diffuse[idx] = c;
	normal[idx] = n;
}

void VertexArray::Set(const Uint32 idx, const vector3f &v, const Color &c, const vector2f &uv)
{
	position[idx] = v;
	diffuse[idx] = c;
	uv0[idx] = uv;
}

void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector2f &uv)
{
	position[idx] = v;
	uv0[idx] = uv;
}

void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv)
{
	position[idx] = v;
	normal[idx] = n;
	uv0[idx] = uv;
}

}
