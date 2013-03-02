#ifdef TEXTURE0
uniform sampler2D texture0; //diffuse
uniform sampler2D texture1; //specular
uniform sampler2D texture2; //glow
uniform sampler2D texture3; //pattern
uniform sampler2D texture4; //color
varying vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif

uniform Scene scene;
uniform Material material;
uniform float shieldStrength;

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
//patterns - simple lookup
#ifdef MAP_COLOR
	float pat = texture2D(texture3, texCoord0).r;
	vec4 mapColor = texture2D(texture4, vec2(pat, 0.0));
	color *= mapColor;
#endif

	color.a = color.a * shieldStrength;

	gl_FragColor = color;

	SetFragDepth();
}
