varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;
varying vec4 varyingemission;

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = gl_Color;
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;

#ifdef TERRAIN_WITH_LAVA
	varyingemission=gl_FrontMaterial.emission;
	//Glow lava terrains
	if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
		varyingemission=3.0*vertexColor;
		varyingemission *= (vertexColor.r+vertexColor.g+vertexColor.b);

	}
#endif

}
