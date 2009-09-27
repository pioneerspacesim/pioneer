uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;

void main(void)
{
	vec4 eyepos = gl_ModelViewMatrix * gl_Vertex;
	gl_TexCoord[5] = eyepos;
	vec4 clipPos = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[6] = clipPos;
	gl_Position = clipPos;
}
