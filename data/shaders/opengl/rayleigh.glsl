// get height in m
float height(const in vec3 rayStart)
{
	float height = sqrt(dot(rayStart, rayStart)) - geosphereRadius;

	return height;
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
    return getDensityAtPointNew(rayStart, texture_LUT);
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


vec3 calculateDirectLight(const in vec3 sunDirection, const in vec3 a, const in vec3 b, const in sampler2D texture_LUT, const int samples)
{
	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	float segmentLength = length(b - a) / samples;

	vec2 density = vec2(0.0);
	for (int i = 0; i < samples; ++i) {
		vec3 c = mix(a, b, vec3(i + 0.5) / samples);

		density += segmentLength * getDensityAtPoint(c, texture_LUT);
	}

	return exp(-(betaR * density.x + betaM * density.y));
}

void processLight(const in vec3 a, const in vec3 b, inout vec2 iDensity, inout vec3 color, const in vec3 sunDirection, const in vec3 rayDirection, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT) {
	if (a == b)
		return;

	float mu = dot(rayDirection, sunDirection);
	float phaseR = rayleighPhaseFunction(mu);
	float phaseM = miePhaseFunction(0.76f, mu);
	vec3 betaR = 1e-6 * vec3(3.8, 13.5, 33.1);
	vec3 betaM = 1e-6 * vec3(21.0);

	int iSamples = 16;
	int oSamples = 8;
	float iSegmentLength = length(b - a) / iSamples;

	for (int i = 0; i < iSamples; ++i) {
		if (iSegmentLength == 0.0) {
			break;
		}

		vec3 c = mix(a, b, vec3(i + 0.5) / iSamples);

		vec2 deltaDensity = iSegmentLength * getDensityAtPoint(c, texture_LUT);

		if (raySphereIntersect(-c, sunDirection, geosphereRadius).y > 0.0)
			continue;

		float d_scal = raySphereIntersect(-c, sunDirection, geosphereRadius * geosphereAtmosTopRad).y;
		vec3 d = c + sunDirection * d_scal;

		vec4 atmosDiffuse = vec4(0.f);
		CalcPlanetDiffuse(atmosDiffuse, diffuse, sunDirection, normalize(c), uneclipsed);
		vec3 sunColor = atmosDiffuse.xyz; // incoming color

		// before scattering
		vec3 color0 = sunColor * calculateDirectLight(sunDirection, c, d, texture_LUT, oSamples);

		// during scattering
		vec3 color1R = color0 * betaR * phaseR * deltaDensity.x;
		vec3 color1M = color0 * betaM * phaseM * deltaDensity.y;
		vec3 color1 = color1R + color1M;

		// after scattering
		vec3 color2 = color1 * exp(-(betaR * iDensity.x + betaM * iDensity.y));

		iDensity += deltaDensity;
		color += color2;
	}
}

vec3 calculateIncidentLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 rayStart, const in vec4 diffuse, const in float uneclipsed, const in sampler2D texture_LUT)
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

	processLight(a, b, density, color, sunDirection, rayDirection, diffuse, uneclipsed, texture_LUT);
	processLight(b, c, density, color, sunDirection, rayDirection, diffuse, uneclipsed, texture_LUT);
	processLight(c, d, density, color, sunDirection, rayDirection, diffuse, uneclipsed, texture_LUT);

	return color;
}

vec3 calculateTerrainLight(const in vec3 sunDirection, const in vec3 rayDirection, const in vec3 camera, const in sampler2D texture_LUT, const int samples)
{
	// get terrain intersection
	vec2 sectTerrain = raySphereIntersect(camera, rayDirection, geosphereRadius);
	vec3 rayStart = rayDirection * sectTerrain.x + camera;

	vec2 sectAtm = raySphereIntersect(-rayStart, sunDirection, geosphereRadius * geosphereAtmosTopRad);
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

    return calculateIncidentLight(lightDir, rayDir, -planetPosition, lightColor, uneclipsed, texture_LUT);;
}
