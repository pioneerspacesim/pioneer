uniform samplerCube s_cubeMap;

varying vec3 v_texCoord;

void main( void )
{
    gl_FragColor = textureCube( s_cubeMap, v_texCoord );
    
}