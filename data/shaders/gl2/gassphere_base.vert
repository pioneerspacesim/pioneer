out vec3 varyingEyepos;
out vec3 varyingNormal;
out vec3 varyingTexCoord0;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	varyingEyepos = vec3(uViewMatrix * a_vertex);
	varyingNormal = vec3(uNormalMatrix * vec4(a_normal, 1.0));
	varyingTexCoord0 = a_normal.xyz;
}
