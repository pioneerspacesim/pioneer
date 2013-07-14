varying vec3 varyingEyepos;
varying vec3 varyingNormal;

uniform Scene scene;
uniform Material material;
uniform vec3 fresnelCentre;

void main(void)
{
	vec4 color = material.diffuse;
	
	vec3 eyenorm = normalize(-varyingEyepos);
	vec3 normal = normalize(varyingNormal);

	float fresnel = 1.0 - clamp(abs(dot(eyenorm, normal)), 0.0, 1.0); // Caculate fresnel.
	fresnel = pow(fresnel, 5.0);
	fresnel += 0.05 * (1.0 - fresnel);
	color.a = color.a * fresnel;
	gl_FragColor = color;

	SetFragDepth();
}
