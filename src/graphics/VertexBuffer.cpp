// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/VertexBuffer.h"
#include "graphics/Types.h"
#include "core/macros.h"

#include "lz4/xxhash.h"

#include <algorithm>
#include <bitset>
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

	size_t GetNumActiveAttribsInSet(AttributeSet set)
	{
		size_t active = 0;
		for (size_t i = 0; i < COUNTOF(s_attribs); i++)
			if (set & s_attribs[i].semantic)
				active++;

		return active;
	}

	void GetActiveAttribsInSet(AttributeSet set, VertexAttrib *attribs, size_t numAttribs)
	{
		size_t out_idx = 0;
		for (size_t i = 0; i < COUNTOF(s_attribs) && out_idx < numAttribs; i++) {
			if (set & s_attribs[i].semantic)
				attribs[out_idx++] = s_attribs[i].semantic;
		}
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

	InvalidVertexFormatReason VertexFormatDesc::ValidateDesc() const
	{
		std::bitset<256> locations {};

		size_t numBindings = GetNumBindings();

		for (size_t i = 0; i < MAX_ATTRIBS && attribs[i].format != ATTRIB_FORMAT_NONE; i++) {
			size_t numLocations = 1;

			if (attribs[i].format == ATTRIB_FORMAT_MAT3)
				numLocations = 3;
			if (attribs[i].format == ATTRIB_FORMAT_MAT3x4)
				numLocations = 4;
			if (attribs[i].format == ATTRIB_FORMAT_MAT4x4)
				numLocations = 4;

			for (size_t loc = 0; loc < numLocations; loc++) {
				if (locations.test(attribs[i].location + loc)) {
					return InvalidVertexFormatReason::LocationOverlap;
				}

				locations.set(attribs[i].location + loc, true);
			}

			if (attribs[i].binding >= numBindings)
				return InvalidVertexFormatReason::InvalidBinding;
		}

		return InvalidVertexFormatReason::OK;
	}

	uint64_t VertexFormatDesc::Hash() const
	{
		// Because we defined VertexFormatDesc with #pragma pack(1), we can be
		// confident there are no padding bytes anywhere in the struct and can
		// hash the entire memory block.
		return XXH64(this, sizeof(VertexFormatDesc), 0);
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

} // namespace Graphics
