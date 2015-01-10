// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

in vec4 color;
in vec2 uv;
in vec3 lightDir;

uniform Scene scene;

out vec4 frag_color;

void main(void)
{
	float len = dot(uv, uv);
	if (len > 1.0)
		discard;
	vec3 normal = vec3(uv.x, uv.y, sqrt(1.0 - len));
	float diff = dot(normal, lightDir);
	frag_color = color * diff + scene.ambient;
	SetFragDepth();
}
