float height(const in vec3 orig, const in vec3 center)
{
	vec3 r = orig - center;
	float height = sqrt(dot(r, r)) - geosphereRadius;

	return height;
}

void scatter(out vec2 density, const in vec3 orig, const in vec3 center)
{
	float height = height(orig, center);

	density = -height / scaleHeight;

	// earth atmospheric density: 1.225 kg/m^3, divided by 1e5
	// 1/1.225e-5 = 81632.65306
	float earthDensities = geosphereAtmosFogDensity * 81632.65306f;
	density /= earthDensities;
}

// orig: ray origin
// dir: ray direction
// center: planet center
// returns (h, t): h = length of perpendicular from center to ray
//                 t = distance left from perpendicular to origin along the ray
//                     (negative means perpendicular left behind)
/*
 *     t>0      H        t<0       dir
 * ----*--------*--------*----------->
 *              |
 *              |
 *              |
 *              * O
 */
void findClosestHeight(out float h, out float t, const in vec3 orig, const in vec3 dir, const in vec3 center)
{
	vec3 radiusVector = center - orig;
	vec3 tangent = dot(dir, radiusVector) * dir;
	vec3 normal = radiusVector - tangent;
	h = sqrt(dot(normal, normal));
	t = dot(tangent, dir);
}

// error function approx., used in predictDensityIn
float erf(const in float x)
{
	float a = 0.14001228904;
	float four_over_pi = 1.27323954;

	float x2 = x*x;
	float r = -x2 * (four_over_pi + a * x2) / (1 + a * x2);
	if (x > 0)
		return  sqrt(1-exp(r)) * 0.5 + 0.5;
	else
		return -sqrt(1-exp(r)) * 0.5 + 0.5;
}

// predict out-scattering density based on coefficients
float predictDensityOut(const in float atmosphereHeight, const in float height, const in float k, const in float b)
{
	if (height > atmosphereHeight)
		return 0.f;

	// earth atmospheric density: 1.225 kg/m^3, divided by 1e5
	// 1/1.225e-5 = 81632.65306
	float earthDensities = geosphereAtmosFogDensity * 81632.65306f;

	if (height < 0)
		return k / earthDensities;

	return k * exp(-height * b) / earthDensities;
}

// predict in-scattering rate: from 0 to 1
float predictDensityIn(const in float radius, const in float atmosphereHeight, const in float height, const in float c, const in float t)
{
	float minHeight = radius + height;
	float h = sqrt(minHeight*minHeight + t*t); // height for our position

	if (h > radius + atmosphereHeight) {
		if (t > 0)
			return 1.f; // no in-scattering, looking towards atmosphere
		else
			return 0.f; // looking from atmosphere
	} else {
		return erf(c * t); // erf is monotonic ascending
	}
}

// predict "scattering density" along the ray
// sample: starting point of the ray
// dir:    direction of ray
// center: relative position of planet center
// radius: planet radius
// atmosphereHeight: max height of atmosphere, bigger height means no atmosphere there
// coefficients: pre-computed (k, b, c) for:
//     k * exp(-height * b) for out-scattering density: k = density along the ray tangent to planet surface
//                                                      b = height at which density reduces by e
//     erf(c * t) for in-scattering density: c = density derivative per 1 km along the ray, assuming: k = 1, height = 0, t = 0 at tangent point
float predictDensityInOut(const in vec3 sample, const in vec3 dir, const in vec3 center, const in float radius, const in float atmosphereHeight, const in vec3 coefficients)
{
    float h, t;
    findClosestHeight(h, t, sample, dir, center);
    h -= radius;

    float opticalDepth = 0.f;
    // find out-scattering density
    opticalDepth = predictDensityOut(atmosphereHeight, h, coefficients.x, coefficients.y);

    // apply in-scattering filter
    opticalDepth *= predictDensityIn(radius, atmosphereHeight, h, coefficients.z, t);
    return opticalDepth;
}

void skipRay(inout vec2 opticalDepth, const in vec3 dir, const in vec2 boundaries, const in vec3 center)
{
	if (boundaries.y == boundaries.x)
		return;

	int numSamples = 8;

	float tCurrent = boundaries.x;
	float segmentLength = (boundaries.y - boundaries.x) / numSamples;
	for (int i = 0; i < numSamples; ++i) {
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * dir;

		// primary ray is approximated by (density * isegmentLength)
		vec2 density;
		scatter(density, samplePosition, center);
		opticalDepth += exp(density) * segmentLength;

		tCurrent += segmentLength;
	}
}

void processRay(inout vec3 sumR, inout vec3 sumM, inout vec2 opticalDepth, const in vec3 sunDirection, const in vec3 dir, const in vec2 boundaries, const in vec3 center, const in vec4 diffuse, const in float uneclipsed)
{
	if (boundaries.y == boundaries.x)
		return;

	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	int numSamples = 16;

	float atmosphereRadius = geosphereRadius * geosphereAtmosTopRad,
	      atmosphereHeight = atmosphereRadius - geosphereRadius;

	float tCurrent = boundaries.x;
	float segmentLength = (boundaries.y - boundaries.x) / numSamples;
	for (int i = 0; i < numSamples; ++i) {
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * dir;

		vec2 density;
		scatter(density, samplePosition, center);
		opticalDepth += exp(density) * segmentLength;

		// light optical depth
		vec2 opticalDepthLight = vec2(0.f);
		vec3 samplePositionLight = samplePosition;

		vec3 sampleGeoCenter = center - samplePosition;
		opticalDepthLight.x = predictDensityInOut(samplePositionLight, sunDirection, sampleGeoCenter, geosphereRadius, atmosphereHeight, coefficientsR);
		opticalDepthLight.y = predictDensityInOut(samplePositionLight, sunDirection, sampleGeoCenter, geosphereRadius, atmosphereHeight, coefficientsM);

		vec3 surfaceNorm = -normalize(sampleGeoCenter);
		vec4 atmosDiffuse = vec4(0.f);
		CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, surfaceNorm, uneclipsed);

		vec3 tau = -(betaR * (opticalDepth.x + opticalDepthLight.x) + betaM * 1.1f * (opticalDepth.y + opticalDepthLight.y));
		vec3 tauR = tau + vec3(density.x);
		vec3 tauM = tau + vec3(density.y);
		vec3 attenuationR = exp(tauR) * segmentLength;
		vec3 attenuationM = exp(tauM) * segmentLength;
		sumR += attenuationR * atmosDiffuse.xyz;
		sumM += attenuationM * atmosDiffuse.xyz;
		tCurrent += segmentLength;
	}
}

// replace (a, b) by (b, a) if a > b
vec2 sortAscending(const in vec2 segment)
{
	return (segment.x > segment.y) ? vec2(segment.y, segment.x) : segment;
}

// given a and b segments, return c = a \ b
vec4 segmentSubtraction(const in vec2 a, const in vec2 b)
{
	vec2 as = sortAscending(a);
	vec2 bs = sortAscending(b);

	// b could be inside a, leaving segments at both sides
	vec4 c;
	c.x = a.x;
	c.w = a.y;

	c.y = min(a.y, max(a.x, b.x));
	c.z = max(a.x, min(a.y, b.y));
	return c;
}

vec3 computeIncidentLight(const in vec3 sunDirection, const in vec3 dir, const in vec3 center, const in vec2 atmosDist, const in vec4 diffuse, const in float uneclipsed)
{
	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	float atmosMin = atmosDist.x * geosphereRadius;
	float atmosMax = atmosDist.y * geosphereRadius;

	// solve Cylinder entry/exit dist
	vec2 cylinder_intersect = rayCylinderIntersect(dir, center, sunDirection, geosphereRadius);
	bool hasIntersect = cylinder_intersect.x != 0 || cylinder_intersect.y != 0;

	vec3 cylinder_near = center - dir * cylinder_intersect.x;
	vec3 cylinder_far  = center - dir * cylinder_intersect.y;

	// test if ray passes through shadow
	float a = dot(cylinder_near, sunDirection);
	float b = dot(cylinder_far , sunDirection);
	bool intersectsShadow = hasIntersect && (a > 0.f || b > 0.f);

	vec2 ground_intersect = raySphereIntersect(center, dir, geosphereRadius);
	bool shadowVisible = intersectsShadow && ground_intersect.x == 0.f;

	/*
	* We have three options:
	* 1) Ray does not intersect shadow
	*    Do nothing
	* 2) Ray intersects shadow, starts inside
	*    (cylinder_intersect.y, tmax)
	* 3) Ray intersects shadow, starts outside
	*    (tmin, cylinder_intersect.x) + (cylinder_intersect.y, tmax)
	*/

	vec2 atmosphere_intersect = raySphereIntersect(center, dir, geosphereRadius * geosphereAtmosTopRad);
	vec4 atmosphere_minus_shadow = shadowVisible ? segmentSubtraction(atmosphere_intersect, cylinder_intersect) : vec4(atmosphere_intersect.x, atmosphere_intersect.y, atmosphere_intersect.y, atmosphere_intersect.y);

	if (ground_intersect.x > 0.f) {
		atmosphere_minus_shadow.yzw = min(atmosphere_minus_shadow.yzw, ground_intersect.x);
	}
	if (ground_intersect.y < 0.f) {
		atmosphere_minus_shadow.xyz = max(atmosphere_minus_shadow.xyz, ground_intersect.y);
	}

	vec3 sumR = vec3(0.f);
	vec3 sumM = vec3(0.f);
	vec2 opticalDepth = vec2(0.f);

	processRay(sumR, sumM, opticalDepth, sunDirection, dir, atmosphere_minus_shadow.xy, center, diffuse, uneclipsed);
	skipRay(opticalDepth, dir, atmosphere_minus_shadow.yz, center);
	processRay(sumR, sumM, opticalDepth, sunDirection, dir, atmosphere_minus_shadow.zw, center, diffuse, uneclipsed);

	float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);

	vec3 ret = (sumR * betaR * phaseR + sumM * betaM * phaseM);
	return ret;
}
