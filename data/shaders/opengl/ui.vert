// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

out vec2 texCoord0;

out vec4 vertexColor;

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = a_color;
	texCoord0 = a_uv0.xy;
}
