// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H
#include "libs.h"

namespace Graphics {

//allowed minimum of GL_MAX_VERTEX_ATTRIBS is 8 on ES2
enum VertexAttrib {
	ATTRIB_NONE      = 0,
	ATTRIB_POSITION  = (1u << 0),
	ATTRIB_NORMAL    = (1u << 1),
	ATTRIB_DIFFUSE   = (1u << 2),
	ATTRIB_UV0       = (1u << 3),
	//ATTRIB_UV1       = (1u << 4),
	//ATTRIB_TANGENT   = (1u << 5),
	//ATTRIB_BITANGENT = (1u << 6),
	//ATTRIB_CUSTOM?
};

enum VertexAttribFormat {
	ATTRIB_FORMAT_NONE = 0,
	ATTRIB_FORMAT_FLOAT2,
	ATTRIB_FORMAT_FLOAT3,
	ATTRIB_FORMAT_FLOAT4,
	ATTRIB_FORMAT_UBYTE4
};

enum BufferUsage {
	BUFFER_USAGE_STATIC,
	BUFFER_USAGE_DYNAMIC
};

enum BufferMapMode {
	BUFFER_MAP_WRITE,
	BUFFER_MAP_READ
};

}

#endif // GRAPHICS_TYPES_H
