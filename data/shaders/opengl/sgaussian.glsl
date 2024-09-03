// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Code adapted from https://therealmjp.github.io/posts/sg-series-part-2-spherical-gaussians-101/

struct SGaussian {
	vec3 Axis;
	vec3 Amplitude;
	float Sharpness;
};

SGaussian CosineLobeSG(in vec3 direction)
{
	SGaussian cosineLobe;
    cosineLobe.Axis = direction;
    cosineLobe.Amplitude = vec3(1.17);
    cosineLobe.Sharpness = 2.133;
	return cosineLobe;
}

vec3 SGInnerProduct(in SGaussian x, in SGaussian y)
{
    float dm = length(x.Sharpness * x.Axis + y.Sharpness * y.Axis);
    vec3 expo = exp(dm - x.Sharpness - y.Sharpness) * x.Amplitude * y.Amplitude;
    float other = 1.0 - exp(-2.0 * dm);
    return (2.0 * PI * expo * other) / dm;
}

/* // example of calculating diffuse irradiance from a spherical gaussian
vec3 SGIrradianceInnerProduct(in SGaussian lightingLobe, in vec3 normal)
{
    SGaussian cosineLobe = CosineLobeSG(normal);
    return max(SGInnerProduct(lightingLobe, cosineLobe), 0.0f);
}
*/
