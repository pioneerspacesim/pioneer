void main(void)
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	float zpos_mv = dot(gl_Vertex, gl_ModelViewMatrix[2]);
	gl_PointSize = gl_Point.size *
		inversesqrt(zpos_mv * zpos_mv * gl_Point.distanceQuadraticAttenuation);
#ifdef ZHACK
	gl_Position = logarithmicTransform();
#else
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#endif
}

