// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec2 texCoord0;
out vec4 texCoord1;

void main(void)
{
	gl_Position = matrixTransform();

	texCoord0 = a_uv0.xy;
	texCoord1 = a_vertex;
}
