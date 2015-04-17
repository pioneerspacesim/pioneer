// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#extension GL_ARB_explicit_attrib_location : enable

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uViewMatrixInverse;
uniform mat4 uViewProjectionMatrix;
uniform mat3 uNormalMatrix;

//Light uniform parameters
struct Light {
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};
uniform Light uLight[4];

struct Material {
	vec4 emission;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

#ifdef VERTEX_SHADER

layout (location = 0) in vec4 a_vertex;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_color;
layout (location = 3) in vec4 a_uv0;
layout (location = 4) in mat4 a_transform;
// shadows 5, 6, and 7
// next available is layout (location = 8) 

#endif // VERTEX_SHADER