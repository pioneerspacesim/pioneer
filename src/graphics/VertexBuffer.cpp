// Copyright � 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/VertexBuffer.h"

namespace Graphics {

Uint32 VertexBufferDesc::GetAttribSize(VertexAttribFormat f)
{
	switch (f) {
	case ATTRIB_FORMAT_FLOAT2:
		return 8;
	case ATTRIB_FORMAT_FLOAT3:
		return 12;
	case ATTRIB_FORMAT_FLOAT4:
		return 16;
	case ATTRIB_FORMAT_UBYTE4:
		return 4;
	default:
		return 0;
	}
}

VertexBufferDesc::VertexBufferDesc()
	: numVertices(0)
	, stride(0)
	, usage(BUFFER_USAGE_STATIC)
{
	for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
		attrib[i].semantic = ATTRIB_NONE;
		attrib[i].format = ATTRIB_FORMAT_NONE;
		attrib[i].offset = 0;
	}

	assert(sizeof(vector2f) == 8);
	assert(sizeof(vector3f) == 12);
	assert(sizeof(Color4ub) == 4);
}

Uint32 VertexBufferDesc::GetOffset(VertexAttrib attr) const
{
	for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
		if (attrib[i].semantic == attr)
			return attrib[i].offset;
	}

	//attrib not found
	assert(false);
	return 0;
}

Uint32 VertexBufferDesc::CalculateOffset(const VertexBufferDesc &desc, VertexAttrib attr)
{
	Uint32 offs = 0;
	for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
		if (desc.attrib[i].semantic == attr)
			return offs;
		offs += GetAttribSize(desc.attrib[i].format);
	}

	//attrib not found
	assert(false);
	return 0;
}

VertexBuffer::~VertexBuffer()
{
}

Uint32 VertexBuffer::GetVertexCount() const
{
	return m_numVertices;
}

void VertexBuffer::SetVertexCount(Uint32 v)
{
	assert(v <= m_desc.numVertices);
	m_numVertices = v;
}

IndexBuffer::IndexBuffer(Uint32 size, BufferUsage usage)
	: m_size(size)
	, m_indexCount(size)
	, m_usage(usage)
{
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::SetIndexCount(Uint32 ic)
{
	assert(ic <= GetSize());
	m_indexCount = std::min(ic, GetSize());
}

}
