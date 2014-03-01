// Copyright � 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H
#include "libs.h"

namespace Graphics {

//Vertex attribute semantic
enum VertexAttrib {
	ATTRIB_NONE      = 0,
	ATTRIB_POSITION  = (1u << 0),
	ATTRIB_NORMAL    = (1u << 1),
	ATTRIB_DIFFUSE   = (1u << 2),
	ATTRIB_UV0       = (1u << 3),
	//ATTRIB_UV1       = (1u << 4),
	//ATTRIB_TANGENT   = (1u << 5),
	//ATTRIB_BITANGENT = (1u << 6)
	//etc.
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
	BUFFER_MAP_NONE,
	BUFFER_MAP_WRITE,
	BUFFER_MAP_READ
};

enum LineType {
	LINE_SINGLE = GL_LINES, //draw one line per two vertices
	LINE_STRIP = GL_LINE_STRIP,  //connect vertices
	LINE_LOOP = GL_LINE_LOOP    //connect vertices,  connect start & end
};

enum PrimitiveType {
	TRIANGLES = GL_TRIANGLES,
	TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
	TRIANGLE_FAN = GL_TRIANGLE_FAN,
	POINTS = GL_POINTS
};

enum BlendMode {
	BLEND_SOLID,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_ALPHA_ONE, //"additive alpha"
	BLEND_ALPHA_PREMULT,
	BLEND_SET_ALPHA, // copy alpha channel
	BLEND_DEST_ALPHA // XXX maybe crappy name
};

enum FaceCullMode {
	CULL_BACK,
	CULL_FRONT,
	CULL_NONE
};

}

#endif // GRAPHICS_TYPES_H
