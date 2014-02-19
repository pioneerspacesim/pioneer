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
	//update offsets in desc
	for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
		if (m_desc.attrib[i].offset == 0)
			m_desc.attrib[i].offset = VertexBufferDesc::CalculateOffset(m_desc, m_desc.attrib[i].semantic);
	}

	//update stride in desc (respecting offsets)
	if (m_desc.stride == 0)
	{
		Uint32 lastAttrib = 0;
		while (lastAttrib < MAX_ATTRIBS) {
			if (m_desc.attrib[lastAttrib].semantic == ATTRIB_NONE)
				break;
			lastAttrib++;
		}

		m_desc.stride = m_desc.attrib[lastAttrib].offset + VertexBufferDesc::GetAttribSize(m_desc.attrib[lastAttrib].format);
	}
	assert(m_desc.stride > 0);
	assert(m_desc.numVertices > 0);

	SetVertexCount(m_desc.numVertices);

	glGenBuffers(1, &m_buffer);

	//Allocate initial data store
	//Using zeroed m_data is not mandatory, but otherwise contents are undefined
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	const Uint32 dataSize = m_desc.numVertices * m_desc.stride;
	m_data = new Uint8[dataSize];
	memset(m_data, 0, dataSize);
	const GLenum usage = (m_desc.usage == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, dataSize, m_data, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Don't keep client data around for static buffers
	if (GetDesc().usage == BUFFER_USAGE_STATIC) {
		delete[] m_data;
		m_data = nullptr;
	}

	//If we had VAOs could set up the pointers already
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
	if (GetDesc().usage == BUFFER_USAGE_STATIC) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		if (mode == BUFFER_MAP_READ)
			return reinterpret_cast<Uint8*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
		else if (mode == BUFFER_MAP_WRITE)
			return reinterpret_cast<Uint8*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	}

	return m_data;
}

void VertexBuffer::Unmap()
{
	assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

	if (GetDesc().usage == BUFFER_USAGE_STATIC) {
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	} else {
		if (m_mapMode == BUFFER_MAP_WRITE) {
			const GLsizei dataSize = m_desc.numVertices * m_desc.stride;
			glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	m_mapMode = BUFFER_MAP_NONE;
}

void VertexBuffer::SetAttribPointers()
{
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		const auto& attr  = m_desc.attrib[i];
		const auto offset = reinterpret_cast<const GLvoid*>(m_desc.attrib[i].offset);
		switch (attr.semantic) {
		case ATTRIB_POSITION:
			glVertexPointer(get_num_components(attr.format), get_component_type(attr.format), m_desc.stride, offset);
			break;
		case ATTRIB_NORMAL:
			glNormalPointer(get_component_type(attr.format), m_desc.stride, offset);
			break;
		case ATTRIB_DIFFUSE:
			glColorPointer(get_num_components(attr.format), get_component_type(attr.format), m_desc.stride, offset);
			break;
		case ATTRIB_UV0:
			glTexCoordPointer(get_num_components(attr.format), get_component_type(attr.format), m_desc.stride, offset);
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
	: Graphics::IndexBuffer(size, hint)
{
	assert(size > 0);

	const GLenum usage = (hint == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
	m_data = new Uint16[size];
	memset(m_data, 0, sizeof(Uint16) * size);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Uint16) * m_size, m_data, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Don't keep client data around for static buffers
	if (GetUsage() == BUFFER_USAGE_STATIC) {
		delete[] m_data;
		m_data = nullptr;
	}
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
	if (GetUsage() == BUFFER_USAGE_STATIC) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
		if (mode == BUFFER_MAP_READ)
			return reinterpret_cast<Uint16*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));
		else if (mode == BUFFER_MAP_WRITE)
			return reinterpret_cast<Uint16*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
	}

	return m_data;
}

void IndexBuffer::Unmap()
{
	assert(m_mapMode != BUFFER_MAP_NONE); //not currently mapped

	if (GetUsage() == BUFFER_USAGE_STATIC) {
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		if (m_mapMode == BUFFER_MAP_WRITE) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint16) * m_size, m_data);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	m_mapMode = BUFFER_MAP_NONE;
}


} }
