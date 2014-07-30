// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

#ifdef ECLIPSE
uniform int shadows;
uniform ivec3 occultedLight;
uniform vec3 shadowCentreX;
uniform vec3 shadowCentreY;
uniform vec3 shadowCentreZ;
uniform vec3 srad;
uniform vec3 lrad;
uniform vec3 sdivlrad;
#endif // ECLIPSE

varying vec4 varyingEyepos;

void sphereEntryExitDist(out float near, out float far, const in vec3 sphereCenter, const in vec3 eyeTo, const in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float i1, i2;
	near = 0.0;
	far = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		i1 = b - det;
		i2 = b + det;
		if (i2 > 0.0) {
			near = max(i1, 0.0);
			far = i2;
		}
	}
}

// integral used in shadow calculations:
// \Int (m - \sqrt(d^2+t^2)) dt = (t\sqrt(d^2+t^2) + d^2 log(\sqrt(d^2+t^2)+t))/2
float shadowInt(const in float t1, const in float t2, const in float dsq, const in float m)
{
	float s1 = sqrt(dsq+t1*t1);
	float s2 = sqrt(dsq+t2*t2);
	return m*(t2-t1) - (t2*s2 - t1*s1 + dsq*( log(max(0.000001, s2+t2)) - log(max(0.000001, s1+t1)))) * 0.5;
}

void main(void)
{
	float skyNear, skyFar;
	vec3 eyenorm = normalize(varyingEyepos.xyz);
	float specularHighlight=0.0;

	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, varyingEyepos.xyz, geosphereScaledRadius * geosphereAtmosTopRad);
	float atmosDist = geosphereScale * (skyFar - skyNear);
	float ldprod=0.0;
	
	// a&b scaled so length of 1.0 means planet surface.
	vec3 a = (skyNear * eyenorm - geosphereCenter) / geosphereScaledRadius;
	vec3 b = (skyFar * eyenorm - geosphereCenter) / geosphereScaledRadius;
	ldprod = AtmosLengthDensityProduct(a, b, atmosColor.a * geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
	
	float fogFactor = 1.0 / exp(ldprod);
	vec4 atmosDiffuse = vec4(0.0);

#if (NUM_LIGHTS > 0)	
	vec3 surfaceNorm = normalize(skyNear * eyenorm - geosphereCenter);
	for (int i=0; i<NUM_LIGHTS; ++i) {

		vec3 lightDir = normalize(vec3(gl_LightSource[i].position));

		float uneclipsed = 1.0;
#ifdef ECLIPSE
		for (int j=0; j<shadows; j++) {
			if (i != occultedLight[j])
				continue;

			// Eclipse handling:
			// Calculate proportion of the in-atmosphere eyeline which is shadowed,
			// weighting according to completeness of the shadow (penumbra vs umbra).
			// This ignores variation in atmosphere density, and ignores outscatter along
			// the eyeline, so is not very accurate. But it gives decent results.

			vec3 centre = vec3( shadowCentreX[j], shadowCentreY[j], shadowCentreZ[j] );

			vec3 ap = a - dot(a,lightDir)*lightDir - centre;
			vec3 bp = b - dot(b,lightDir)*lightDir - centre;

			vec3 dirp = normalize(bp-ap);
			float ad = dot(ap,dirp);
			float bd = dot(bp,dirp);
			vec3 p = ap - dot(ap,dirp)*dirp;
			float perpsq = dot(p,p);

			// we now want to calculate the proportion of shadow on the horizontal line
			// segment from ad to bd, shifted vertically from centre by \sqrt(perpsq). For
			// the partially occluded segments, to have an analytic solution to the integral
			// we estimate the light intensity to drop off linearly with radius between
			// maximal occlusion and none.

			float minr = srad[j]-lrad[j];
			float maxr = srad[j]+lrad[j];
			float maxd = sqrt( max(0.0, maxr*maxr - perpsq) );
			float mind = sqrt( max(0.0, minr*minr - perpsq) );

			float shadow = ( shadowInt(clamp(ad, -maxd, -mind), clamp(bd, -maxd, -mind), perpsq, maxr)
				+ shadowInt(clamp(ad, mind, maxd), clamp(bd, mind, maxd), perpsq, maxr) )
				/ (maxr-minr) + (clamp(bd, -mind, mind) - clamp(ad, -mind, mind));

			float maxOcclusion = min(1.0, (sdivlrad[j])*(sdivlrad[j]));

			uneclipsed -= maxOcclusion * shadow / (bd-ad);
		}
#endif // ECLIPSE
		uneclipsed = clamp(uneclipsed, 0.0, 1.0);

		float nDotVP =  max(0.0, dot(surfaceNorm, lightDir));
		float nnDotVP = max(0.0, dot(surfaceNorm, -lightDir));  //need backlight to increase horizon
		atmosDiffuse +=  gl_LightSource[i].diffuse * uneclipsed * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0) * INV_NUM_LIGHTS);

		//Calculate Specular Highlight
		vec3 L = normalize(gl_LightSource[i].position.xyz - varyingEyepos.xyz); 
		vec3 E = normalize(-varyingEyepos.xyz);
		vec3 R = normalize(-reflect(L,vec3(0.0))); 
		specularHighlight += pow(max(dot(R,E),0.0),64.0) * uneclipsed * INV_NUM_LIGHTS;

	}
#endif

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (atmosDiffuse.r+atmosDiffuse.g+atmosDiffuse.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	atmosDiffuse.a = 1.0;
	gl_FragColor = (1.0-fogFactor) * (atmosDiffuse*
		vec4(atmosColor.rgb, 1.0)) +
		(0.02-clamp(fogFactor,0.0,0.01))*atmosDiffuse*ldprod*sunset +     //increase light on lower atmosphere.
		atmosColor*specularHighlight*(1.0-fogFactor)*sunset;		  //add light from specularHighlight.

	SetFragDepth();
}
