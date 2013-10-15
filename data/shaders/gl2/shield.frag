varying vec3 varyingEyepos;
varying vec3 varyingNormal;

uniform Scene scene;
uniform Material material;
uniform float shieldStrength;

//for station waypoint interpolation
vec4 lerp(const float t, const vec4 v1, const vec4 v2)
{
	return t*v2 + (1.0-t)*v1;
}

vec4 red = vec4(1.0, 0.5, 0.5, 0.5);
vec4 blue = vec4(0.5, 0.5, 1.0, 1.0);

void main(void)
{
	//vec4 color = material.diffuse;
	vec4 color = lerp(shieldStrength, red, blue);
	vec4 fillColour = color * 0.15;
	
	vec3 eyenorm = normalize(-varyingEyepos);

	float fresnel = 1.0 - abs(dot(eyenorm, varyingNormal)); // Calculate fresnel.
	fresnel = pow(fresnel, 10.0);
	fresnel += 0.05 * (1.0 - fresnel);
	
	color.a = (fillColour + clamp(fresnel * 0.5, 0.0, 1.0)) * ((shieldStrength+1.0) * 0.5);
	
	gl_FragColor = color;

	SetFragDepth();
}
