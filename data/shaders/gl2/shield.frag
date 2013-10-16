varying vec3 varyingEyepos;
varying vec3 varyingNormal;

uniform Scene scene;
uniform Material material;
uniform float shieldStrength;
uniform float shieldCooldown;

const vec4 red = vec4(1.0, 0.5, 0.5, 0.5);
const vec4 blue = vec4(0.5, 0.5, 1.0, 1.0);

void main(void)
{
	//vec4 color = material.diffuse;
	vec4 color = mix(red, blue, shieldStrength);
	vec4 fillColour = color * 0.15;
	
	vec3 eyenorm = normalize(-varyingEyepos);

	float fresnel = 1.0 - abs(dot(eyenorm, varyingNormal)); // Calculate fresnel.
	fresnel = pow(fresnel, 10.0);
	fresnel += 0.05 * (1.0 - fresnel);
	
	// combine a base colour with the (clamped) fresnel value and fade it out according to the cooldown period.
	color.a = (fillColour + clamp(fresnel * 0.5, 0.0, 1.0)) * shieldCooldown;
	
	gl_FragColor = color;

	SetFragDepth();
}
