varying vec4 color;

uniform Material material;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_PointSize = 1.0 + pow(gl_Color.r,3.0);
	color = gl_Color * material.emission;
}
