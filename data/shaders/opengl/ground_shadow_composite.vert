// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"

out vec2 texCoord0;

void main(void)
{
	gl_Position = uViewProjectionMatrix * a_vertex;
	texCoord0 = a_uv0.xy;
}
