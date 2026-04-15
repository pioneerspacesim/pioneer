// get height in m
float height(const in vec3 rayStart)
{
	float height = sqrt(dot(rayStart, rayStart)) - geosphereRadius;

	return height;
}

// get density at given point
vec2 getDensityAtPointOld(const in vec3 rayStart, const in sampler2D texture_LUT)
{
	float height = height(rayStart); // in meters

	vec2 density = -height / scaleHeight;

	// earth atmospheric density: 1.225 kg/m^3, divided by 1e5
	// 1/1.225e-5 = 81632.65306
	float earthDensities = log(geosphereAtmosFogDensity * 81632.65306f);
	density += earthDensities;
	return exp(density);
}

// get density at given point
vec2 getDensityAtPointNew(const in vec3 rayStart, const in sampler2D texture_LUT)
{
	float height = height(rayStart); // in meters
	float maxHeight = (geosphereAtmosTopRad - 1.f) * geosphereRadius;

	float ratio = height / maxHeight;

	vec2 density = vec2(0.f);
	// normalized to [0; 1) - remap back to [-128; 128)
	density.x = texture(texture_LUT, vec2(0.0, ratio)).x;
	density.y = texture(texture_LUT, vec2(1.0, ratio)).x;

	density = density * 256 - 128;
	return exp(density);
}

vec2 getDensityAtPoint(const in vec3 rayStart, const in sampler2D texture_LUT)
{
#if 1
    return getDensityAtPointNew(rayStart, texture_LUT);
#else
    return getDensityAtPointOld(rayStart, texture_LUT);
#endif
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
void findClosestHeight(out float h, out float t, const in vec3 orig, const in vec3 rayDirection, const in vec3 rayStart)
{
	vec3 radiusVector = orig + rayStart;
	vec3 tangent = dot(rayDirection, radiusVector) * rayDirection;
	vec3 normal = tangent - radiusVector;
	h = sqrt(dot(normal, normal));
	t = dot(tangent, rayDirection);
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
float predictDensityInOut(const in vec3 sample, const in vec3 rayDirection, const in vec3 rayStart, const in float radius, const in float atmosphereHeight, const in vec3 coefficients)
{
    float h, t;
    findClosestHeight(h, t, sample, rayDirection, rayStart);
    h -= radius;

    float opticalDepth = 0.f;
    // find out-scattering density
    opticalDepth = predictDensityOut(atmosphereHeight, h, coefficients.x, coefficients.y);

    // apply in-scattering filter
    opticalDepth *= predictDensityIn(radius, atmosphereHeight, h, coefficients.z, t);
    return opticalDepth;
}

void skipRay(inout vec2 opticalDepth, const in vec3 rayDirection, const in vec2 boundaries, const in vec3 rayStart, const in sampler2D texture_LUT)
{
	if (boundaries.y == boundaries.x)
		return;

	int numSamples = 8;

	float tCurrent = boundaries.x;
	float segmentLength = (boundaries.y - boundaries.x) / numSamples;
	for (int i = 0; i < numSamples; ++i) {
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * rayDirection;

		// primary ray is approximated by (density * isegmentLength)
		vec2 density = getDensityAtPoint(samplePosition + rayStart, texture_LUT);
		opticalDepth += density * segmentLength;

		tCurrent += segmentLength;
	}
}

void processRayFast(inout vec3 sumR, inout vec3 sumM, inout vec2 opticalDepth, const in vec3 sunDirection, const in vec3 rayDirection, const in vec2 boundaries, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
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
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * rayDirection;

		vec2 density = getDensityAtPoint(samplePosition + rayStart, texture_LUT);
		vec2 depthAdd = density * segmentLength;
		opticalDepth += depthAdd;

		// light optical depth
		vec2 opticalDepthLight = vec2(0.f);
		vec3 samplePositionLight = samplePosition;

		vec3 sampleGeoCenter = samplePosition + rayStart;
		opticalDepthLight.x = predictDensityInOut(samplePositionLight, sunDirection, sampleGeoCenter, geosphereRadius, atmosphereHeight, coefficientsR);
		opticalDepthLight.y = predictDensityInOut(samplePositionLight, sunDirection, sampleGeoCenter, geosphereRadius, atmosphereHeight, coefficientsM);

		vec3 surfaceNorm = normalize(sampleGeoCenter);
		vec4 atmosDiffuse = vec4(0.f);
		CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, surfaceNorm, uneclipsed);

		vec3 tau = exp(-(betaR * (opticalDepth.x + opticalDepthLight.x) + betaM * 1.1f * (opticalDepth.y + opticalDepthLight.y)));
		sumR += tau * depthAdd.x * atmosDiffuse.xyz;
		sumM += tau * depthAdd.y * atmosDiffuse.xyz;
		tCurrent += segmentLength;
	}
}

void processRayFull(inout vec3 sumR, inout vec3 sumM, inout vec2 opticalDepth, const in vec3 sunDirection, const in vec3 rayDirection, const in vec2 boundaries, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
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
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * rayDirection;

		vec2 density = getDensityAtPoint(samplePosition + rayStart, texture_LUT);
		vec2 depthAdd = density * segmentLength;
		opticalDepth += depthAdd;

		// light optical depth
		vec2 opticalDepthLight = vec2(0.f);
		vec3 sampleGeoCenter = samplePosition + rayStart;

		int numSamplesLight = 8;
		vec2 boundariesLight = raySphereIntersect(-sampleGeoCenter, sunDirection, geosphereRadius * geosphereAtmosTopRad);
		float segmentLengthLight = boundariesLight.y / numSamplesLight;
		float tCurrentLight = 0.f;
		for (int j = 0; j < numSamplesLight; ++j) {
			vec3 samplePositionLight = vec3(segmentLengthLight * 0.5f + tCurrentLight) * sunDirection + samplePosition;
			vec2 densityLDir = getDensityAtPoint(samplePositionLight + rayStart, texture_LUT);
			opticalDepthLight += densityLDir * segmentLengthLight;

			tCurrentLight += segmentLengthLight;
		}

		vec3 surfaceNorm = normalize(sampleGeoCenter);
		vec4 atmosDiffuse = vec4(0.f);
		CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, surfaceNorm, uneclipsed);

		vec3 tau = exp(-(betaR * (opticalDepth.x + opticalDepthLight.x) + betaM * 1.1f * (opticalDepth.y + opticalDepthLight.y)));
		sumR += tau * depthAdd.x * atmosDiffuse.xyz;
		sumM += tau * depthAdd.y * atmosDiffuse.xyz;
		tCurrent += segmentLength;
	}
}

void processRay(inout vec3 sumR, inout vec3 sumM, inout vec2 opticalDepth, const in vec3 sunDirection, const in vec3 rayDirection, const in vec2 boundaries, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
{
#if 1
    processRayFull(sumR, sumM, opticalDepth, sunDirection, rayDirection, boundaries, rayStart, diffuse, uneclipsed, texture_LUT);
#else
    processRayFast(sumR, sumM, opticalDepth, sunDirection, rayDirection, boundaries, rayStart, diffuse, uneclipsed, texture_LUT);
#endif
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

/*
 * given:
 * sunDirection - direction from camera to light source, normalized
 * dir - direction from camera to pixel being rendered, normalized
 * center - position of planet relative to camera, in absolute scale
 *
 * splits given ray into parts by planet shadow:
 * - xy: ray before shadow
 * - yz: ray inside shadow
 * - zw: ray after shadow
 */
vec4 getRaySegment(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 rayStart)
{
	// solve Cylinder entry/exit dist
	vec2 cylinder_intersect = rayCylinderIntersect(rayDirection, -rayStart, sunDirection, geosphereRadius);
	bool hasIntersect = cylinder_intersect.x != 0 || cylinder_intersect.y != 0;

	vec3 cylinder_near = rayStart + rayDirection * cylinder_intersect.x;
	vec3 cylinder_far  = rayStart + rayDirection * cylinder_intersect.y;

	// test if ray passes through shadow
	float a = dot(cylinder_near, sunDirection);
	float b = dot(cylinder_far , sunDirection);
	bool intersectsShadow = hasIntersect && (a < 0.f || b < 0.f);

	vec2 ground_intersect = raySphereIntersect(-rayStart, rayDirection, geosphereRadius);
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

	vec2 atmosphere_intersect = raySphereIntersect(-rayStart, rayDirection, geosphereRadius * geosphereAtmosTopRad);
	vec4 atmosphere_minus_shadow = shadowVisible ? segmentSubtraction(atmosphere_intersect, cylinder_intersect) : vec4(atmosphere_intersect.x, atmosphere_intersect.y, atmosphere_intersect.y, atmosphere_intersect.y);

	if (ground_intersect.x > 0.f) {
		atmosphere_minus_shadow.yzw = min(atmosphere_minus_shadow.yzw, ground_intersect.x);
	}
	if (ground_intersect.y < 0.f) {
		atmosphere_minus_shadow.xyz = max(atmosphere_minus_shadow.xyz, ground_intersect.y);
	}

	return atmosphere_minus_shadow;
}

vec3 computeIncidentLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
{
	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	vec3 sumR = vec3(0.f);
	vec3 sumM = vec3(0.f);
	vec2 opticalDepth = vec2(0.f);

	vec4 segment = getRaySegment(sunDirection, rayDirection, rayStart);

	processRay(sumR, sumM, opticalDepth, sunDirection, rayDirection, segment.xy, rayStart, diffuse, uneclipsed, texture_LUT);
	skipRay(opticalDepth, rayDirection, segment.yz, rayStart, texture_LUT);
	processRay(sumR, sumM, opticalDepth, sunDirection, rayDirection, segment.zw, rayStart, diffuse, uneclipsed, texture_LUT);

	float mu = dot(rayDirection, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);

	vec3 ret = (sumR * betaR * phaseR + sumM * betaM * phaseM);
	return ret;
}

vec2 calculateDensityBetweenPoints(const in vec3 a, const in vec3 b, const in sampler2D texture_LUT)
{
	// get mean density between A and B and multiply by length
	// assume density scales linearly between A and B on short distances
	vec3 c = mix(a, b, 0.5);
	vec2 density = getDensityAtPoint(c, texture_LUT);

	return density * length(b - a);
}

/*
 * Given:
 * - Planet parameters
 * - Atmosphere parameters
 * - Point A
 * - Point B
 * Calculate:
 * - How much light did not scattered A -> B
 */
vec3 calculateAtmosphereAttenuation(const in vec3 a, const in vec3 b, const in sampler2D texture_LUT)
{
	vec2 opticalDepth = calculateDensityBetweenPoints(a, b, texture_LUT);

	// what does it mean?
	// is it multiplied by density and value of 1 means light dims by a factor of e?
	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	vec3 beta = betaR * opticalDepth.x + betaM * opticalDepth.y;
	return exp(-beta);
}

vec3 calculateDirectLight(const in vec3 sunDirection, const in vec3 a, const in vec3 b, const in sampler2D texture_LUT, const int samples)
{
	float rayLength = length(b - a);

	float segmentLength = rayLength / samples;

	vec3 lightMultiplier = vec3(1.f);

	for (int i = 0; i < samples; ++i) {
		vec3 segmentStart  = b - ((i + 0) * segmentLength) * sunDirection;
		vec3 segmentFinish = b - ((i + 1) * segmentLength) * sunDirection;

		lightMultiplier *= calculateAtmosphereAttenuation(segmentStart, segmentFinish, texture_LUT);
	}

	return lightMultiplier;
}

vec3 calculateIncidentLight2(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
{
	vec4 segment = getRaySegment(sunDirection, rayDirection, rayStart);

	/*
	 * rayStart -> enterAtm -> enterShadow -> exitShadow -> exitAtm -> rayFinish
	 */
	float enterAtm    = segment.x;
	float enterShadow = segment.y;
	float exitShadow  = segment.z;
	float exitAtm     = segment.w;

	int numSamples = 16;
	int numSamplesLight = 8;
	vec3 lightR = vec3(0.0f);
	vec3 lightM = vec3(0.0f);

	float mu = dot(rayDirection, sunDirection);
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);

	float segmentStart = exitShadow;
	float segmentFinish = exitAtm;
	if (segmentStart != segmentFinish) {
		float segmentLength = (segmentStart - segmentFinish) / numSamples;
		for (int i = 0; i < numSamples; ++i) {
			vec3 a = rayStart + ((i + 0.f) * segmentLength + segmentFinish) * rayDirection;
			vec3 b = rayStart + ((i + 1.f) * segmentLength + segmentFinish) * rayDirection;

			vec3 c = mix(a, b, 0.5f);
			vec3 d = c + sunDirection * raySphereIntersect(-c, sunDirection, geosphereRadius * geosphereAtmosTopRad).y;

			vec4 atmosDiffuse = vec4(0.f);
			CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, normalize(c), uneclipsed);
			vec2 density = calculateDensityBetweenPoints(a, b, texture_LUT);

			vec3 attenuationAC = calculateAtmosphereAttenuation(a, c, texture_LUT);
			vec3 attenuationCB = calculateAtmosphereAttenuation(c, b, texture_LUT);

			// indirect: input and output
			vec3 scattered = calculateDirectLight(sunDirection, c, d, texture_LUT, numSamplesLight);
			vec3 scatteredOutR = scattered * density.x * atmosDiffuse.xyz;
			vec3 scatteredOutM = scattered * density.y * atmosDiffuse.xyz;

			// direct: input and output
			lightR *= attenuationAC;
			lightR += scatteredOutR;
			lightR *= attenuationCB;

			lightM *= attenuationAC;
			lightM += scatteredOutM;
			lightM *= attenuationCB;
		}
	}

	segmentStart = enterShadow;
	segmentFinish = exitShadow;
	if (segmentStart != segmentFinish) {
		float segmentLength = (segmentStart - segmentFinish) / numSamples;
		for (int i = 0; i < numSamples; ++i) {
			vec3 a = rayStart + ((i + 0.f) * segmentLength + segmentFinish) * rayDirection;
			vec3 b = rayStart + ((i + 1.f) * segmentLength + segmentFinish) * rayDirection;

			vec3 c = mix(a, b, 0.5f);
			vec3 d = c + sunDirection * raySphereIntersect(-c, sunDirection, geosphereRadius * geosphereAtmosTopRad).y;

			vec4 atmosDiffuse = vec4(0.f);
			CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, normalize(c), uneclipsed);
			vec2 density = calculateDensityBetweenPoints(a, b, texture_LUT);

			vec3 attenuationAC = calculateAtmosphereAttenuation(a, c, texture_LUT);
			vec3 attenuationCB = calculateAtmosphereAttenuation(c, b, texture_LUT);

			// indirect: input and output
			vec3 scattered = calculateDirectLight(sunDirection, c, d, texture_LUT, numSamplesLight);
			vec3 scatteredOutR = scattered * density.x * atmosDiffuse.xyz;
			vec3 scatteredOutM = scattered * density.y * atmosDiffuse.xyz;

			// direct: input and output
			lightR *= attenuationAC;
			//lightR += scatteredOutR;
			lightR *= attenuationCB;

			lightM *= attenuationAC;
			//lightM += scatteredOutM;
			lightM *= attenuationCB;
		}
	}

	segmentStart = enterAtm;
	segmentFinish = enterShadow;
	if (segmentStart != segmentFinish) {
		float segmentLength = (segmentStart - segmentFinish) / numSamples;
		for (int i = 0; i < numSamples; ++i) {
			vec3 a = rayStart + ((i + 0.f) * segmentLength + segmentFinish) * rayDirection;
			vec3 b = rayStart + ((i + 1.f) * segmentLength + segmentFinish) * rayDirection;

			vec3 c = mix(a, b, 0.5f);
			vec3 d = c + sunDirection * raySphereIntersect(c, sunDirection, geosphereRadius * geosphereAtmosTopRad).y;

			vec4 atmosDiffuse = vec4(0.f);
			CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, normalize(c), uneclipsed);
			vec2 density = calculateDensityBetweenPoints(a, b, texture_LUT);

			vec3 attenuationAC = calculateAtmosphereAttenuation(a, c, texture_LUT);
			vec3 attenuationCB = calculateAtmosphereAttenuation(c, b, texture_LUT);

			// indirect: input and output
			vec3 scattered = calculateDirectLight(sunDirection, c, d, texture_LUT, numSamplesLight);
			vec3 scatteredOutR = scattered * density.x * atmosDiffuse.xyz;
			vec3 scatteredOutM = scattered * density.y * atmosDiffuse.xyz;

			// direct: input and output
			lightR *= attenuationAC;
			lightR += scatteredOutR;
			lightR *= attenuationCB;

			lightM *= attenuationAC;
			lightM += scatteredOutM;
			lightM *= attenuationCB;
		}
	}

	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	vec3 totalR = lightR * betaR * phaseR;
	vec3 totalM = lightM * betaM * phaseM;
	vec3 total = vec3(0.f);
	total += totalR;
	total += totalM;
	return total;
}

vec3 calculateTerrainLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 camera, const in sampler2D texture_LUT, const int samples)
{
	// get terrain intersection
	vec2 sectTerrain = raySphereIntersect(camera, rayDirection, geosphereRadius);
	vec3 rayStart = rayDirection * sectTerrain.x + camera;

	vec2 sectAtm = raySphereIntersect(rayStart, sunDirection, geosphereRadius * geosphereAtmosTopRad);
	vec3 rayFinish = sunDirection * sectAtm.y + rayStart;

	return calculateDirectLight(sunDirection, rayStart, rayFinish, texture_LUT, samples);
}

vec3 calculateTerrainColor(const in vec4 planet, const in vec4 atmosphere, const in vec4 lightColor, const in vec3 lightDir, const in vec3 rayStart, const in vec3 rayDir, const in float uneclipsed, const in sampler2D texture_LUT)
{
	// rayStart is already multiplied by planet radius
	vec3 planetPosition = planet.xyz * planet.w + rayStart;

	vec3 atmospherePosition = atmosphere.xyz * atmosphere.w;

	return calculateTerrainLight(lightDir, rayDir, planetPosition, texture_LUT, 16);
}

vec3 calculateAtmosphereColor(const in vec4 planet, const in vec4 atmosphere, const in vec4 lightColor, const in vec3 lightDir, const in vec3 rayStart, const in vec3 rayDir, const in float uneclipsed, const in sampler2D texture_LUT)
{
    // rayStart is already multiplied by planet radius
    vec3 planetPosition = planet.xyz * planet.w + rayStart;

    vec3 atmospherePosition = atmosphere.xyz * atmosphere.w;

    vec3 lightOld = computeIncidentLight(lightDir, rayDir, -planetPosition, lightColor, uneclipsed, texture_LUT);
    vec3 lightNew = calculateIncidentLight2(lightDir, rayDir, -planetPosition, lightColor, uneclipsed, texture_LUT);

    //return lightOld;
    return lightNew;
}
