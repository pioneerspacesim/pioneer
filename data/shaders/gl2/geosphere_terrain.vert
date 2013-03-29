varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;
varying vec4 unshadowed;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

uniform int shadows;
uniform ivec3 occultedLight;
uniform mat3 shadowCentre;
uniform vec3 srad;
uniform vec3 lrad;

#ifdef TERRAIN_WITH_LAVA
varying vec4 varyingEmission;
uniform Material material;
#endif

#define PI 3.141592653589793

//#define USE_GOOD_MATHS 1

float discCovered(float dist, float rad) {
	// proportion of unit disc covered by a second disc of radius rad placed
	// dist from centre of first disc.
	//
	// XXX: same function is in Camera.cpp
	//
	// WLOG, the second disc is displaced horizontally to the right.
	// xl = rightwards distance to intersection of the two circles.
	// xs = leftwards distance from centre of second disc to intersection.
	// d = vertical distance to an intersection point
	//
	// The clamps on xl,xs handle the cases where one disc contains the other.

	float radsq = rad*rad;
#ifdef USE_GOOD_MATHS
	float xl = clamp((dist*dist + 1.0 - radsq) / (2.0*dist), -1.0, 1.0);
	float xs = clamp(dist - xl, -rad, rad);

	// XXX: having 1.001 rather that 1.0 in the following appears necessary to
	// avoid flicker due (I'm assuming) to sqrting negative numbers
	float d = sqrt(1.001 - xl*xl);

	float th = clamp(acos(xl), 0.0, PI);
	float th2 = clamp(acos(xs/rad), 0.0, PI);

	// covered area can be calculated as the sum of segments from the two
	// discs plus/minus some triangles, and it works out as follows:
	return clamp((th + radsq*th2 - dist*d)/PI, 0.0, 1.0);
#else
	// linear interpolation version: faster but visibly less accurate
	float maxOcclusion = min(1.0, radsq);
	return mix(0.0, maxOcclusion,
			clamp(
				( rad+1.0-dist ) / ( rad+1.0 - abs(rad-1.0) ),
				0.0, 1.0));
#endif	
}

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = gl_Color;
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;

	// set unshadowed[i] to the effective intensity of light i:
	vec3 v = (varyingEyepos - geosphereCenter)/geosphereScaledRadius;
	float lenInvSq = 1.0/(dot(v,v));
	for (int i=0; i<NUM_LIGHTS; i++) {
		vec3 lightDir = normalize(vec3(gl_LightSource[i].position));
		unshadowed[i] = 1.0;
		for (int j=0; j<shadows; j++)
			if (i == occultedLight[j]) {
				// Apply eclipse:
				vec3 projectedPoint = v - dot(lightDir,v)*lightDir;
				// By our assumptions, the proportion of light blocked at this point by
				// this sphere is the proportion of the disc of radius lrad around
				// projectedPoint covered by the disc of radius srad around shadowCentre.
				float dist = length(projectedPoint - shadowCentre[j]);
				unshadowed[i] *= 1.0 - discCovered(dist/lrad[j], srad[j]/lrad[j]);
			}
	}

#ifdef TERRAIN_WITH_LAVA
	varyingEmission = material.emission;
	//Glow lava terrains
	if ( vertexColor.r > 0.4 && vertexColor.g < 0.2 && vertexColor.b < 0.4 ) {
		varyingEmission = 3.0*vertexColor;
		varyingEmission *= (vertexColor.r+vertexColor.g+vertexColor.b);

	}
#endif
}
