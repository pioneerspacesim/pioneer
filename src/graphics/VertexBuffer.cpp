// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/VertexBuffer.h"
#include "graphics/Types.h"
#include "core/macros.h"

#include <algorithm>
#include <cstring>

namespace Graphics {

	struct AttribData {
		VertexAttrib semantic;
		VertexAttribFormat format;
		uint8_t location;
	};

	static const AttribData s_attribs[] = {
		{ ATTRIB_POSITION,   ATTRIB_FORMAT_FLOAT3, 0 },
		{ ATTRIB_POSITION2D, ATTRIB_FORMAT_FLOAT2, 0 },
		{ ATTRIB_NORMAL,     ATTRIB_FORMAT_FLOAT3, 1 },
		{ ATTRIB_DIFFUSE,    ATTRIB_FORMAT_UBYTE4, 2 },
		{ ATTRIB_UV0,        ATTRIB_FORMAT_FLOAT2, 3 },
		{ ATTRIB_TANGENT,    ATTRIB_FORMAT_FLOAT3, 5 },
	};

	static uint32_t GetAttribSize(VertexAttribFormat f)
	{
		switch (f) {
		case ATTRIB_FORMAT_NONE:
			return 0;
		case ATTRIB_FORMAT_FLOAT:
			return 4;
		case ATTRIB_FORMAT_FLOAT2:
			return 8;
		case ATTRIB_FORMAT_FLOAT3:
			return 12;
		case ATTRIB_FORMAT_FLOAT4:
			return 16;
		case ATTRIB_FORMAT_UBYTE4:
			return 4;
		case ATTRIB_FORMAT_MAT3:
			return 36;
		case ATTRIB_FORMAT_MAT3x4:
			return 48;
		case ATTRIB_FORMAT_MAT4x4:
			return 64;
		default:
			return 0;
		}
	}

	size_t GetAttributeIndexInSet(AttributeSet set, VertexAttrib attrib)
	{
		if (!(set & attrib))
			return SIZE_MAX;

		size_t index = 0;

		for (size_t i = 0; i < COUNTOF(s_attribs); i++) {
			const AttribData &data = s_attribs[i];

			if (data.semantic == attrib)
				break;

			if (set & data.semantic)
				index++;
		}

		return index;
	}

	VertexFormatDesc::VertexFormatDesc()
	{
		std::memset(attribs, 0, sizeof(VertexAttribDesc) * MAX_ATTRIBS);
		std::memset(bindings, 0, sizeof(VertexBindingDesc) * MAX_BINDINGS);
	}

	VertexFormatDesc VertexFormatDesc::FromAttribSet(AttributeSet set)
	{
		// Early-out assertion, maybe not useful?
		assert(set.HasAttrib(ATTRIB_POSITION) || set.HasAttrib(ATTRIB_POSITION2D));

		VertexFormatDesc vbd = {};
		size_t attribIdx = 0;
		uint16_t offset = 0;

		// Create and fill the list of vertex attribute descriptors
		// By walking the s_attribs array, we guarantee a consistent order of vertex attribute descriptors,
		// allowing the list to be queried with an index returned from GetAttributeIndexInSet
		for (size_t i = 0; i < COUNTOF(s_attribs); i++) {
			const AttribData &attrib = s_attribs[i];

			if (set.HasAttrib(attrib.semantic)) {
				vbd.attribs[attribIdx++] = { attrib.format, attrib.location, 0, offset };
				offset += GetAttribSize(attrib.format);
			}
		}

		// Create the default single binding for the given AttributeSet
		vbd.bindings[0] = { offset, true, VertexAttribRate::ATTRIB_RATE_NORMAL };

		return vbd;
	}

	size_t VertexFormatDesc::GetNumAttribs() const
	{
		for (size_t i = 0; i < MAX_ATTRIBS; i++) {
			if (attribs[i].format == ATTRIB_FORMAT_NONE)
				return i;
		}

		return MAX_ATTRIBS;
	}

	size_t VertexFormatDesc::GetNumBindings() const
	{
		for (size_t i = 0; i < MAX_BINDINGS; i++) {
			if (!bindings[i].enabled)
				return i;
		}

		return MAX_BINDINGS;
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	bool VertexBuffer::SetVertexCount(Uint32 v)
	{
		if (v <= m_capacity) {
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
