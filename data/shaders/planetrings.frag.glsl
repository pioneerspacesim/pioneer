uniform sampler2D texture0;

void main(void)
{
	// Bits of ring in shadow!
	vec4 col = vec4(0.0);
	vec4 texCol = texture2D(texture0, gl_TexCoord[0].st);

	for (int i=0; i<NUM_LIGHTS; ++i) {
		float l = findSphereEyeRayEntryDistance(-vec3(gl_TexCoord[1]), vec3(gl_ModelViewMatrixInverse * gl_LightSource[i].position), 1.0);
		if (l <= 0.0) {
			col = col + texCol*gl_LightSource[i].diffuse;
		}
	}
	col.a = texCol.a;
	gl_FragColor = col;

	SetFragDepth();
}
