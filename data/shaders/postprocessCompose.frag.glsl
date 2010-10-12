#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect fboTex;
uniform sampler2DRect bloomTex;
uniform float avgLum;

void main(void)
{
	// higher values gives a brighter scene
	const float alpha = 0.18;
	vec3 col = vec3(texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y)));
	//col += texture2DRect(bloomTex, vec2(gl_FragCoord.x*0.25, gl_FragCoord.y*0.25));
	const float lum = alpha / avgLum;
	vec3 yuv;
	yuv.r = lum * dot(col, vec3(0.299, 0.587, 0.114));
	yuv.g = dot(col, vec3(-0.147, -0.289, 0.436));
	yuv.b = dot(col, vec3(0.615, -0.515, -0.100));
	
	col.r = yuv.r + 1.140*yuv.b;
	col.g = yuv.r - 0.395*yuv.g - 0.581*yuv.b;
	col.b = yuv.r + 2.032*yuv.g;

	gl_FragColor = vec4(col.r, col.g, col.b, 1.0);
}
