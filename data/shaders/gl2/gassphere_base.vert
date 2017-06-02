// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec3 varyingTexCoord0;

uniform vec3 geosphereCenter;
uniform float geosphereRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = normalize(uNormalMatrix * a_normal);
	varyingTexCoord0 = a_normal.xyz;
}
