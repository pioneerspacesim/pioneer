varying vec3 norm;
varying vec2 texCoord;
varying vec3 ecPosition3;

void main(void)
{
	gl_Position = logarithmicTransform();

	norm = gl_NormalMatrix * gl_Normal;
	texCoord = gl_MultiTexCoord0.xy;
	ecPosition3 = (gl_ModelViewMatrix * gl_Vertex).xyz;
}
