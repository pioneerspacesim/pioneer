varying vec3 varyingEyepos;
varying vec3 varyingNormal;
varying vec4 vertexColor;

uniform vec3 geosphereCenter;
uniform float geosphereScaledRadius;

void main(void)
{
	gl_Position = logarithmicTransform();
	vertexColor = gl_Color;
	varyingEyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	varyingNormal = gl_NormalMatrix * gl_Normal;
}
