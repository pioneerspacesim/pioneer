// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#extension GL_ARB_explicit_attrib_location : enable

//Light uniform parameters
struct Light {
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};

#ifdef UNIFORM_BUFFERS

struct Material {
	vec4 diffuse;
	vec3 specular;
	float shininess;
	vec4 emission;
};

struct Scene {
	vec4 lightIntensity;
	vec4 ambient;
};

layout(std140) uniform LightData {
	Light uLight[4];
};

layout(std140) uniform DrawData {
	Material material;
	Scene scene;

	mat4 uViewMatrix;
	mat4 uViewMatrixInverse;
	mat4 uViewProjectionMatrix;
};

#else

uniform mat4 uViewMatrix;
uniform mat4 uViewMatrixInverse;
uniform mat4 uViewProjectionMatrix;
uniform mat3 uNormalMatrix;

//scene uniform parameters
struct Scene {
	vec4 ambient;
};

struct Material {
	vec4 emission;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform Light uLight[4];

#endif

#ifdef VERTEX_SHADER

layout (location = 0) in vec4 a_vertex;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_color;
layout (location = 3) in vec4 a_uv0;
layout (location = 4) in vec4 a_uv1;
layout (location = 5) in vec3 a_tangent;
layout (location = 6) in mat4 a_transform;
// a_transform @ 6 shadows (uses) 7, 8, and 9
// next available is layout (location = 10)

// shorthand to abstract away instancing
vec4 matrixTransform()
{
#ifdef USE_INSTANCING
	return uViewProjectionMatrix * a_transform * a_vertex;
#else
	return uViewProjectionMatrix * a_vertex;
#endif
}

#endif // VERTEX_SHADER
