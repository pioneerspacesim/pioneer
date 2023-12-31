// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include "SDL_stdinc.h"

namespace Graphics {

	//Vertex attribute semantic
	enum VertexAttrib : uint8_t {
		ATTRIB_NONE = 0,
		ATTRIB_POSITION = (1u << 0),
		ATTRIB_NORMAL = (1u << 1),
		ATTRIB_DIFFUSE = (1u << 2),
		ATTRIB_UV0 = (1u << 3),
		//ATTRIB_UV1       = (1u << 4),
		ATTRIB_TANGENT = (1u << 5),
		//ATTRIB_BITANGENT = (1u << 6)
		ATTRIB_POSITION2D = (1u << 7),
		//etc.
	};

	// typedef Uint32 AttributeSet;

	struct AttributeSet {
		AttributeSet() :
			m_attr(0) {}
		AttributeSet(VertexAttrib attr) :
			m_attr(attr) {}
		AttributeSet(uint32_t attr) :
			m_attr(attr) {}
		AttributeSet &operator=(uint32_t rhs)
		{
			m_attr = rhs;
			return *this;
		}
		operator uint32_t() const { return m_attr; }

		inline bool HasAttrib(uint32_t attr) const { return (m_attr & attr) == attr; }

	private:
		uint32_t m_attr;
	};

	enum VertexAttribFormat : uint8_t {
		ATTRIB_FORMAT_NONE = 0,
		ATTRIB_FORMAT_FLOAT2,
		ATTRIB_FORMAT_FLOAT3,
		ATTRIB_FORMAT_FLOAT4,
		ATTRIB_FORMAT_UBYTE4
	};

	enum ConstantDataFormat : uint8_t {
		DATA_FORMAT_NONE = 0,
		DATA_FORMAT_INT,
		DATA_FORMAT_FLOAT,
		DATA_FORMAT_FLOAT3,
		DATA_FORMAT_FLOAT4,
		DATA_FORMAT_MAT3,
		DATA_FORMAT_MAT4
	};

	enum BufferUsage {
		BUFFER_USAGE_STATIC,
		BUFFER_USAGE_DYNAMIC
	};

	enum BufferMapMode {
		BUFFER_MAP_NONE,
		BUFFER_MAP_WRITE,
		BUFFER_MAP_READ
	};

	enum IndexBufferSize {
		INDEX_BUFFER_16BIT,
		INDEX_BUFFER_32BIT
	};

	// clang-format off
	enum PrimitiveType {
		POINTS = 0,		//GL_POINTS,
		LINE_SINGLE,	//GL_LINES,				//draw one line per two vertices
		LINE_LOOP,		//GL_LINE_LOOP,			//connect vertices,  connect start & end
		LINE_STRIP,		//GL_LINE_STRIP,		//connect vertices
		TRIANGLES,		//GL_TRIANGLES,
		TRIANGLE_STRIP,	//GL_TRIANGLE_STRIP,
		TRIANGLE_FAN,	//GL_TRIANGLE_FAN
	};
	// clang-format on

	enum BlendMode : uint32_t {
		BLEND_SOLID,
		BLEND_ADDITIVE,
		BLEND_ALPHA,
		BLEND_ALPHA_ONE, //"additive alpha"
		BLEND_ALPHA_PREMULT,
		BLEND_SET_ALPHA, // copy alpha channel
		BLEND_DEST_ALPHA // XXX maybe crappy name
	};

	enum FaceCullMode : uint32_t {
		CULL_BACK,
		CULL_FRONT,
		CULL_NONE
	};

} // namespace Graphics

#endif // GRAPHICS_TYPES_H
