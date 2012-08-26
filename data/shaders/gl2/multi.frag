#ifdef TEXTURE0
uniform sampler2D texture0;
varying vec2 texCoord0;
#endif
#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif
#if (NUM_LIGHTS > 0)
varying vec3 eyePos;
varying vec3 normal;
#endif

struct Material {
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
uniform Material material;

#if (NUM_LIGHTS > 0)
//ambient, diffuse, specular
//would be a good idead to make specular optional
void ads(in int lightNum, in vec3 pos, in vec3 n, inout vec4 light, inout vec4 specular)
{
	vec3 s = normalize(vec3(gl_LightSource[lightNum].position)); //directional light
	vec3 v = normalize(vec3(-pos));
	vec3 h = normalize(v + s);
	light += gl_LightSource[lightNum].diffuse * material.diffuse * max(dot(s, n), 0.0);
#ifdef MAP_SPECULAR
	specular += texture2D(texture1, uv0) * material.specular * gl_LightSource[lightNum].diffuse * pow(max(dot(h, n), 0.0), material.shininess);
#else
	specular += material.specular * gl_LightSource[lightNum].diffuse * pow(max(dot(h, n), 0.0), material.shininess);
#endif
}
#endif

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

	//lighting - only one light right now
#if (NUM_LIGHTS > 0)
	vec4 light = vec4(0.0);
	vec4 specular = vec4(0.0);
	ads(0, eyePos, normal, light, specular);
#endif

#if (NUM_LIGHTS > 0)
	gl_FragColor = color * light + specular;
#else
	gl_FragColor = color;
#endif
	SetFragDepth();
}
