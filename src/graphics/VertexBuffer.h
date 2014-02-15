// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_VERTEXBUFFER_H
#define GRAPHICS_VERTEXBUFFER_H

#include "libs.h"
#include "graphics/Types.h"

namespace Graphics {

const Uint32 MAX_ATTRIBS = 8;

struct VertexAttribDesc {
	//position, texcoord, normal etc.
	VertexAttrib semantic;
	//float3, float2 etc.
	VertexAttribFormat format;
	//byte offset of the attribute, if zero this
	//is automatically filled for created buffers
	Uint32 offset;
};

struct VertexBufferDesc {
	VertexBufferDesc();
	//byte size (stride) of one vertex
	Uint32 GetVertexSize() const;
	//byte offset of an attribute
	Uint32 GetOffset(VertexAttrib) const;
	static Uint32 GetAttribSize(VertexAttribFormat);

	VertexAttribDesc attrib[MAX_ATTRIBS];
	Uint32 numVertices;
	BufferUsage usage;
};

class VertexBuffer : public RefCounted {
public:
	virtual ~VertexBuffer();
	const VertexBufferDesc &GetDesc() const { return m_desc; }

	template <typename T> T *Map(BufferMapMode mode) {
		return reinterpret_cast<T*>(MapInternal(mode));
	}
	virtual void Unmap() = 0;

	//vertex count used for rendering.
	//by default the maximum set in description
	Uint32 GetVertexCount() const;
	void SetVertexCount(Uint32);

protected:
	virtual Uint8 *MapInternal(BufferMapMode) = 0;
	VertexBufferDesc m_desc;
	Uint32 m_numVertices;
};

class IndexBuffer : public RefCounted {
public:
	IndexBuffer(Uint32 size);
	virtual ~IndexBuffer();
	virtual Uint16 *Map(BufferMapMode) = 0;
	virtual void Unmap() = 0;

	Uint32 GetSize() const { return m_size; }
	Uint32 GetIndexCount() const { return m_indexCount; }
	void SetIndexCount(Uint32);

protected:
	Uint32 m_size;
	Uint32 m_indexCount;
};

}
#endif // GRAPHICS_VERTEXBUFFER_H
