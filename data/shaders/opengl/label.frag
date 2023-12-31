// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// #extension GL_ARB_gpu_shader5 : enable

// Partially copied from multi.frag, should probably have some sort of
// generic framework for surface shaders

#include "attributes.glsl"
#include "lib.glsl"

#ifdef TEXTURE0
in vec2 texCoord0;
#endif

#if NUM_LIGHTS > 0
in vec3 eyePos;
in vec3 normal;
#endif

#ifdef TEXTURE0
uniform sampler2D texture0; //diffuse
#endif

uniform vec4 lightIntensity;

out vec4 frag_color;

Surface initSurface()
{
	Surface surf;
	surf.color = material.diffuse;
	surf.specular = material.specular.xyz;
	surf.shininess = material.shininess;
	surf.emissive = material.emission.xyz;
	surf.normal = vec3(0, 0, 1);
	surf.ambientOcclusion = 1.0;
	return surf;
}

void getSurface(inout Surface surf)
{
#ifdef TEXTURE0
	surf.color *= texture(texture0, texCoord0);
#endif

	// signed distance to the 'edge' of the font - negative is outside the edge, positive is inside the edge
	float dist = surf.color.a - 0.5;
	// calculate the derivative of distance around 0 to create smooth antialiasing
	float deltaDist = fwidth(dist);
	surf.color.a = smoothstep(-deltaDist, deltaDist, dist);

#if (NUM_LIGHTS > 0)
//directional lighting
	surf.normal = normalize(normal);
#endif
}

void main(void)
{
	// initialize here to prevent warnings about possibly-unused variables
	Surface surface = initSurface();

	getSurface(surface);

#ifdef ALPHA_TEST
	if (surface.color.a == 0.0)
		discard;
#endif

	//directional lighting
#if (NUM_LIGHTS > 0)
	//ambient only make sense with lighting
	vec3 diffuse = scene.ambient.xyz * surface.color.xyz;
	vec3 specular = vec3(0.0);
	float intensity[4] = float[](
		lightIntensity.x,
		lightIntensity.y,
		lightIntensity.z,
		lightIntensity.w
	);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		BlinnPhongDirectionalLight(uLight[i], intensity[i], surface, eyePos, diffuse, specular);
	}

	vec3 final_color = diffuse * surface.ambientOcclusion + surface.emissive + specular;
	frag_color = vec4(final_color, surface.color.w);
#else
	frag_color = surface.color;
#endif
}
