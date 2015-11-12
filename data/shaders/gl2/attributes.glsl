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

attribute vec4 a_vertex;
attribute vec3 a_normal;
attribute vec4 a_color;
attribute vec4 a_uv0;
attribute vec3 a_tangent;
attribute mat4 a_transform;
// shadows 6, 7, and 8
// next available is layout (location = 9) 

#endif // VERTEX_SHADER