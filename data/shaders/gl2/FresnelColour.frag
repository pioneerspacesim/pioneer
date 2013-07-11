varying vec3 varyingEyepos;
varying vec3 varyingNormal;

uniform Scene scene;
uniform Material material;
uniform vec3 fresnelCentre;

void main(void)
{
	vec4 color = material.diffuse;
	
	vec3 eyenorm = normalize(-fresnelCentre);
	vec3 normal = normalize(varyingNormal);

	vec3 h = normalize(normal + eyenorm); // Half-vector.
	float fresnel = 1.0 - dot(eyenorm, h); // Caculate fresnel.
	fresnel = pow(fresnel, 5.0);
	fresnel += 0.1 * (1.0 - fresnel);
	gl_FragColor = color * fresnel;

	SetFragDepth();
}
