varying vec3 norm;
varying vec2 texCoord;

void main(void)
{
	gl_Position = logarithmicTransform();

	norm = gl_NormalMatrix * gl_Normal;
	texCoord = gl_MultiTexCoord0.xy;
}
