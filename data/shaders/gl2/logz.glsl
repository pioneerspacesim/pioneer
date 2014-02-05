// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// See http://www.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&reply_id=3513134
#ifdef FRAGMENT_SHADER
uniform float invLogZfarPlus1;
#endif
varying float varLogDepth;

#ifdef VERTEX_SHADER
vec4 logarithmicTransform()
{
	vec4 vertexPosClip = gl_ModelViewProjectionMatrix * gl_Vertex;
	varLogDepth = vertexPosClip.z;
	return vertexPosClip;
}
#else
void SetFragDepth()
{
	gl_FragDepth = gl_DepthRange.near + (gl_DepthRange.far * log(varLogDepth + 1.0) * invLogZfarPlus1);
}
#endif
