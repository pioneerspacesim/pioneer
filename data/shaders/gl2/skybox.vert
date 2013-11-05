uniform vec4 u_viewPosition;

varying vec3 v_texCoord;

void main( void )
{
    vec3 position = gl_Vertex.xyz;
    position += u_viewPosition.xyz;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
    v_texCoord    = gl_Vertex.xyz;    
}