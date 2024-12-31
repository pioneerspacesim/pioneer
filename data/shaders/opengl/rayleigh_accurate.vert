// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "lib.glsl"
#include "basesphere_uniforms.glsl"

out vec4 varyingEyepos;

void main(void)
{
	gl_Position = matrixTransform();
	varyingEyepos = uViewMatrix * a_vertex;
}
