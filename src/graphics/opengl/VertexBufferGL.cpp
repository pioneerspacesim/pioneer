// Copyright � 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/opengl/VertexBufferGL.h"
#include "graphics/VertexArray.h"
#include "utils.h"

namespace Graphics { namespace OGL {

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

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_buffer);

	//Allocate initial data store
	//Using zeroed m_data is not mandatory, but otherwise contents are undefined
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	const Uint32 dataSize = m_desc.numVertices * m_desc.stride;
	m_data = new Uint8[dataSize];
	memset(m_data, 0, dataSize);
	const GLenum usage = (m_desc.usage == BUFFER_USAGE_STATIC) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
	glBufferData(GL_ARRAY_BUFFER, dataSize, m_data, usage);

	//Setup the VAO pointers
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		const auto& attr  = m_desc.attrib[i];
		if (attr.semantic == ATTRIB_NONE)
			break;

		// Tell OpenGL what the array contains
		const auto offset = reinterpret_cast<const GLvoid*>(attr.offset);
		switch (attr.semantic) {
		case ATTRIB_POSITION:
			glEnableVertexAttribArray(0);	// Enable the attribute at that location
			glVertexAttribPointer(0, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);	
			break;
		case ATTRIB_NORMAL:
			glEnableVertexAttribArray(1);	// Enable the attribute at that location
			glVertexAttribPointer(1, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
			break;
		case ATTRIB_DIFFUSE:
			glEnableVertexAttribArray(2);	// Enable the attribute at that location
			glVertexAttribPointer(2, get_num_components(attr.format), get_component_type(attr.format), GL_TRUE, m_desc.stride, offset);	// only normalise the colours
			break;
		case ATTRIB_UV0:
			glEnableVertexAttribArray(3);	// Enable the attribute at that location
			glVertexAttribPointer(3, get_num_components(attr.format), get_component_type(attr.format), GL_FALSE, m_desc.stride, offset);
			break;
		case ATTRIB_NONE:
		default:
			break;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Don't keep client data around for static buffers
	if (GetDesc().usage == BUFFER_USAGE_STATIC) {
		delete[] m_data;
		m_data = nullptr;
	}
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_buffer);
	glDeleteVertexArrays(1, &m_vao);
	delete[] m_data;
}

Uint8 *VertexBuffer::MapInternal(BufferMapMode mode)
{
	assert(mode != BUFFER_MAP_NONE); //makes no sense
	assert(m_mapMode == BUFFER_MAP_NONE); //must not be currently mapped
	m_mapMode = mode;
	if (GetDesc().usage == BUFFER_USAGE_STATIC) {
		glBindVertexArray(m_vao);
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
	glBindVertexArray(0);

	m_mapMode = BUFFER_MAP_NONE;
}

#pragma pack(push, 4)
struct PosUVVert {
	vector3f pos;
	vector2f uv;
};

struct PosColVert {
	vector3f pos;
	Color4ub col;
};

struct PosVert {
	vector3f pos;
};

struct PosColUVVert {
	vector3f pos;
	Color4ub col;
	vector2f uv;
};

struct PosNormUVVert {
	vector3f pos;
	vector3f norm;
	vector2f uv;
};
#pragma pack(pop)

void CopyPosUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
{
	PosUVVert* vtxPtr = vb->Map<PosUVVert>(Graphics::BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(PosUVVert));
	for(Uint32 i=0 ; i<va.GetNumVerts() ; i++)
	{
		vtxPtr[i].pos	= va.position[i];
		vtxPtr[i].uv	= va.uv0[i];
	}
	vb->Unmap();
}

void CopyPosCol(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
{
	PosColVert* vtxPtr = vb->Map<PosColVert>(Graphics::BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(PosColVert));
	for(Uint32 i=0 ; i<va.GetNumVerts() ; i++)
	{
		vtxPtr[i].pos	= va.position[i];
		vtxPtr[i].col	= va.diffuse[i];
	}
	vb->Unmap();
}

void CopyPos(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
{
	PosVert* vtxPtr = vb->Map<PosVert>(Graphics::BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(PosVert));
	for(Uint32 i=0 ; i<va.GetNumVerts() ; i++)
	{
		vtxPtr[i].pos	= va.position[i];
	}
	vb->Unmap();
}

void CopyPosColUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
{
	PosColUVVert* vtxPtr = vb->Map<PosColUVVert>(Graphics::BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(PosColUVVert));
	for(Uint32 i=0 ; i<va.GetNumVerts() ; i++)
	{
		vtxPtr[i].pos	= va.position[i];
		vtxPtr[i].col	= va.diffuse[i];
		vtxPtr[i].uv	= va.uv0[i];
	}
	vb->Unmap();
}

void CopyPosNormUV0(Graphics::VertexBuffer *vb, const Graphics::VertexArray &va)
{
	PosNormUVVert* vtxPtr = vb->Map<PosNormUVVert>(Graphics::BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(PosNormUVVert));
	for(Uint32 i=0 ; i<va.GetNumVerts() ; i++)
	{
		vtxPtr[i].pos	= va.position[i];
		vtxPtr[i].norm	= va.normal[i];
		vtxPtr[i].uv	= va.uv0[i];
	}
	vb->Unmap();
}

// copies the contents of the VertexArray into the buffer
bool VertexBuffer::Populate(const VertexArray &va)
{
	assert(va.GetNumVerts()>0);
	bool result = false;
	const Graphics::AttributeSet as = va.GetAttributeSet();
	switch( as ) {
	case Graphics::ATTRIB_POSITION:														CopyPos(this, va);			result = true;	break;
	case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE:							CopyPosCol(this, va);		result = true;	break;
	case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0:								CopyPosUV0(this, va);		result = true;	break;
	case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0:	CopyPosColUV0(this, va);	result = true;	break;
	case Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0:	CopyPosNormUV0(this, va);	result = true;	break;
	}
	return result;
}

void VertexBuffer::Bind() {
	glBindVertexArray(m_vao);
}

void VertexBuffer::Release() {
	glBindVertexArray(0);
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

} //namespace OGL
} //namespace Graphics
