// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/gl2/GL2VertexBuffer.h"

namespace Graphics { namespace GL2 {

GLint get_num_components(VertexAttribFormat fmt)
{
	switch (fmt) {
	case ATTRIB_FORMAT_FLOAT2:
		return 2;
	case ATTRIB_FORMAT_FLOAT3:
		return 3;
	case ATTRIB_FORMAT_FLOAT4:
	case ATTRIB_FORMAT_UBYTE4:
		return 4;
	default:
		assert(false);
		return 0;
	}
}

GLenum get_component_type(VertexAttribFormat fmt)
{
	switch (fmt) {
	case ATTRIB_FORMAT_UBYTE4:
		return GL_UNSIGNED_BYTE;
	case ATTRIB_FORMAT_FLOAT2:
	case ATTRIB_FORMAT_FLOAT3:
	case ATTRIB_FORMAT_FLOAT4:
	default:
		return GL_FLOAT;
	}
}

VertexBuffer::VertexBuffer(const VertexBufferDesc &desc)
{
	m_desc = desc;
	//update offsets
	for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
		if (m_desc.attrib[i].offset == 0)
			m_desc.attrib[i].offset = m_desc.GetOffset(m_desc.attrib[i].semantic);
	}
	SetVertexCount(m_desc.numVertices);

	glGenBuffers(1, &m_buffer);

	//allocate initial data
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	const Uint32 dataSize = m_desc.numVertices * m_desc.GetVertexSize();
	m_data = new Uint8[dataSize];
	memset(m_data, 0, dataSize);
	const GLenum usage = (m_desc.usage == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, dataSize, m_data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_buffer);
	delete[] m_data;
}

Uint8 *VertexBuffer::MapInternal(BufferMapMode mode)
{
	assert(mode != BUFFER_MAP_NONE); //makes no sense
	assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
	m_mapMode = mode;
	return m_data;
}

void VertexBuffer::Unmap()
{
	assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

	if (m_mapMode == BUFFER_MAP_WRITE) {
		const GLsizei dataSize = m_desc.numVertices * m_desc.GetVertexSize();
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_mapMode = BUFFER_MAP_NONE;
}

void VertexBuffer::SetAttribPointers()
{
	const Uint32 stride = m_desc.GetVertexSize();
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		const auto& attr  = m_desc.attrib[i];
		const auto offset = reinterpret_cast<const GLvoid*>(m_desc.attrib[i].offset);
		switch (attr.semantic) {
		case ATTRIB_POSITION:
			glVertexPointer(get_num_components(attr.format), get_component_type(attr.format), stride, offset);
			break;
		case ATTRIB_NORMAL:
			glNormalPointer(get_component_type(attr.format), stride, offset);
			break;
		case ATTRIB_DIFFUSE:
			glColorPointer(get_num_components(attr.format), get_component_type(attr.format), stride, offset);
			break;
		case ATTRIB_UV0:
			glTexCoordPointer(get_num_components(attr.format), get_component_type(attr.format), stride, offset);
			break;
		case ATTRIB_NONE:
		default:
			return;
		}
	}
}

void VertexBuffer::UnsetAttribPointers()
{
}

IndexBuffer::IndexBuffer(Uint32 size, BufferUsage hint)
	: Graphics::IndexBuffer(size)
{
	const GLenum usage = (hint == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
	m_data = new Uint16[size];
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint16) * m_size, m_data, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_buffer);
	delete[] m_data;
}

Uint16 *IndexBuffer::Map(BufferMapMode mode)
{
	assert(mode != BUFFER_MAP_NONE); //makes no sense
	assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
	m_mapMode = mode;
	return m_data;
}

void IndexBuffer::Unmap()
{
	assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

	if (m_mapMode == BUFFER_MAP_WRITE) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint16) * m_size, m_data);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	m_mapMode = BUFFER_MAP_NONE;
}


} }
