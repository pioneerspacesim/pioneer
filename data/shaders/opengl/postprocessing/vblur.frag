uniform sampler2D texture0;
in vec2 v_texCoord;
in vec2 v_blurTexCoords[14];

out vec4 frag_color;

void main(void)
{
   frag_color = vec4(0);
   frag_color += texture2D(texture0, v_blurTexCoords[ 0])*0.0044299121055113265;
   frag_color += texture2D(texture0, v_blurTexCoords[ 1])*0.00895781211794;
   frag_color += texture2D(texture0, v_blurTexCoords[ 2])*0.0215963866053;
   frag_color += texture2D(texture0, v_blurTexCoords[ 3])*0.0443683338718;
   frag_color += texture2D(texture0, v_blurTexCoords[ 4])*0.0776744219933;
   frag_color += texture2D(texture0, v_blurTexCoords[ 5])*0.115876621105;
   frag_color += texture2D(texture0, v_blurTexCoords[ 6])*0.147308056121;
   frag_color += texture2D(texture0, v_texCoord         )*0.159576912161;
   frag_color += texture2D(texture0, v_blurTexCoords[ 7])*0.147308056121;
   frag_color += texture2D(texture0, v_blurTexCoords[ 8])*0.115876621105;
   frag_color += texture2D(texture0, v_blurTexCoords[ 9])*0.0776744219933;
   frag_color += texture2D(texture0, v_blurTexCoords[10])*0.0443683338718;
   frag_color += texture2D(texture0, v_blurTexCoords[11])*0.0215963866053;
   frag_color += texture2D(texture0, v_blurTexCoords[12])*0.00895781211794;
   frag_color += texture2D(texture0, v_blurTexCoords[13])*0.0044299121055113265;
}