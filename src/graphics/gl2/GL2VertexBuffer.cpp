// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/gl2/GL2VertexBuffer.h"

namespace Graphics { namespace GL2 {

VertexBuffer::VertexBuffer(const VertexBufferDesc &desc)
{
	m_desc = desc;
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

Uint8 *VertexBuffer::MapInternal(BufferMapMode)
{
	return m_data;
}

void VertexBuffer::Unmap()
{
	const GLsizei dataSize = m_desc.numVertices * m_desc.GetVertexSize();
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetAttribPointers()
{
	const Uint32 stride = m_desc.GetVertexSize();
	GLsizei offset = 0;
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		auto sem = m_desc.attrib[i].semantic;
		if (sem == ATTRIB_POSITION)
			glVertexPointer(3, GL_FLOAT, stride, (GLvoid*)offset);
		else if (sem == ATTRIB_NORMAL)
			glNormalPointer(GL_FLOAT, stride, (GLvoid*)offset);
		else if (sem == ATTRIB_DIFFUSE)
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)offset);
		else if (sem == ATTRIB_UV0)
			glTexCoordPointer(2, GL_FLOAT, stride, (GLvoid*)offset);
		else
			break;
		//glVertexAttribPointer(i, MapNumComponents(m_desc.attrib[i].format),
		//	MapType(m_desc.attribs[i].format), GL_TRUE, stride, (GLvoid*)offset);
		offset += m_desc.GetAttribSize(m_desc.attrib[i].format);
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

Uint16 *IndexBuffer::Map(BufferMapMode)
{
	return m_data;
}

void IndexBuffer::Unmap()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Uint16) * m_size, m_data);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


} }
