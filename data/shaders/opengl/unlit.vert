// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// #extension GL_ARB_gpu_shader5 : enable

#include "attributes.glsl"
#include "lib.glsl"

#ifdef TEXTURE0
out vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
out vec4 vertexColor;
#endif

void main(void)
{
	gl_Position = matrixTransform();
#ifdef VERTEXCOLOR
	vertexColor = a_color;
#endif
#ifdef TEXTURE0
	texCoord0 = a_uv0.xy;
#endif
}
