varying vec3 ecPos;
varying vec3 norm;
varying vec2 uv0;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emissive;
uniform int shininess;
uniform vec4 ambient; //scene global ambient
uniform sampler2D texture0; //diffuse
uniform sampler2D texture1; //specular
uniform sampler2D texture2; //glow
uniform sampler2D texture3;

vec3 ads(int light, vec3 pos, vec3 norm)
{
	vec3 Kd = diffuse.rgb;
	vec3 Ka = ambient.rgb;
#if MAP_SPECULAR
	vec3 Ks = texture2D(texture1, uv0 * 4.0).rgb * specular;
#else
	vec3 Ks = specular;
#endif
	vec3 n = normalize(norm);
	vec3 s = normalize(vec3(gl_LightSource[light].position) - pos);
	vec3 v = normalize(vec3(-pos));
	vec3 h = normalize(v + s);
	vec3 I = gl_LightSource[light].diffuse.rgb;
	return I * (Ka +
		Kd * max(dot(s, n), 0.0) +
		Ks * pow(max(dot(h, n), 0.0), shininess));
}

void main(void)
{
	vec4 light = vec4(ads(0, ecPos, norm), 1.0) +
#if MAP_EMISSIVE
		texture2D(texture2, uv0); //glow map
#else
		emissive; //just emissive parameter
#endif
#if TEXTURE0
	gl_FragColor = texture2D(texture0, uv0) * light;
#else
	gl_FragColor = light;
#endif
	SetFragDepth(gl_TexCoord[6].z);
}
