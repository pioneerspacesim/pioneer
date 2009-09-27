uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;

void main(void)
{
	gl_Position = logarithmicTransform();
	vec4 eyepos = gl_ModelViewMatrix * gl_Vertex;
	gl_TexCoord[6] = eyepos;
}
