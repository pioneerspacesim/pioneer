out vec2 texCoord;

void main(void)
{
   gl_Position = vec4( a_vertex.xy, 0.0, 1.0 );
   gl_Position = sign( gl_Position );
    
   texCoord = (vec2(gl_Position.x, gl_Position.y) + vec2(1.0)) / vec2(2.0);
}