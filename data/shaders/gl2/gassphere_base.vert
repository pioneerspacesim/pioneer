varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec3 varyingTexCoord0;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;
	varyingTexCoord0 = gl_Normal.xyz;
}
