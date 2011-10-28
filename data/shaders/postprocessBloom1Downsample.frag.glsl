uniform sampler2DRect fboTex;
uniform float avgLum;
uniform float middleGrey;
// 1st downscale stage of making the bloom texture
// only takes bits of fbo that are bright enough

void main(void)
{
	const vec3 getlum = vec3(0.299, 0.587, 0.114);
	vec3 col = vec3(0.0);
	vec2 p = 2.0*gl_FragCoord.xy;
	vec3 c;
	float lum;
	float minLumToBloom = max(32.0*avgLum, 1.0);
	
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y)));
	lum = dot(getlum, c);
	if (lum > minLumToBloom) col += c;//*(lum-minLumToBloom);
	// This bit causes gradual onset of bloom    ^^^^^^^^^
	
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y)));
	lum = dot(getlum, c);
	if (lum > minLumToBloom) col += c;//*(lum-minLumToBloom);
	
	c = vec3(texture2DRect(fboTex, vec2(p.x+1.0, p.y+1.0)));
	lum = dot(getlum, c);
	if (lum > minLumToBloom) col += c;//*(lum-minLumToBloom);
	
	c = vec3(texture2DRect(fboTex, vec2(p.x, p.y+1.0)));
	lum = dot(getlum, c);
	if (lum > minLumToBloom) col += c;//*(lum-minLumToBloom);
	
	col *= 0.25;
	gl_FragColor = vec4(col.r, col.g, col.b, 0.0);
}
