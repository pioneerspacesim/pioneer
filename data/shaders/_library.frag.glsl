
uniform float invLogZfarPlus1;
varying float varLogDepth;

void SetFragDepth()
{
	gl_FragDepth = gl_DepthRange.near + (gl_DepthRange.far * log(varLogDepth + 1.0) * invLogZfarPlus1);
}

void PointLight(in int i,
	in vec3 eye,
	in vec3 ecPosition3,
	in vec3 normal,
	inout vec4 ambient,
	inout vec4 diffuse,
	inout vec4 specular)
{
	float nDotVP;
	// normal . light direction
	float nDotHV;
	// normal . light half vector
	float pf;
	// power factor
	float attenuation;
	// computed attenuation factor
	float d;
	// distance from surface to light source
	vec3 VP;
	// direction from surface to light position
	vec3 halfVector;
	// direction of maximum highlights
	// Compute vector from surface to light position
	VP = vec3(gl_LightSource[i].position) - ecPosition3;
	// Compute distance between surface and light position
	d = length(VP);
	// Normalize the vector from surface to light position
	VP = normalize(VP);
	// Compute attenuation
	attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
		gl_LightSource[i].linearAttenuation * d +
		gl_LightSource[i].quadraticAttenuation * d * d);
	halfVector = normalize(VP + eye);
	nDotVP = max(0.0, dot(normal, VP));
	nDotHV = max(0.0, dot(normal, halfVector));
	if (nDotVP == 0.0) pf = 0.0;
	else pf = pow(nDotHV, gl_FrontMaterial.shininess);
	ambient += gl_LightSource[i].ambient * attenuation;
	diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;
	specular += gl_LightSource[i].specular * pf * attenuation;
}

