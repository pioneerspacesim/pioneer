#include "Render.h"
#include "Shader.h"

namespace Render {

/*
 * So if we are using the z-hack VPROG_POINTSPRITE then this still works.
 */
void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	float quadratic[] =  { 0.0f, 0.005f, 0.0f };
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0 );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, 100.0 );

	if (GLEW_ARB_point_sprite) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
		glEnable(GL_POINT_SPRITE_ARB );
	}
	if (Shader::IsVtxProgActive()) glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
		
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPointSize(size);
	glColor4fv(modulationCol);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &v[0].x);
	glDrawArrays(GL_POINTS, 0, num);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPointSize(1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	if (GLEW_ARB_point_sprite) {
		glDisable(GL_POINT_SPRITE_ARB);
		glDisable(GL_TEXTURE_2D);
	}
	if (Shader::IsVtxProgActive()) glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

	quadratic[0] = 1; quadratic[1] = 0;
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

}; /* namespace Render */
