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

	float quadratic[] =  { 0.0f, 0.0f, 1.0f };
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0 );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, 100.0 );
		
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glColor4fv(modulationCol);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	

	// XXX does this shader work ok with quad billboards??
	if (Shader::IsVtxProgActive()) glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	if (GLEW_ARB_point_sprite) {
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable(GL_POINT_SPRITE_ARB);
		
		glPointSize(200.0f*size);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &v[0].x);
		glDrawArrays(GL_POINTS, 0, num);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPointSize(1);

		glDisable(GL_POINT_SPRITE_ARB);
		glDisable(GL_TEXTURE_2D);
	
	} else {
		// quad billboards
		const float sz = 0.5f*size;
		vector3f v1(sz, sz, 0.0f);
		vector3f v2(sz, -sz, 0.0f);
		vector3f v3(-sz, -sz, 0.0f);
		vector3f v4(-sz, sz, 0.0f);
		
		glBegin(GL_QUADS);
		for (int i=0; i<num; i++) {
			vector3f pos(&v[i].x);
			vector3f zaxis = pos.Normalized();
			vector3f xaxis = vector3f::Cross(vector3d(0,1,0), zaxis).Normalized();
			vector3f yaxis = vector3f::Cross(zaxis,xaxis);
			matrix4x4f rot = matrix4x4f::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();

			glTexCoord2f(0.0f,0.0f);
			glVertex3fv(&(pos+rot*v1).x);
			glTexCoord2f(0.0f,1.0f);
			glVertex3fv(&(pos+rot*v2).x);
			glTexCoord2f(1.0f,1.0f);
			glVertex3fv(&(pos+rot*v3).x);
			glTexCoord2f(1.0f,0.0f);
			glVertex3fv(&(pos+rot*v4).x);
		}
		glEnd();
	}
	if (Shader::IsVtxProgActive()) glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

	quadratic[0] = 1; quadratic[1] = 0;
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

}; /* namespace Render */
