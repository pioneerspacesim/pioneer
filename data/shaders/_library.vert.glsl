varying float varLogDepth;

// From http://www.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&reply_id=3513134
vec4 logarithmicTransform()
{
	vec4 vertexPosClip = gl_ModelViewProjectionMatrix * gl_Vertex;
	varLogDepth = vertexPosClip.z;
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
