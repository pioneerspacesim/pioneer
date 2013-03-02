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
varying vec3 eyePos;
varying vec3 normal;

uniform Scene scene;
uniform Material material;
uniform float shieldStrength;

//ambient, diffuse, specular
//would be a good idead to make specular optional
void ads(in vec3 pos, in vec3 n, inout vec4 light, inout vec4 specular)
{
	vec3 s = normalize(vec3(n)); //directional light
	vec3 v = normalize(vec3(-pos));
	vec3 h = normalize(v + s);
	light += material.diffuse * max(dot(s, n), 0.0);

	specular += texture2D(texture1, texCoord0) * material.specular *  pow(max(dot(h, n), 0.0), material.shininess);

	specular.a = 0.0;
	light.a = 1.0;
}

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

	
//ambient and emissive only make sense with lighting
#ifdef MAP_EMISSIVE
	vec4 light = scene.ambient + texture2D(texture2, texCoord0); //glow map
#else
	vec4 light = scene.ambient + material.emission; //just emissive parameter
#endif
	vec4 specular = vec4(0.0);
	ads(eyePos, normal, light, specular);
	
	color.a = color.a * shieldStrength;

	gl_FragColor = color * light + specular;

	SetFragDepth();
}
