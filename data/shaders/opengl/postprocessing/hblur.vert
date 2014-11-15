out vec2 v_texCoord;
out vec2 v_blurTexCoords[14];

void main(void)
{
   // Clean up inaccuracies
   vec2 Pos = sign(a_vertex.xy);

   gl_Position = vec4(Pos.xy, 0, 1);
   // Image-space
   v_texCoord.x = 0.5 * (1.0 + Pos.x);
   v_texCoord.y = 0.5 * (1.0 + Pos.y);   
   
   v_blurTexCoords[ 0] = v_texCoord + vec2(-0.028, 0.0);
   v_blurTexCoords[ 1] = v_texCoord + vec2(-0.024, 0.0);
   v_blurTexCoords[ 2] = v_texCoord + vec2(-0.020, 0.0);
   v_blurTexCoords[ 3] = v_texCoord + vec2(-0.016, 0.0);
   v_blurTexCoords[ 4] = v_texCoord + vec2(-0.012, 0.0);
   v_blurTexCoords[ 5] = v_texCoord + vec2(-0.008, 0.0);
   v_blurTexCoords[ 6] = v_texCoord + vec2(-0.004, 0.0);
   v_blurTexCoords[ 7] = v_texCoord + vec2( 0.004, 0.0);
   v_blurTexCoords[ 8] = v_texCoord + vec2( 0.008, 0.0);
   v_blurTexCoords[ 9] = v_texCoord + vec2( 0.012, 0.0);
   v_blurTexCoords[10] = v_texCoord + vec2( 0.016, 0.0);
   v_blurTexCoords[11] = v_texCoord + vec2( 0.020, 0.0);
   v_blurTexCoords[12] = v_texCoord + vec2( 0.024, 0.0);
   v_blurTexCoords[13] = v_texCoord + vec2( 0.028, 0.0);
}