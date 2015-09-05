// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

in vec3 varyingEyepos;
in vec3 varyingNormal;

uniform Scene scene;
uniform Material material;

out vec4 frag_color;

void main(void)
{
	vec4 color = material.diffuse;
	
	vec3 eyenorm = normalize(-varyingEyepos);

	float fresnel = 1.0 - abs(dot(eyenorm, varyingNormal)); // Calculate fresnel.
	fresnel = pow(fresnel, 10.0);
	fresnel += 0.05 * (1.0 - fresnel);
	color.a = color.a * clamp(fresnel * 0.5, 0.0, 1.0);
	frag_color = color;

	SetFragDepth();
}
