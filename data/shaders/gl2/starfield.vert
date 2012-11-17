//http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

float noise2dtrig(vec2 x){
    return abs(0.5-fract(sin(dot(x.xy ,vec2(12.9898,78.233))) * 43758.5453))*2.0;
}

/*
struct Material {
	vec4 emission;
};
*/
//uniform Material material;

uniform float time;
uniform float brightness;
uniform bool twinkling;
uniform float effect;
uniform float starScaling;

varying vec4 color;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	float starSize = gl_Color.r;
	vec3 col = vec3(gl_Color.r);
	col = vec3(gl_Color.g);

	float b = 1.0;
	if (twinkling){
		
 		//id for each star sent in alpha channel
		float p = gl_Color.a*200.0;

		// select stars for twinkling by choosing a section of the fractional part of starid*largenumber
		// the section is moved with time
		float tw = fract(100.0*p+0.4*time);

		#define fractToTwinkle 0.3
		// tw: 0 in fractToTwinkle to 1.0, 0-1 as (0.0 to 0.1)*Fract to twinkle, 0-1 as (0.9 to 1.0) * fractToTwinkle
		tw = (tw<fractToTwinkle)?
			((clamp(abs((0.5*fractToTwinkle)-tw)*(1.0/(0.5*fractToTwinkle)) 
			       ,0.6,1.0) - 0.6)*(1.0/0.4))
			:0.0;


		//input time and star id as coordinates of 2d noise space
		b = 1.75*pow(0.01+0.99*noise2dtrig(vec2(p, 0.0000005*time)),0.20);
		//b = clamp(noise2dtrig(vec2(p, 0.000005*time)),0.0,0.2)/0.2;
		// for small stars reduce range of change
		b = mix(1.0,b,(clamp(starSize-0.51,0.0,0.04))*(1.0/0.04));
		// blend away from 1.0 with effect and twinkling mask
		b = mix(1.0,b,effect*tw);
	}

	gl_PointSize = starScaling*(1.0 + (b)*starSize); //b controls a portion of star size 
	color = vec4(col.rgb,brightness*b);
}
