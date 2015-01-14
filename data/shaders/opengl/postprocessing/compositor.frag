#define BlendMode 1

uniform sampler2D texture0;
uniform sampler2D texture1;
in vec2 v_texCoord;

out vec4 frag_color;

void main(void)
{
   frag_color = texture2D(texture0, v_texCoord);
   
   vec4 scene_sample = texture2D(texture0, v_texCoord);
   vec4 blur_sample = texture2D(texture1, v_texCoord);
   
#if (BlendMode == 0)
   // No bloom
   frag_color = scene_sample;
#elif (BlendMode == 1)
   // Weighted additive blending
   frag_color = clamp(scene_sample + (blur_sample * 0.35), 0.0, 1.0);
#elif (BlendMode == 2)    
   // Screen blending
   frag_color = clamp((scene_sample + blur_sample) - (scene_sample * blur_sample), 0.0, 1.0);
   frag_color.w = 1.0;
#elif (BlendMode == 3)
   // Softlight blending
   // Due to the nature of soft lighting, we need to bump the black region of the glowmap
   // to 0.5, otherwise the blended result will be dark (black soft lighting will darken
   // the image).
   blur_sample = (blur_sample * 0.5) + 0.5;      
   frag_color.xyz = 
      vec3((blur_sample.x <= 0.5) ? 
         (scene_sample.x - (1.0 - 2.0 * blur_sample.x) * scene_sample.x * (1.0 - scene_sample.x)) : 
         (((blur_sample.x > 0.5) && (scene_sample.x <= 0.25)) ? 
         (scene_sample.x + (2.0 * blur_sample.x - 1.0) * (4.0 * scene_sample.x * (4.0 * scene_sample.x + 1.0) * (scene_sample.x - 1.0) + 7.0 * scene_sample.x)) : 
         (scene_sample.x + (2.0 * blur_sample.x - 1.0) * (sqrt(scene_sample.x) - scene_sample.x))),
      (blur_sample.y <= 0.5) ? 
         (scene_sample.y - (1.0 - 2.0 * blur_sample.y) * scene_sample.y * (1.0 - scene_sample.y)) : 
         (((blur_sample.y > 0.5) && (scene_sample.y <= 0.25)) ? 
            (scene_sample.y + (2.0 * blur_sample.y - 1.0) * (4.0 * scene_sample.y * (4.0 * scene_sample.y + 1.0) * (scene_sample.y - 1.0) + 7.0 * scene_sample.y)) : 
            (scene_sample.y + (2.0 * blur_sample.y - 1.0) * (sqrt(scene_sample.y) - scene_sample.y))),
      (blur_sample.z <= 0.5) ? 
         (scene_sample.z - (1.0 - 2.0 * blur_sample.z) * scene_sample.z * (1.0 - scene_sample.z)) : 
         (((blur_sample.z > 0.5) && (scene_sample.z <= 0.25)) ? 
            (scene_sample.z + (2.0 * blur_sample.z - 1.0) * (4.0 * scene_sample.z * (4.0 * scene_sample.z + 1.0) * (scene_sample.z - 1.0) + 7.0 * scene_sample.z)) : 
            (scene_sample.z + (2.0 * blur_sample.z - 1.0) * (sqrt(scene_sample.z) - scene_sample.z))));
   frag_color.w = 1.0;
#endif
}