int _iSamples = 16;
int _oSamples = 8;

// get density at given point
vec2 getDensityAtPoint(const in vec3 rayStart, const in sampler2D densityLUT)
{
	float height = length(rayStart) - geosphereRadius; // in meters
	float maxHeight = (geosphereAtmosTopRad - 1.f) * geosphereRadius;

	float ratio = height / maxHeight;

	return exp(texture(densityLUT, vec2(0.0, ratio)).xy);
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

vec3 directLight2dLUT(const in vec3 a, const in vec3 b, const in sampler2D scatterLUT) {
	float height = length(a) - geosphereRadius;
	float maxHeight = geosphereAtmosTopRad * geosphereRadius - geosphereRadius;

	float height_scaled = height / maxHeight;

	float cos_phi = dot(normalize(b - a), normalize(a));
	vec2 density = exp(texture(scatterLUT, vec2(acos(cos_phi) / PI, height_scaled)).xy);

	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	return exp(-(betaR * density.x + betaM * density.y));
}


vec3 directLight1dLUT(const in vec3 a, const in vec3 b, const in sampler2D densityLUT, const int samples) {

	float segmentLength = length(b - a) / samples;

	vec2 density = vec2(0.f);
	for (int i = 0; i < samples; ++i) {
		vec3 c = mix(a, b, vec3(i + 0.5) / samples);

		density += segmentLength * getDensityAtPoint(c, densityLUT);
	}

	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	return exp(-(betaR * density.x + betaM * density.y));
}


vec3 calculateDirectLight(const in vec3 a, const in vec3 b, const in sampler2D densityLUT, const int samples, const in sampler2D scatterLUT)
{
    //return directLight1dLUT(a, b, densityLUT, samples);
    return directLight2dLUT(a, b, scatterLUT);
}

void processLight(const in vec3 a, const in vec3 b, inout vec2 iDensity, inout vec3 color, const in vec3 sunDirection, const in vec3 rayDirection, const in vec4 diffuse, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT) {
	if (a == b)
		return;

	float mu = dot(rayDirection, sunDirection);
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);
	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	int iSamples = _iSamples;
	int oSamples = _oSamples;
	float iSegmentLength = length(b - a) / iSamples;
	if (iSegmentLength == 0.0) {
		return;
	}

	for (int i = 0; i < iSamples; ++i) {
		vec3 c = mix(a, b, vec3(i + 0.5f) / iSamples);

		vec2 deltaDensity = iSegmentLength * getDensityAtPoint(c, densityLUT);

		vec2 intersect = raySphereIntersect(-c, sunDirection, geosphereRadius);

		float d_scal = raySphereIntersect(-c, sunDirection, geosphereRadius * geosphereAtmosTopRad).y;
		vec3 d = c + sunDirection * d_scal;

		// before scattering
		vec3 color0 = diffuse.xyz * calculateDirectLight(c, d, densityLUT, oSamples, scatterLUT);

		// during scattering
		vec3 color1R = color0 * betaR * phaseR * deltaDensity.x;
		vec3 color1M = color0 * betaM * phaseM * deltaDensity.y;
		vec3 color1 = color1R + color1M;

		// after scattering
		vec3 color2 = color1 * exp(-(betaR * iDensity.x + betaM * iDensity.y));

		iDensity += deltaDensity;
		color += (intersect.y > 0.f) ? vec3(0.f) : color2;
	}
}

vec3 calculateIncidentLightAB(const in vec3 sunDirection, const in vec3 rayStart, const in vec3 rayEnd, const in vec4 diffuse, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
    vec3 rayDirection = normalize(rayEnd - rayStart);
    float rayLength   = length(rayEnd - rayStart);

	vec4 segment = getRaySegment(sunDirection, rayDirection, rayStart);

	/*
	 * rayStart -> enterAtm -> enterShadow -> exitShadow -> exitAtm -> rayFinish
	 */
	float enterAtm    = min(rayLength, segment.x);
	float enterShadow = min(rayLength, segment.y);
	float exitShadow  = min(rayLength, segment.z);
	float exitAtm     = min(rayLength, segment.w);

	vec3 a = rayStart + rayDirection * enterAtm;
	vec3 b = rayStart + rayDirection * enterShadow;
	vec3 c = rayStart + rayDirection * exitShadow;
	vec3 d = rayStart + rayDirection * exitAtm;

	vec2 density = vec2(0.0);
	vec3 color = vec3(0.f);

	processLight(a, b, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);
	processLight(b, c, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);
	processLight(c, d, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);

	return color;
}

vec3 calculateIncidentLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
	vec4 segment = getRaySegment(sunDirection, rayDirection, rayStart);

	/*
	 * rayStart -> enterAtm -> enterShadow -> exitShadow -> exitAtm -> rayFinish
	 */
	float enterAtm    = segment.x;
	float enterShadow = segment.y;
	float exitShadow  = segment.z;
	float exitAtm     = segment.w;

	vec3 a = rayStart + rayDirection * enterAtm;
	vec3 b = rayStart + rayDirection * enterShadow;
	vec3 c = rayStart + rayDirection * exitShadow;
	vec3 d = rayStart + rayDirection * exitAtm;

	vec2 density = vec2(0.0);
	vec3 color = vec3(0.f);

	processLight(a, b, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);
	processLight(b, c, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);
	processLight(c, d, density, color, sunDirection, rayDirection, diffuse, uneclipsed, densityLUT, scatterLUT);

	return color;
}

vec3 calculateTerrainLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 camera, const in sampler2D densityLUT, const int samples, const in sampler2D scatterLUT)
{
	// get terrain intersection
	vec2 sectTerrain = raySphereIntersect(camera, rayDirection, geosphereRadius);
	vec3 rayStart = rayDirection * sectTerrain.x + camera;

	vec2 sectAtm = raySphereIntersect(-rayStart, sunDirection, geosphereRadius * geosphereAtmosTopRad);
	vec3 rayFinish = sunDirection * sectAtm.y + rayStart;

	vec3 light = calculateDirectLight(rayStart, rayFinish, densityLUT, samples, scatterLUT);

	return light;
}

vec3 calculateTerrainColor(const in vec4 planet, const in vec4 atmosphere, const in vec4 lightColor, const in vec3 lightDir, const in vec3 rayStart, const in vec3 rayDir, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
	// rayStart is already multiplied by planet radius
	vec3 planetPosition = planet.xyz * planet.w + rayStart;

	vec3 atmospherePosition = atmosphere.xyz * atmosphere.w;

	vec3 light = calculateTerrainLight(lightDir, rayDir, planetPosition, densityLUT, _iSamples, scatterLUT);

	return light;
}

vec3 calculateAtmosphereColor(const in vec4 planet, const in vec4 atmosphere, const in vec4 lightColor, const in vec3 lightDir, const in vec3 rayStart, const in vec3 rayDir, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
    // rayStart is already multiplied by planet radius
    vec3 planetPosition = planet.xyz * planet.w + rayStart;

    vec3 atmospherePosition = atmosphere.xyz * atmosphere.w;

    return calculateIncidentLight(lightDir, rayDir, -planetPosition, lightColor, uneclipsed, densityLUT, scatterLUT);
}

vec3 calculateAtmosphereColorAB(const in vec4 planet, const in vec4 atmosphere, const in vec3 rayStart, const in vec3 rayEnd, const in vec4 lightColor, const in vec3 lightDir, const in float uneclipsed, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
    // rayStart is already multiplied by planet radius
    vec3 planetPositionStart = planet.xyz * planet.w + rayStart;
    vec3 planetPositionEnd = planet.xyz * planet.w + rayEnd;

    vec3 atmospherePosition = atmosphere.xyz * atmosphere.w;
    return calculateIncidentLightAB(lightDir, -planetPositionStart, -planetPositionEnd, lightColor, uneclipsed, densityLUT, scatterLUT);
}

// Compute the per-wavelength (RGB) optical transmittance of an atmospheric
// medium over the given optical depth. This is a convenience function.
vec3 calcTransmittance(const in vec2 opticalDepth)
{
	const vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	const vec3 betaM = 1e-6 * vec3(21.0);

	return exp(-(betaR * opticalDepth.x + betaM * opticalDepth.y));
}

// Compute the atmospheric transmittance along a ray from a point within the atmosphere to a point on the edge of the atmosphere.
// NOTE: rayStart is relative to geosphereCenter
// NOTE: rayDir points "out" of the atmosphere
vec3 calculateRayTransmittance(const in vec3 rayStart, const in vec3 rayDir, const in sampler2D scatterLUT)
{
	// Compute the height of the sampled point in the atmosphere in 0..1
	float height_scaled = clamp((length(rayStart) - 1) / (geosphereAtmosTopRad - 1), 0, 1);

	// To compute the density of the atmosphere along the ray, we need to know
	// the angle of the ray relative to vertical.
	float cos_phi = dot(rayDir, normalize(rayStart)); //* 0.5 + 0.5;
	float alpha = acos(cos_phi) / PI;

	// Density is stored to the texture in log scale
	vec2 density = exp(texture(scatterLUT, vec2(alpha, height_scaled)).xy);

	// Compute the transmittance factor for the three color wavelengths we are rendering.
	return calcTransmittance(density);
}

// Compute the in-scattering of light along a ray from a point in the atmosphere
// until its exit. RayDir points in the direction the light being in-scattered will travel.
//
// The parameterization is backwards because it is much easier to express a
// *surface shader* in terms of the surface being shaded.
vec3 calculateRayInscattering(inout vec2 opticalDepth, const in vec3 rayStart, const in vec3 rayDir, const in float rayLength, const in vec3 lightDir, const in vec3 lightColor, const in sampler2D densityLUT, const in sampler2D scatterLUT)
{
	vec3 color = vec3(0);

	vec2 atmos = raySphereIntersect(-rayStart, rayDir, geosphereAtmosTopRad);
	if (atmos.y == 0)
		return color; // early-out if we're not in an atmosphere

	float atmosDist = min(atmos.y, rayLength);
	vec3 atmosExit = rayStart + rayDir * atmosDist;

	// This phase function uses the convention that the ray points from the
	// camera to the surface so we need to invert the ray dir here.
	float mu = dot(-rayDir, lightDir);
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);
	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	float iSegmentLength = geosphereRadius * atmosDist / _iSamples;
	if (iSegmentLength == 0.0) {
		return color;
	}

	for (int i = 0; i < _iSamples; ++i) {
		// Trace backwards from the exit to the start of the ray to accumulate
		// optical depth over the sample ray.
		vec3 c = mix(atmosExit, rayStart, vec3(i + 0.5f) / _iSamples);

		float heightRatio = (length(c) - 1) / (geosphereAtmosTopRad - 1);
		vec2 avgDensity = exp(texture(densityLUT, vec2(0.0, heightRatio)).xy);

		vec2 deltaDensity = iSegmentLength * avgDensity;

		// find if this ray intersects the terrain
		vec2 intersect = raySphereIntersect(-c, lightDir, 1);

		// outscattering of light reaching this point
		vec3 color0 = lightColor * calculateRayTransmittance(c, lightDir, scatterLUT);

		// evaluate the phase function to find the wavelength-dependent
		// proportion of light being scattered in the camera direction
		vec3 color1R = color0 * betaR * phaseR * deltaDensity.x;
		vec3 color1M = color0 * betaM * phaseM * deltaDensity.y;
		vec3 color1 = color1R + color1M;

		// outscattering of this new light on the way to the camera
		vec3 color2 = color1 * calcTransmittance(opticalDepth);

		opticalDepth += deltaDensity;
		color += step(0, intersect.y) * color2;
	}

	return color;
}
