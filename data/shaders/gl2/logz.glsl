// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// See http://www.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&reply_id=3513134
#ifdef FRAGMENT_SHADER
uniform float invLogZfarPlus1;
#endif
varying float varLogDepth;

#ifdef VERTEX_SHADER
vec4 logarithmicTransform()
{
#ifdef USE_INSTANCING
	//vec4 vertexPosClip = uProjectionMatrix * uViewMatrix * a_transform * a_vertex;
	vec4 vertexPosClip = uViewProjectionMatrix * a_transform * a_vertex;
#else
	vec4 vertexPosClip = uViewProjectionMatrix * a_vertex;
#endif
	varLogDepth = vertexPosClip.z;
	return vertexPosClip;
}
#elif defined(FRAGMENT_SHADER)
void SetFragDepth()
{
	gl_FragDepth = gl_DepthRange.near + (gl_DepthRange.far * log(varLogDepth + 1.0) * invLogZfarPlus1);
}
#endif
