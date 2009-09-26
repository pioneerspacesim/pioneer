
void DirectionalLight(in int i,
                       in vec3 normal,
                       inout vec4 ambient,
                       inout vec4 diffuse,
                       inout vec4 specular)
{
	float nDotVP; 
	float nDotHV;         
	float pf;             
	nDotVP = max(0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));
	nDotHV = max(0.0, dot(normal, vec3(gl_LightSource[i].halfVector)));
	if (nDotVP == 0.0) pf = 0.0;
	else pf = pow(nDotHV, gl_FrontMaterial.shininess);
	ambient += gl_LightSource[i].ambient;
	diffuse += gl_LightSource[i].diffuse * nDotVP;
	specular += gl_LightSource[i].specular * pf;
}

void DirectionalLight(in int i,
                       in vec3 normal,
                       inout vec4 ambient,
                       inout vec4 diffuse)
{
	float nDotVP = max(0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));
	ambient += gl_LightSource[i].ambient;
	diffuse += gl_LightSource[i].diffuse * nDotVP;
}

// Calculate length*density product of a line through the atmosphere
// a - start coord (normalized relative to atmosphere radius)
// b - end coord " "
// centerDensity - atmospheric density at centre of sphere
// length - real length of line in meters
// Density equation is d = D-D*r where D=centreDensity, r=point dist from centre
float AtmosLengthDensityProduct(vec3 a, vec3 b, float centreDensity, float len)
{
	/* 6 samples */
	float ldprod = 0.0;
	vec3 dir = b-a;
	ldprod = 6.0*centreDensity - centreDensity*(
			length(a) +
			length(a + 0.2*dir) +
			length(a + 0.4*dir) +
			length(a + 0.6*dir) +
			length(a + 0.8*dir) +
			length(b));
	ldprod *= len / 6.0;
	return ldprod;	
}

