// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef FRAGMENT_SHADER

struct Surface {
	vec4 color;
	vec3 specular;
	float shininess;
	vec3 normal;
	vec3 emissive;
	float ambientOcclusion;
};

// Currently used by: hopefully everything
// Evaluates a standard blinn-phong diffuse+specular, with the addition of a
// light intensity term to scale the lighting contribution based on (pre-calculated)
// global occlusion
void BlinnPhongDirectionalLight(in Light light, in float intensity, in Surface surf, in vec3 fragPos, inout vec3 diffuse, inout vec3 specular)
{
	// This code calculates directional lights
	vec3 L = normalize(light.position.xyz); // surface->light vector
	vec3 V = normalize(-fragPos); // surface->eye vector
	vec3 H = normalize(L + V); // halfway vector
	diffuse += surf.color.xyz * light.diffuse.xyz * intensity * max(dot(L, surf.normal), 0.0);
	specular += surf.specular * light.specular.xyz * intensity * pow(max(dot(H, surf.normal), 0.0), surf.shininess);
}

//Currently used by: planet ring shader, geosphere shaders
float findSphereEyeRayEntryDistance(in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float entryDist = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0.0) {
			entryDist = max(i1, 0.0);
		}
	}
	return entryDist;
}

// Used by: geosphere shaders
// Calculate length*density product of a line through the atmosphere
// a - start coord (normalized relative to atmosphere radius)
// b - end coord " "
// centerDensity - atmospheric density at centre of sphere
// length - real length of line in meters
float AtmosLengthDensityProduct(vec3 a, vec3 b, float surfaceDensity, float len, float invScaleHeight)
{
	/* 4 samples */
	float ldprod = 0.0;
	vec3 dir = b-a;
	ldprod = surfaceDensity * (
			exp(-invScaleHeight*(length(a)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.2*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.4*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.6*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.8*dir)-1.0)) +
			exp(-invScaleHeight*max(length(b)-1.0, 0.0)));
	ldprod *= len;
	return ldprod;
}
#endif
