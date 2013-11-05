uniform samplerCube texture0;

varying vec3 v_texCoord;

void main( void )
{
    gl_FragColor = textureCube( texture0, v_texCoord );
    
}