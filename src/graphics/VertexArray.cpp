// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VertexArray.h"

#include "profiler/Profiler.h"

namespace Graphics {

	VertexArray::VertexArray(AttributeSet attribs, int size)
	{
		PROFILE_SCOPED()
		m_attribs = attribs;

		if (size > 0)
			Reserve(size);
	}

	VertexArray::~VertexArray()
	{
	}

	void VertexArray::Reserve(uint32_t size)
	{
		if (m_attribs & ATTRIB_POSITION)
			position.reserve(size);
		if (m_attribs & ATTRIB_DIFFUSE)
			diffuse.reserve(size);
		if (m_attribs & ATTRIB_NORMAL)
			normal.reserve(size);
		if (m_attribs & ATTRIB_UV0)
			uv0.reserve(size);
		if (m_attribs & ATTRIB_TANGENT)
			tangent.reserve(size);
	}

	void VertexArray::Clear(uint32_t size)
	{
		position.clear();
		diffuse.clear();
		normal.clear();
		uv0.clear();
		tangent.clear();

		if (size > 0)
			Reserve(size);
	}

	void VertexArray::Add(const vector3f &v)
	{
		position.emplace_back(v);
	}

	void VertexArray::Add(const vector3f &v, const Color &c)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
	}

	void VertexArray::Add(const vector3f &v, const Color &c, const vector3f &n)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
		normal.emplace_back(n);
	}

	void VertexArray::Add(const vector3f &v, const Color &c, const vector2f &uv)
	{
		position.emplace_back(v);
		diffuse.emplace_back(c);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector2f &uv)
	{
		position.emplace_back(v);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n, const vector2f &uv)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
		uv0.emplace_back(uv);
	}

	void VertexArray::Add(const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang)
	{
		position.emplace_back(v);
		normal.emplace_back(n);
		uv0.emplace_back(uv);
		tangent.emplace_back(tang);
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

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n)
	{
		position[idx] = v;
		normal[idx] = n;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv)
	{
		position[idx] = v;
		normal[idx] = n;
		uv0[idx] = uv;
	}

	void VertexArray::Set(const Uint32 idx, const vector3f &v, const vector3f &n, const vector2f &uv, const vector3f &tang)
	{
		position[idx] = v;
		normal[idx] = n;
		uv0[idx] = uv;
		tangent[idx] = tang;
	}

} // namespace Graphics
