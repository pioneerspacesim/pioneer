varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec3 varyingTexCoord0;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = vec3(gl_ModelViewMatrix * a_vertex);
	varyingNormal = gl_NormalMatrix * a_normal;
	varyingTexCoord0 = a_normal.xyz;
}
