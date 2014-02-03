// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

varying vec4 color;
varying vec2 uv;
varying vec3 lightDir;

uniform Scene scene;

void main(void)
{
	float len = dot(uv, uv);
	if (len > 1.0)
		discard;
	vec3 normal = vec3(uv.x, uv.y, sqrt(1.0 - len));
	float diff = dot(normal, lightDir);
	gl_FragColor = color * diff + scene.ambient;
	SetFragDepth();
}
