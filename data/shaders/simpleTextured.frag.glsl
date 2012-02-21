varying vec4 color;
varying vec2 texCoord0;
uniform sampler2D texture0;

void main(void)
{
	gl_FragColor = texture2D(texture0, texCoord0) * color;
}
