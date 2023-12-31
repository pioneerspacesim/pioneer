// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef ECLIPSE

struct Eclipse {
	vec3 shadowCentreX;
	vec3 shadowCentreY;
	vec3 shadowCentreZ;
	vec3 srad;
	vec3 lrad;
	vec3 sdivlrad;
};

float discCovered(const in float dist, const in float rad) {
	// proportion of unit disc covered by a second disc of radius rad placed
	// dist from centre of first disc.
	//
	// XXX: same function is in Camera.cpp
	//
	// WLOG, the second disc is displaced horizontally to the right.
	// xl = rightwards distance to intersection of the two circles.
	// xs = normalised leftwards distance from centre of second disc to intersection.
	// d = vertical distance to an intersection point
	//
	// The clamps on xl,xs handle the cases where one disc contains the other.

	float radsq = rad*rad;

	float xl = clamp((dist*dist + 1.0 - radsq) / (2.0*max(0.001,dist)), -1.0, 1.0);
	float xs = clamp((dist - xl)/max(0.001,rad), -1.0, 1.0);
	float d = sqrt(max(0.0, 1.0 - xl*xl));

	float th = clamp(acos(xl), 0.0, PI);
	float th2 = clamp(acos(xs), 0.0, PI);

	// covered area can be calculated as the sum of segments from the two
	// discs plus/minus some triangles, and it works out as follows:
	return clamp((th + radsq*th2 - dist*d)/PI, 0.0, 1.0);
}

// integral used in shadow calculations:
// \Int (m - \sqrt(d^2+t^2)) dt = (t\sqrt(d^2+t^2) + d^2 log(\sqrt(d^2+t^2)+t))/2
float shadowInt(const in float t1, const in float t2, const in float dsq, const in float m)
{
	float s1 = sqrt(dsq+t1*t1);
	float s2 = sqrt(dsq+t2*t2);
	return m*(t2-t1) - (t2*s2 - t1*s1 + dsq*( log(max(0.000001, s2+t2)) - log(max(0.000001, s1+t1)))) * 0.5;
}

float calcUneclipsed(const in Eclipse params, const in int numShadows, const in vec3 v, const in vec3 lightDir)
{
	float uneclipsed = 1.0;
	for (int j=0; j<numShadows; j++)
	{
		vec3 centre = vec3( params.shadowCentreX[j], params.shadowCentreY[j], params.shadowCentreZ[j] );

		// Apply eclipse:
		vec3 projectedPoint = v - dot(lightDir,v)*lightDir;
		// By our assumptions, the proportion of light blocked at this point by
		// this sphere is the proportion of the disc of radius lrad around
		// projectedPoint covered by the disc of radius srad around shadowCentre.
		float dist = length(projectedPoint - centre);
		uneclipsed *= 1.0 - discCovered(dist/params.lrad[j], params.sdivlrad[j]);
	}
	return uneclipsed;
}

float calcUneclipsedSky(const in Eclipse params, const in int numShadows, const in vec3 a, const in vec3 b, const in vec3 lightDir)
{
	float uneclipsed = 1.0;
	for (int j=0; j<numShadows; j++)
	{
		// Eclipse handling:
		// Calculate proportion of the in-atmosphere eyeline which is shadowed,
		// weighting according to completeness of the shadow (penumbra vs umbra).
		// This ignores variation in atmosphere density, and ignores outscatter along
		// the eyeline, so is not very accurate. But it gives decent results.

		vec3 centre = vec3( params.shadowCentreX[j], params.shadowCentreY[j], params.shadowCentreZ[j] );

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

		float minr = params.srad[j]-params.lrad[j];
		float maxr = params.srad[j]+params.lrad[j];
		float maxd = sqrt( max(0.0, maxr*maxr - perpsq) );
		float mind = sqrt( max(0.0, minr*minr - perpsq) );

		float shadow = ( shadowInt(clamp(ad, -maxd, -mind), clamp(bd, -maxd, -mind), perpsq, maxr)
			+ shadowInt(clamp(ad, mind, maxd), clamp(bd, mind, maxd), perpsq, maxr) )
			/ (maxr-minr) + (clamp(bd, -mind, mind) - clamp(ad, -mind, mind));

		float maxOcclusion = min(1.0, (params.sdivlrad[j])*(params.sdivlrad[j]));

		uneclipsed -= maxOcclusion * shadow / (bd-ad);
	}
	return uneclipsed;
}

#else

#define calcUneclipsed(p,a,b,c) 1.0
#define calcUneclipsedSky(p,a,b,c,d) 1.0

#endif // ECLIPSE
