varying vec3 ecPos;
varying vec3 norm;
varying vec2 uv0;
uniform sampler2D texture0; //diffuse
uniform sampler2D texture1; //specular
uniform sampler2D texture2; //glow
uniform sampler2D texture3; //pattern
uniform sampler2D texture4; //color

struct Scene {
	vec4 ambient; //scene global ambient
};

struct Material {
	vec4 diffuse;
	vec4 emissive;
	vec4 specular;
	int shininess;
};

uniform Scene scene;
uniform Material material;

vec4 light;
vec4 specular;

void ads(int lightNum, vec3 pos, vec3 n)
{
	//vec3 s = normalize(vec3(gl_LightSource[lightNum].position) - pos);
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

void main(void)
{
	light = scene.ambient +
#ifdef MAP_EMISSIVE
		texture2D(texture2, uv0) * material.emissive; //glow map
#else
		material.emissive; //just emissive parameter
#endif

	specular = vec4(0.0);
	ads(0, ecPos, norm);
	ads(1, ecPos, norm);
	light.a = 1.0;

#ifdef MAP_COLOR
	float pat = texture2D(texture3, uv0).r;
	vec4 mapColor = texture2D(texture4, vec2(pat, 0.0));
	vec4 base = texture2D(texture0, uv0) * mapColor;
#else
	vec4 base = texture2D(texture0, uv0);
#endif

#ifdef ALPHA_TEST
	if (base.a < 0.5)
		discard;
#endif

#ifdef TEXTURE0
	gl_FragColor = base * light + specular;
#else
	gl_FragColor = light + specular;
#endif
	SetFragDepth();
}
