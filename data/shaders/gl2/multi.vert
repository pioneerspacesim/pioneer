// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef TEXTURE0
varying vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif
#if (NUM_LIGHTS > 0)
varying vec3 eyePos;
varying vec3 normal;
	#ifdef HEAT_COLOURING
		uniform mat3 heatingMatrix;
		uniform vec3 heatingNormal; // normalised
		varying vec3 heatingDir;
	#endif // HEAT_COLOURING
#endif

void main(void)
{
	gl_Position = logarithmicTransform();
#ifdef VERTEXCOLOR
	vertexColor = a_color;
#endif
#ifdef TEXTURE0
	texCoord0 = a_uv0.xy;
#endif
#if (NUM_LIGHTS > 0)
	eyePos = vec3(gl_ModelViewMatrix * a_vertex);
	normal = normalize(gl_NormalMatrix * a_normal);
#ifdef HEAT_COLOURING
	heatingDir = normalize(heatingMatrix * heatingNormal);
#endif
#endif
}
