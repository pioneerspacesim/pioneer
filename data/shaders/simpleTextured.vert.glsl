varying vec4 color;
varying vec2 texCoord0;

void main(void)
{
	texCoord0 = gl_MultiTexCoord0.st;
	color = gl_Color;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
