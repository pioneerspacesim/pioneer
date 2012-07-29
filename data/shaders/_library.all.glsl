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
float AtmosLengthDensityProduct(vec3 a, vec3 b, float surfaceDensity, float len, float invScaleHeight)
{
	/* 4 samples */
	float ldprod = 0.0;
	vec3 dir = b-a;
	ldprod = surfaceDensity * (
			exp(-invScaleHeight*(length(a)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.2*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.4*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.6*dir)-1.0)) +
			exp(-invScaleHeight*(length(a + 0.8*dir)-1.0)) +
			exp(-invScaleHeight*(length(b)-1.0)));
	ldprod *= len;
	return ldprod;
}

float findSphereEyeRayEntryDistance(in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float entryDist = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0.0) {
			entryDist = max(i1, 0.0);
		}
	}
	return entryDist;
}

