varying vec3 norm;
varying vec2 texCoord;
varying vec3 ecPosition3;
#ifdef TEXTURE
uniform sampler2D texture0;
#endif
#ifdef GLOWMAP
uniform sampler2D texture1;
#endif

uniform Scene scene;

#define APPLY_LIGHT(i) \
	/* direction of maximum highlights */							\
	/* Compute vector from surface to light position */				\
	VP = vec3(gl_LightSource[i].position) - ecPosition3;			\
	/* Compute distance between surface and light position */		\
	d = length(VP);													\
	/* Normalize the vector from surface to light position */		\
	VP = normalize(VP);												\
	/* Compute attenuation */										\
	attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +	\
		gl_LightSource[i].linearAttenuation * d +					\
		gl_LightSource[i].quadraticAttenuation * d * d);			\
	halfVector = normalize(VP + eye);								\
	nDotVP = max(0.0, dot(normal, VP));							\
	nDotHV = max(0.0, dot(normal, halfVector));					\
	if (nDotVP == 0.0) pf = 0.0;									\
	else pf = pow(nDotHV, gl_FrontMaterial.shininess);				\
	diff += gl_LightSource[i].diffuse * nDotVP * attenuation;		\
	spec += gl_LightSource[i].specular * pf * attenuation;

void main(void)
{
	vec3 eye = vec3(0.0, 0.0, 0.0);//gl_TexCoord[2]); XXX what
	vec3 normal = normalize(norm);
	vec4 diff = vec4(0.0);
	vec4 spec = vec4(0.0);

	// XXX this old hack is likely not necessary any more but I don't feel
	// like changing it right now. Will rewrite the lighting anyway.
	{ // unrolled loop to work around nvidia driver error

		// this is bog standard point light source shading
		float nDotVP;
		// normal . light direction
		float nDotHV;
		// normal . light half vector
		float pf;
		// power factor
		float attenuation;
		// computed attenuation factor
		float d;
		// distance from surface to light source
		vec3 VP;
		// direction from surface to light position
		vec3 halfVector;

		#if (NUM_LIGHTS > 0)
		APPLY_LIGHT(4)
		#endif
		#if (NUM_LIGHTS > 1)
		APPLY_LIGHT(5)
		#endif
		#if (NUM_LIGHTS > 2)
		APPLY_LIGHT(6)
		#endif
		#if (NUM_LIGHTS > 3)
		APPLY_LIGHT(7)
		#endif
	}

#ifdef GLOWMAP
	vec4 emission = texture2D(texture1, texCoord);
#else
	vec4 emission = gl_FrontMaterial.emission;
#endif

	gl_FragColor =
		(scene.ambient * gl_FrontMaterial.ambient) +
		(diff * gl_FrontMaterial.diffuse) +
		(spec * gl_FrontMaterial.specular) +
		emission;
	gl_FragColor.w = gl_FrontMaterial.diffuse.w;

#ifdef TEXTURE
	gl_FragColor *= texture2D(texture0, texCoord);
#endif

	SetFragDepth();
}
