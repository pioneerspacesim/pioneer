// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

out vec4 varyingEyepos;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = uViewMatrix * a_vertex;
}

