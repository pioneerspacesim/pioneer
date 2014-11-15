out vec2 v_texCoord;

void main(void)
{
   // Clean up inaccuracies
   vec2 Pos = sign(a_vertex.xy);

   gl_Position = vec4(Pos.xy, 0, 1);
   // Image-space
   v_texCoord.x = 0.5 * (1.0 + Pos.x);
   v_texCoord.y = 0.5 * (1.0 + Pos.y);
   
}