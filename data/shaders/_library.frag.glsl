
uniform float invLogZfarPlus1;

void SetFragDepth(float z)
{
  	gl_FragDepth = gl_DepthRange.near + (gl_DepthRange.far * log(z + 1.0) * invLogZfarPlus1);
}

