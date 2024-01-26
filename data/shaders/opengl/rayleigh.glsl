float height(const in vec3 orig, const in vec3 center)
{
	vec3 r = orig - center;
	float height = sqrt(dot(r, r)) - geosphereRadius;

	return height;
}

void scatter(out float hr, out float hm, const in vec3 orig, const in vec3 center)
{
	float Hr = 7994;
	float Hm = 1200;

	float height = height(orig, center);

	hr = -height / Hr;
	hm = -height / Hm;
}

void findClosestHeight(out float h, out float t, const in vec3 orig, const in vec3 dir, const in vec3 center)
{
	vec3 radiusVector = center - orig;
	vec3 tangent = dot(dir, radiusVector) * dir;
	vec3 normal = radiusVector - tangent;
	h = sqrt(dot(normal, normal));
	t = dot(tangent, dir);
}

// error function
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

	if (height < 0)
		return k;

	return k * exp(-height * b);
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

vec3 computeIncidentLight(const in vec3 sunDirection, const in vec3 dir, const in vec3 center, const in vec2 atmosDist)
{
	vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	vec3 betaM = vec3(21e-6f);

	float earthRadius = geosphereRadius,
	      atmosphereRadius = geosphereRadius * geosphereAtmosTopRad,
	      atmosphereHeight = atmosphereRadius - earthRadius;

	float tmin = atmosDist.x * geosphereRadius;
	float tmax = atmosDist.y * geosphereRadius;

	int numSamples = 16;
	float segmentLength = (tmax - tmin) / numSamples;
	float tCurrent = tmin;
	vec3 sumR = vec3(0.0);
	vec3 sumM = vec3(0.0); // mie and rayleigh contribution
	float opticalDepthR = 0, opticalDepthM = 0;
	float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
	float phaseR = 3.f / (16.f * 3.141592) * (1 + mu * mu);
	float g = 0.76f;
	float phaseM = 3.f / (8.f * 3.141592) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));

	float kR, bR, cR;
	float kM, bM, cM;

	kR = coefficientsR.x;
	bR = coefficientsR.y;
	cR = coefficientsR.z;

	kM = coefficientsM.x;
	bM = coefficientsM.y;
	cM = coefficientsM.z;

	for (int i = 0; i < numSamples; ++i) {
		vec3 samplePosition = vec3(tCurrent + segmentLength * 0.5f) * dir;
		float hr, hm;
		scatter(hr, hm, samplePosition, center);
		opticalDepthR += exp(hr) * segmentLength;
		opticalDepthM += exp(hm) * segmentLength;

		// light optical depth
		float opticalDepthLightR = 0, opticalDepthLightM = 0;
		vec3 samplePositionLight = samplePosition;

		float hl, tl;
		findClosestHeight(hl, tl, samplePositionLight, sunDirection, center);
		hl -= earthRadius;

		// find out-scattering density
		opticalDepthLightR = predictDensityOut(atmosphereHeight, hl, kR, bR);
		opticalDepthLightM = predictDensityOut(atmosphereHeight, hl, kM, bM);

		// apply in-scattering filter
		opticalDepthLightR *= predictDensityIn(earthRadius, atmosphereHeight, hl, cR, tl);
		opticalDepthLightM *= predictDensityIn(earthRadius, atmosphereHeight, hl, cM, tl);

		vec3 tau = -(betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM));
		vec3 tauR = tau + vec3(hr);
		vec3 tauM = tau + vec3(hm);
		vec3 attenuationR = exp(tauR) * segmentLength;
		vec3 attenuationM = exp(tauM) * segmentLength;
		sumR += attenuationR;
		sumM += attenuationM;
		tCurrent += segmentLength;
	}

	vec3 ret = (sumR * betaR * phaseR + sumM * betaM * phaseM);
	return ret;
}
