// model uniforms
uniform sampler2D tex;
uniform sampler2D texGlow;
uniform bool usetex;
uniform bool useglow;

// Light is split between directly and ambiently lit portions depending on atmosphere.
// For the cases of no shaders and non-atmospheric models this is handled by varying light source properties.
uniform float intensity;
uniform float ambient;

// uniforms needed for atmosphere
uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the planet has been made
uniform float planetScaledRadius;
uniform float planetAtmosTopRad;
uniform vec3 planetCenter;
uniform float planetAtmosFogDensity;
uniform float planetAtmosInvScaleHeight;


varying vec4 varyingEyepos;
varying vec3 varyingNormal;

void main(void)
{

	vec3 tnorm = normalize(varyingNormal);

	// Calculate colour of model surface
	// this is the same as models without atmosphere except fragment colour is changed instead of modulating light source properties
	vec4 amb = vec4(0.0);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);
	vec4 fragCol = vec4(0.0);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float nDotVP = max(0.0001, dot(tnorm, normalize(vec3(gl_LightSource[i].position))));
		float nDotHV = max(0.0001, dot(tnorm, vec3(gl_LightSource[i].halfVector)));
		float pf = max(0.0, pow(nDotHV, gl_FrontMaterial.shininess));
		amb += gl_LightSource[i].ambient;
		diff += gl_LightSource[i].diffuse * nDotVP;
		spec += gl_LightSource[i].specular * pf;
	}


	vec4 emission = gl_FrontMaterial.emission;
	if ( useglow )
		emission = texture2D(texGlow, gl_TexCoord[0].st);

	// Light which is normally directly lit is split between directly and ambiently lit.
	// Ambient and intensity hold the respective fractions.

	// i.e. split up the old directly lit component 
	// between a directly lit part which varies with sun position and an ambiently lit part which is constant
	diff = diff*intensity + vec4(ambient);
	spec = spec*intensity;

	fragCol =
		gl_LightModel.ambient * gl_FrontMaterial.ambient + // global scene ambient colour used to give ambient lighting at close range in the dark
		amb * gl_FrontMaterial.ambient + // amb is normally 0 due to light source ambient being 0
		diff * gl_FrontMaterial.diffuse +
		spec * gl_FrontMaterial.specular +
		emission;

	fragCol.w = gl_FrontMaterial.diffuse.w;

	if ( usetex )
		fragCol *= texture2D(tex, gl_TexCoord[0].st);

	// Calculate the effect of atmosphere in between the model surface and the camera
	// adds a fogging effect with increasing amount of atmosphere (ldprod) in this atmosphere model
	// this is the same as for geosphere terrain except:
	// 	model colour is used instead of terrain colour, 
	// 	there is no scaling needed to keep visible planet surface inside the viewing frustrum
	//	scene ambient/diffuse lighting is moved to the model shading code

	vec3 eyepos = varyingEyepos;
	vec3 eyenorm = normalize(eyepos);

	// when does the eye ray intersect atmosphere
	float atmosStart = findSphereEyeRayEntryDistance(planetCenter, eyepos, planetScaledRadius * planetAtmosTopRad);

	float fogFactor;
	{
		float atmosDist = (length(eyepos) - atmosStart);
		float ldprod;
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (atmosStart * eyenorm - planetCenter) / planetScaledRadius;
		vec3 b = (eyepos - planetCenter) / planetScaledRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.w*planetAtmosFogDensity, atmosDist, planetAtmosInvScaleHeight);
		fogFactor = 1.0 / exp(ldprod);
	}

	vec4 atmosDiffuse = vec4(0.0,0.0,0.0,1.0);
	{
		vec3 surfaceNorm = normalize(atmosStart*eyenorm - planetCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {
			atmosDiffuse += gl_LightSource[i].diffuse * max(0.0, dot(surfaceNorm, normalize(vec3(gl_LightSource[i].position))));
		}
	}
	atmosDiffuse.a = 1.0;

	gl_FragColor = (fogFactor) * (fragCol) +
		(1.0-fogFactor)*(atmosDiffuse*atmosColor);

#ifdef ZHACK
	SetFragDepth(gl_TexCoord[6].z);
#endif
}
