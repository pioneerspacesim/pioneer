uniform float invLogZfarPlus1;

// makes depth value = log(C*z + 1) / log(C*zfar + 1)
vec4 logarithmicTransform() 
{
	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
	pos.z = log2(pos.z + 1.0) * invLogZfarPlus1 * pos.w;
	return pos;
}

float findSphereEyeRayEntryDistance(in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	float entryDist = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0) {
			entryDist = max(i1, 0.0);
		}
	}
	return entryDist;
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
