// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

uniform sampler2D texture0; //diffuse
varying vec2 texCoord0;

varying vec4 vertexColor;

uniform Scene scene;
uniform Material material;

void main(void)
{
	gl_FragColor = vertexColor * texture2D(texture0, texCoord0).x;
	
	SetFragDepth();
}
