// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

varying vec4 color;
varying vec2 uv;
varying vec3 lightDir;

void main(void)
{
	gl_Position = logarithmicTransform();
	color = a_color;
	uv = a_uv0.xy * 2.0 - 1.0; //recenter to -1,1 range
	lightDir = normalize(-a_vertex.xyz);
}
