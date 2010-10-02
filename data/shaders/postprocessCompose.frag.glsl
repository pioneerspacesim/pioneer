uniform sampler2DRect fboTex;
uniform sampler2DRect bloomTex;

void main(void)
{
	vec4 col = texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y));
	col += texture2DRect(bloomTex, vec2(gl_FragCoord.x*0.25, gl_FragCoord.y*0.25));
	float v;
	if ((col.r > col.g) && (col.r > col.b)) v = col.r;
	else if (col.g > col.b) v = col.g;
	else v = col.b;
	v = v / (v + 1.0);
	col *= v;
	gl_FragColor = col;
}
