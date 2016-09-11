// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform Material material;

varying vec4 vertexColor;


void main(void)
{
	// NUM_LIGHTS > 0 -- unlit rendering - stars
	//emission is used to boost colour of stars, which is a bit odd
	gl_FragColor = material.emission + vertexColor;

	SetFragDepth();
}
