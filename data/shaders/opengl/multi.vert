// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// #extension GL_ARB_gpu_shader5 : enable

#ifdef TEXTURE0
out vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
out vec4 vertexColor;
#endif
#if (NUM_LIGHTS > 0)
out vec3 eyePos;
out vec3 normal;
	#ifdef HEAT_COLOURING
		uniform mat3 heatingMatrix;
		uniform vec3 heatingNormal; // normalised
		out vec3 heatingDir;
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
#ifdef USE_INSTANCING
	eyePos = vec3(uViewMatrix * (a_transform * a_vertex));
	normal = normalize(uNormalMatrix * (mat3(a_transform) * a_normal));
#else
	eyePos = vec3(uViewMatrix * a_vertex);
	normal = normalize(uNormalMatrix * a_normal);
#endif
	
	//mat3 mn = mat3(transpose(inverse(uViewMatrix)));
	//normal = normalize(mn * a_normal);
#ifdef HEAT_COLOURING
	heatingDir = normalize(heatingMatrix * heatingNormal);
#endif
#endif
}
