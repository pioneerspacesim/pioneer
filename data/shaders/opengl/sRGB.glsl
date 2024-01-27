// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#define GAMMA_FACTOR 2.2

vec4 toLinear(in vec4 val)
{
	return pow(val, vec4(vec3(GAMMA_FACTOR), 1.0));
}

vec3 toLinear(in vec3 val)
{
	return pow(val, vec3(GAMMA_FACTOR));
}

vec4 toSRGB(in vec4 val)
{
	return pow(val, vec4(vec3(1.0 / GAMMA_FACTOR), 1.0));
}

vec3 toSRGB(in vec3 val)
{
	return pow(val, vec3(1.0 / GAMMA_FACTOR));
}
