// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "attributes.glsl"
#include "logz.glsl"
#include "lib.glsl"

#ifdef TEXTURE0
uniform sampler2D texture0; //diffuse
uniform sampler2D texture1; //specular
uniform sampler2D texture2; //glow
uniform sampler2D texture3; //ambient
uniform sampler2D texture4; //pattern
uniform sampler2D texture5; //color
uniform sampler2D texture6; //normal
varying vec2 texCoord0;
#endif

#ifdef VERTEXCOLOR
varying vec4 vertexColor;
#endif
#if (NUM_LIGHTS > 0)
varying vec3 eyePos;
varying vec3 normal;
#ifdef MAP_NORMAL
	varying vec3 tangent;
	varying vec3 bitangent;
#endif
	#ifdef HEAT_COLOURING
		uniform sampler2D heatGradient;
		uniform float heatingAmount; // 0.0 to 1.0 used for `u` component of heatGradient texture
		varying vec3 heatingDir;
	#endif // HEAT_COLOURING
#endif // (NUM_LIGHTS > 0)

uniform Scene scene;
uniform Material material;

#if (NUM_LIGHTS > 0)
//ambient, diffuse, specular
//would be a good idea to make specular optional
void ads(in int lightNum, in vec3 pos, in vec3 n, inout vec4 light, inout vec4 specular)
{
	vec3 s = normalize(vec3(uLight[lightNum].position)); //directional light
	vec3 v = normalize(vec3(-pos));
	vec3 h = normalize(v + s);
	light += uLight[lightNum].diffuse * material.diffuse * max(dot(s, n), 0.0);
#ifdef MAP_SPECULAR
	specular += texture2D(texture1, texCoord0) * material.specular * uLight[lightNum].specular * pow(max(dot(h, n), 0.0), material.shininess);
#else
	specular += material.specular * uLight[lightNum].specular * pow(max(dot(h, n), 0.0), material.shininess);
#endif
	specular.a = 0.0;
	light.a = 1.0;
}
#endif

void main(void)
{
#ifdef VERTEXCOLOR
	vec4 color = vertexColor;
#else
	vec4 color = material.diffuse;
#endif
#ifdef TEXTURE0
	color *= texture2D(texture0, texCoord0);
#endif
//patterns - simple lookup
#ifdef MAP_COLOR
	vec4 pat = texture2D(texture4, texCoord0);
	vec4 mapColor = texture2D(texture5, vec2(pat.r, 0.0));
	vec4 tint = mix(vec4(1.0),mapColor,pat.a);
	color *= tint;
#endif

#ifdef ALPHA_TEST
	if (color.a < 0.5)
		discard;
#endif

//directional lighting
#if (NUM_LIGHTS > 0)
#ifdef MAP_NORMAL
	vec3 bump = (texture2D(texture6, texCoord0).xyz * 2.0) - vec3(1.0);
	mat3 tangentFrame = mat3(tangent, bitangent, normal);
	vec3 vNormal = tangentFrame * bump;
#else
	vec3 vNormal = normal;
#endif
	//ambient only make sense with lighting
	vec4 light = scene.ambient;
	vec4 specular = vec4(0.0);
	for (int i=0; i<NUM_LIGHTS; ++i) {
		ads(i, eyePos, vNormal, light, specular);
	}

#ifdef MAP_AMBIENT
	// this is crude "baked ambient occulsion" - basically multiply everything by the ambient texture
	// scaling whatever we've decided the lighting contribution is by 0.0 to 1.0 to account for sheltered/hidden surfaces
	light *= texture2D(texture3, texCoord0);
#endif

	//emissive only make sense with lighting
#ifdef MAP_EMISSIVE
	light += texture2D(texture2, texCoord0); //glow map
#else
	light += material.emission; //just emissive parameter
#endif
#endif //NUM_LIGHTS

#if (NUM_LIGHTS > 0)
	#ifdef HEAT_COLOURING
		if (heatingAmount > 0.0)
		{
			float dphNn = clamp(dot(heatingDir, vNormal), 0.0, 1.0);
			float heatDot = heatingAmount * (dphNn * dphNn * dphNn);
			vec4 heatColour = texture2D(heatGradient, vec2(heatDot, 0.5)); //heat gradient blend
			gl_FragColor = color * light + specular;
			gl_FragColor.rgb = gl_FragColor.rgb + heatColour.rgb;
		}
		else
		{
			gl_FragColor = color * light + specular;
		}
	#else
		gl_FragColor = color * light + specular;
	#endif // HEAT_COLOURING
#else
	gl_FragColor = color;
#endif
	SetFragDepth();
}
