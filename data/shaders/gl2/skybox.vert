uniform vec4 u_viewPosition;
uniform gl_MaterialParameters material;

varying vec3 v_texCoord;
varying float v_skyboxFactor;

void main( void )
{
    vec3 position = gl_Vertex.xyz;
    position += u_viewPosition.xyz;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
    v_texCoord    = gl_Vertex.xyz;    
	v_skyboxFactor = material.shininess;
}