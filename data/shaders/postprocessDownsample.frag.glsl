#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect fboTex;
uniform float avgLum;
// downscale stage of making the bloom texture

void main(void)
{
	const vec3 getlum = vec3(0.299, 0.587, 0.114);
	vec3 col = vec3(0.0);
	vec2 p = 2.0*gl_FragCoord.xy;
	vec3 c;
	const float minLumToBloom = max(16.0*avgLum, 1.0);
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y)));
	if (dot(getlum,c) > minLumToBloom) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y)));
	if (dot(getlum,c) > minLumToBloom) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y+1.0)));
	if (dot(getlum,c) > minLumToBloom) col += c;
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y+1.0)));
	if (dot(getlum,c) > minLumToBloom) col += c;
	col *= 0.25;
	gl_FragColor = vec4(clamp(col.r,0.0,60000.0));
}
