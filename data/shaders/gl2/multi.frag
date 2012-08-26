#ifdef TEXTURE0
uniform sampler2D texture0;
varying vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif

struct Material {
	vec4 diffuse;
};
uniform Material material;

void main(void)
{
#ifdef VERTEXCOLOR
	vec4 color = vertexColor;
#else
	vec4 color = material.diffuse;
#endif
#ifdef TEXTURE0
	color *= texture2D(texture0, texCoord0);
#endif

#ifdef ALPHA_TEST
	if (color.a < 0.5)
		discard;
#endif
	gl_FragColor = color;
	SetFragDepth();
}
