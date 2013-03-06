uniform vec4 atmosColor;
// to keep distances sane we do a nearer, smaller scam. this is how many times
// smaller the geosphere has been made
uniform float geosphereScale;
uniform float geosphereScaledRadius;
uniform float geosphereAtmosTopRad;
uniform vec3 geosphereCenter;
uniform float geosphereAtmosFogDensity;
uniform float geosphereAtmosInvScaleHeight;

varying vec4 varyingEyepos;

void sphereEntryExitDist(out float near, out float far, in vec3 sphereCenter, in vec3 eyeTo, in float radius)
{
	vec3 v = -sphereCenter;
	vec3 dir = normalize(eyeTo);
	float b = -dot(v, dir);
	float det = (b * b) - dot(v, v) + (radius * radius);
	near = 0.0;
	far = 0.0;
	if (det > 0.0) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0.0) {
			near = max(i1, 0.0);
			far = i2;
		}
	}
}

void main(void)
{
	float skyNear, skyFar;
	vec3 eyepos = vec3(varyingEyepos);
	vec3 eyenorm = normalize(eyepos);
	float specularHighlight=0.0;

	sphereEntryExitDist(skyNear, skyFar, geosphereCenter, eyepos, geosphereScaledRadius * geosphereAtmosTopRad);
	float atmosDist = geosphereScale * (skyFar - skyNear);
	float ldprod=0.0;
	{
		vec3 dir = eyenorm;
		// a&b scaled so length of 1.0 means planet surface.
		vec3 a = (skyNear * dir - geosphereCenter) / geosphereScaledRadius;
		vec3 b = (skyFar * dir - geosphereCenter) / geosphereScaledRadius;
		ldprod = AtmosLengthDensityProduct(a, b, atmosColor.a * geosphereAtmosFogDensity, atmosDist, geosphereAtmosInvScaleHeight);
	}
	float fogFactor = 1.0 / exp(ldprod);
	vec4 atmosDiffuse = vec4(0.0);
	{
		vec3 surfaceNorm = normalize(skyNear * eyenorm - geosphereCenter);
		for (int i=0; i<NUM_LIGHTS; ++i) {

			//Calculate Specular Highlight
			vec3 L = normalize(gl_LightSource[i].position.xyz - eyepos); 
			vec3 E = normalize(-eyepos);
			vec3 R = normalize(-reflect(L,vec3(0.0))); 
			specularHighlight += pow(max(dot(R,E),0.0),64.0)/float(NUM_LIGHTS);

			float nDotVP =  max(0.0, dot(surfaceNorm, normalize(vec3(gl_LightSource[i].position))))	;
			float nnDotVP = max(0.0, dot(surfaceNorm, normalize(-vec3(gl_LightSource[i].position))));  //need backlight to increase horizon
			atmosDiffuse +=   gl_LightSource[i].diffuse * 0.5*(nDotVP+0.5*clamp(1.0-nnDotVP*4.0,0.0,1.0)/float(NUM_LIGHTS));

			
		}
	}

	//calculate sunset tone red when passing through more atmosphere, clamp everything.
	float atmpower = (atmosDiffuse.r+atmosDiffuse.g+atmosDiffuse.b)/3.0;
	vec4 sunset = vec4(0.8,clamp(pow(atmpower,0.8),0.0,1.0),clamp(pow(atmpower,1.2),0.0,1.0),1.0);

	atmosDiffuse.a = 1.0;
	gl_FragColor = (1.0-fogFactor) * (atmosDiffuse*
		vec4(atmosColor.rgb, 1.0)) +
		(0.02-clamp(fogFactor,0.0,0.01))*atmosDiffuse*ldprod*sunset +     //increase light on lower atmosphere.
		atmosColor*specularHighlight*(1.0-fogFactor)*sunset;		  //add light from specularHighlight.

	SetFragDepth();
}
