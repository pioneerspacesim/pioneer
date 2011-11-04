uniform sampler2DRect fboTex;
uniform sampler2DRect bloomTex;
uniform float avgLum;
uniform float middleGrey;

void main(void)
{
	vec3 col = vec3(texture2DRect(fboTex, vec2(gl_FragCoord.x, gl_FragCoord.y)));
	col += 0.1 * vec3(texture2DRect(bloomTex, vec2(gl_FragCoord.x*0.25, gl_FragCoord.y*0.25)));

	// This is the reinhard tonemapping algorithm, i think...
	float X,Y,Z,x,y;
	X = dot(vec3(0.4124, 0.3576, 0.1805), col);
	Y = dot(vec3(0.2126, 0.7152, 0.0722), col);
	Z = dot(vec3(0.0193, 0.1192, 0.9505), col);
	x = X / (X+Y+Z);
	y = Y / (X+Y+Z);
	// Y is luminance. fiddle with it
	Y = Y * (middleGrey / avgLum); // scale luminance
//	Y *= lum;
	Y = Y / (1.0 + Y); // compress luminance
	// convert back to RGB
	X = x*(Y/y);
	Z = (1.0-x-y)*(Y/y);
	col.r = 3.2405*X - 1.5371*Y - 0.4985*Z;
	col.g = -0.9693*X + 1.8760*Y + 0.0416*Z;
	col.b = 0.0556*X - 0.2040*Y + 1.0572*Z;

	gl_FragColor = vec4(col.r, col.g, col.b, 1.0);
}
