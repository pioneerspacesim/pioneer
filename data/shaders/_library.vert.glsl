
// From http://www.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&reply_id=3513134
vec4 logarithmicTransform() 
{
	vec4 vertexPosClip = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[6] = vertexPosClip;
	return vertexPosClip;
}

/*
vec2 findSphereEyeRayEntryExitDistance(in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	vec2 dists = vec2(0.0);
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0) {
			dists.x = max(i1, 0.0);
			dists.y = i2;
		}
	}
	return dists;
}
*/

float intensityOfOccultedLight(vec3 lightDir, vec3 v, vec3 occultCentre, float srad, float lrad) {
	vec3 projectedPoint = v - dot(lightDir,v)*lightDir;
	// By our assumptions, the proportion of light blocked at this point by
	// this sphere is the proportion of the disc of radius lrad around
	// projectedPoint covered by the disc of radius srad around occultCentre.
	float dist = length(projectedPoint - occultCentre);
	if (dist > srad+lrad)
		return 1.0;
	else if (dist < abs(srad-lrad))
		if (srad > lrad)
			// umbra
			return 0.0;
		else
			// penumbra
			return 1.0 - (srad*srad) / (lrad*lrad);
	else
		// antumbra - just linearly interpolate (TODO:better?)
		return 1.0 - ((srad+lrad-dist)*(
					(srad > lrad) ? 1 : (srad*srad) / (lrad*lrad))) /
			(srad+lrad-abs(srad-lrad));
}
