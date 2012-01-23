uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereAtmosTopRad;
uniform float geosphereRadius;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;

uniform int occultedLight;
uniform vec3 occultCentre;
uniform float srad;
uniform float lrad;
uniform float maxOcclusion;

varying vec4 varyingEyepos;

void sphereEntryExitDist(out float near, out float far, in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	near = 0.0;
	far = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0.0) {
			near = max(i1, 0.0);
			far = i2;
		}
	}
}

void main(void)
{
	float skyNear, skyFar;
	vec3 eyepos = vec3(varyingEyepos);
	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, eyepos, geosphereAtmosTopRad);
	float atmosDist = geosphereScale * (skyFar - skyNear);
	float ldprod;
	{
		vec3 dir = normalize(eyepos);
		vec3 a = (skyNear * dir - geosphereCenter) / geosphereAtmosTopRad;
		vec3 b = (skyFar * dir - geosphereCenter) / geosphereAtmosTopRad;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*geosphereAtmosFogDensity, atmosDist);
	}
	float fogFactor = 1.0 / exp(ldprod);
	vec4 atmosDiffuse = vec4(0.0,0.0,0.0,1.0);
	{
		vec3 surfaceNorm = normalize(eyepos - geosphereCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {
			vec3 lightDir = normalize(vec3(gl_LightSource[i].position));
			float uneclipsed = 1.0;
			if (i == occultedLight)
			{
				// Eclipse handling:
				// Calculate proportion of the in-atmosphere eyeline which is shadowed,
				// weighting according to completeness of the shadow (penumbra vs umbra).
				// This ignores variation in atmosphere density, and ignores outscatter along
				// the eyeline, so is not very accurate. But it gives decent results.
				vec3 dir = normalize(eyepos);
				vec3 a = (skyNear * dir - geosphereCenter) / geosphereRadius;
				vec3 b = (skyFar * dir - geosphereCenter) / geosphereRadius;
				vec3 ap = a - dot(a,lightDir)*lightDir - occultCentre;
				vec3 bp = b - dot(b,lightDir)*lightDir - occultCentre;

				vec3 dirp = normalize(bp-ap);
				float ad = dot(ap,dirp);
				float bd = dot(bp,dirp);
				if (bd > ad) {
					vec3 p = ap - dot(ap,dirp)*dirp;
					float perpsq = dot(p,p);

					// We do need a gradually deepening penumbra, or we get ugly lines when the
					// shadow is viewed side-on; a more efficient approach to this than the
					// following loop would be nice.
					const int pN = 12;
					for (int pi=pN; pi>=0; pi--) {
						float c = (srad+pi*lrad/pN)*(srad+pi*lrad/pN) - perpsq;
						if (c < 0.0)
							break;
						float rootc = sqrt(c);
						uneclipsed -= (maxOcclusion/(pN+1)) *
							(clamp(rootc,ad,bd) - clamp(-rootc,ad,bd)) / (bd-ad);
					}
				}
				// Note: the shadow the planet casts on its own atmosphere is dealt with adequately
				// by the "dot(surfaceNorm, lightDir)" term below.
			}
			atmosDiffuse += gl_LightSource[i].diffuse * uneclipsed * max(0.0, dot(surfaceNorm, lightDir));
		}
	}
	atmosDiffuse.a = 1.0;
	//float sun = max(0.0, dot(normalize(eyepos),normalize(vec3(gl_LightSource[0].position))));
	gl_FragColor = (1.0-fogFactor) * (atmosDiffuse*
		vec4(atmosColor.r, atmosColor.g, atmosColor.b, 1.0));

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
