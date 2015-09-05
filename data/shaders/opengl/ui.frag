// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

uniform sampler2D texture0; //diffuse
in vec2 texCoord0;

in vec4 vertexColor;

uniform Scene scene;
uniform Material material;

out vec4 frag_color;

void main(void)
{
	frag_color = vertexColor * texture(texture0, texCoord0).x;
	
	SetFragDepth();
}
