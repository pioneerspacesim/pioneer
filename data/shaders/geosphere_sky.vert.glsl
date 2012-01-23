uniform vec3 geosphereCenter;
uniform float geosphereRadius;

uniform int occultedLight;
uniform vec3 occultCentre;
uniform float srad;
uniform float lrad;
uniform float maxOcclusion;
uniform vec4 lightDiscRadii;

varying vec4 varyingEyepos;

void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
	varyingEyepos = gl_ModelViewMatrix * gl_Vertex;
}
