//http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

float noise2dtrig(vec2 x){
    return abs(fract(sin(dot(x.xy ,vec2(12.9898,78.233))) * 43758.5453));
}


uniform float time;
uniform bool twinkling;
uniform float effect;


void main(void)
{
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif

	float starSize = pow(gl_Color.r,4.0);
	float b = 1.0;
	if (twinkling){
		
 		//id for each star sent in alpha channel
		float p = gl_Color.a*200.0;
		//input time and star id as coordinates of 2d noise space
		b = pow(0.4+0.6*noise2dtrig(vec2(p, time)),0.4);
		// for small stars reduce range of change
		b = mix(1.0,b,(clamp(starSize,0.00,0.2))*(1.0/0.2));
		// blend away from 1.0 with effect
		b = mix(1.0,b,effect);
	}

	gl_PointSize = 1.0 + (b*2.5+0.8)*starSize; //b controls a portion of star size 
	gl_FrontColor = vec4(gl_Color.rgb,gl_FrontMaterial.emission*b);
}

