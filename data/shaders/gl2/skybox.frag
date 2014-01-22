uniform samplerCube texture0;

varying vec3 v_texCoord;
varying float v_skyboxFactor;

void main( void )
{
    gl_FragColor = textureCube( texture0, v_texCoord ) * v_skyboxFactor;
    
}