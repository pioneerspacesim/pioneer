// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "graphics/VertexBuffer.h"
#include "graphics/Types.h"

#include <algorithm>

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

	VertexBufferDesc::VertexBufferDesc() :
		numVertices(0),
		stride(0),
		usage(BUFFER_USAGE_STATIC)
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

	VertexBufferDesc VertexBufferDesc::FromAttribSet(AttributeSet set)
	{
		// Create and fill the list of vertex attribute descriptors
		VertexBufferDesc vbd;
		Uint32 attribIdx = 0;
		assert(set.HasAttrib(ATTRIB_POSITION));
		vbd.attrib[attribIdx].semantic = ATTRIB_POSITION;
		vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
		++attribIdx;

		if (set.HasAttrib(ATTRIB_NORMAL)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_NORMAL;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
			++attribIdx;
		}
		if (set.HasAttrib(ATTRIB_DIFFUSE)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_DIFFUSE;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_UBYTE4;
			++attribIdx;
		}
		if (set.HasAttrib(ATTRIB_UV0)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_UV0;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT2;
			++attribIdx;
		}
		if (set.HasAttrib(ATTRIB_TANGENT)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_TANGENT;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
			++attribIdx;
		}

		vbd.CalculateOffsets();
		return vbd;
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

	void VertexBufferDesc::CalculateOffsets()
	{
		//update offsets in desc
		// at the end of the loop, offs will be the stride of the buffer
		Uint32 offs = 0;
		for (Uint32 i = 0; i < MAX_ATTRIBS; i++) {
			if (attrib[i].offset)
				offs = attrib[i].offset;
			else
				attrib[i].offset = offs;
			offs += GetAttribSize(attrib[i].format);
		}

		//update stride in desc (respecting offsets)
		if (stride == 0) stride = offs;
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	bool VertexBuffer::SetVertexCount(Uint32 v)
	{
		if (v <= m_desc.numVertices) {
			m_size = v;
			return true;
		}
		return false;
	}

	// ------------------------------------------------------------
	IndexBuffer::IndexBuffer(Uint32 size, BufferUsage usage, IndexBufferSize elem) :
		Mappable(size),
		m_indexCount(size),
		m_elemSize(elem),
		m_usage(usage)
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

	// ------------------------------------------------------------
	InstanceBuffer::InstanceBuffer(Uint32 size, BufferUsage usage) :
		Mappable(size),
		m_usage(usage)
	{
	}

	InstanceBuffer::~InstanceBuffer()
	{
	}

	void InstanceBuffer::SetInstanceCount(const Uint32 ic)
	{
		assert(ic <= GetSize());
		m_instanceCount = std::min(ic, GetSize());
	}

} // namespace Graphics
