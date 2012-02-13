#include "Render.h"
#include "RenderTarget.h"
#include <stdexcept>
#include <sstream>
#include <iterator>
#include "RendererLegacy.h"
#include "RendererGL2.h"

static GLuint boundArrayBufferObject = 0;
static GLuint boundElementArrayBufferObject = 0;

namespace Render {

static bool initted = false;

Shader *simpleShader;
Shader *planetRingsShader[4];

SHADER_CLASS_BEGIN(BillboardShader)
	SHADER_UNIFORM_SAMPLER(some_texture)
SHADER_CLASS_END()

BillboardShader *billboardShader;

int State::m_numLights = 1;
float State::m_znear = 10.0f;
float State::m_zfar = 1e6f;
float State::m_invLogZfarPlus1;

void BindArrayBuffer(GLuint bo)
{
	if (boundArrayBufferObject != bo) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, bo);
		boundArrayBufferObject = bo;
	}
}

bool IsArrayBufferBound(GLuint bo)
{
	return boundArrayBufferObject == bo;
}

void BindElementArrayBuffer(GLuint bo)
{
	if (boundElementArrayBufferObject != bo) {
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bo);
		boundElementArrayBufferObject = bo;
	}
}

bool IsElementArrayBufferBound(GLuint bo)
{
	return boundElementArrayBufferObject == bo;
}

void UnbindAllBuffers()
{
	BindElementArrayBuffer(0);
	BindArrayBuffer(0);
}

Renderer* Init(int screen_width, int screen_height, bool wantShaders)
{
	assert(!initted);
	if (initted) return 0;

	Renderer *renderer = 0;

	PrintGLInfo();

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = wantShaders;

	if (shadersAvailable && wantShaders)
		renderer = new RendererGL2(screen_width, screen_height);
	else
		renderer = new RendererLegacy(screen_width, screen_height);

	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
	
	initted = true;

	if (shadersEnabled) {
		simpleShader = new Shader("simple");
		billboardShader = new BillboardShader("billboard");
		planetRingsShader[0] = new Shader("planetrings", "#define NUM_LIGHTS 1\n");
		planetRingsShader[1] = new Shader("planetrings", "#define NUM_LIGHTS 2\n");
		planetRingsShader[2] = new Shader("planetrings", "#define NUM_LIGHTS 3\n");
		planetRingsShader[3] = new Shader("planetrings", "#define NUM_LIGHTS 4\n");
	}

	return renderer;
}

void Uninit()
{
	delete simpleShader;
	delete billboardShader;
	delete planetRingsShader[0];
	delete planetRingsShader[1];
	delete planetRingsShader[2];
	delete planetRingsShader[3];
	FreeLibs();
}

void SwapBuffers()
{
	SDL_GL_SwapBuffers();
}

bool AreShadersEnabled()
{
	return shadersEnabled;
}

void ToggleShaders()
{
	if (shadersAvailable) {
		shadersEnabled = (shadersEnabled ? false : true);
	}
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
}

/**
 * So if we are using the z-hack VPROG_POINTSPRITE then this still works.
 * Desired texture should already be bound on calling PutPointSprites()
 */
void PutPointSprites(int num, vector3f *v, float size, const float modulationCol[4], int stride)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

//	float quadratic[] =  { 0.0f, 0.0f, 0.00001f };
//	glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
//	glPointParameterf(GL_POINT_SIZE_MIN, 1.0 );
//	glPointParameterf(GL_POINT_SIZE_MAX, 10000.0 );
		
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glColor4fv(modulationCol);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	

	// XXX point sprite thing needs some work. remember to enable point
	// sprite shader in LmrModel.cpp
	if (AreShadersEnabled()) {
		// this is a bit dumb since it doesn't care how many lights
		// the scene has, and this is a constant...
		billboardShader->Use();
		billboardShader->set_some_texture(0);
	}

//	/*if (Shader::IsVtxProgActive())*/ glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	if (0) {//GLEW_ARB_point_sprite) {
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable(GL_POINT_SPRITE_ARB);
		
		glPointSize(size);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, v);
		glDrawArrays(GL_POINTS, 0, num);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPointSize(1);

		glDisable(GL_POINT_SPRITE_ARB);
		glDisable(GL_TEXTURE_2D);
	
	} else {
		// quad billboards
		matrix4x4f rot;
		glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
		rot.ClearToRotOnly();
		rot = rot.InverseOf();

		const float sz = 0.5f*size;
		const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
		const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
		const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
		const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

		glBegin(GL_QUADS);
		for (int i=0; i<num; i++) {
			vector3f pos(*v);
			vector3f vert;

			vert = pos+rotv4;
			glTexCoord2f(0.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv3;
			glTexCoord2f(0.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv2;
			glTexCoord2f(1.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv1;
			glTexCoord2f(1.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			v = reinterpret_cast<vector3f*>(reinterpret_cast<char*>(v)+stride);
		}
		glEnd();
	}
//	/*if (Shader::IsVtxProgActive())*/ glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

//	quadratic[0] = 1; quadratic[1] = 0;
//	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	billboardShader->Unuse();
}

void PrintGLInfo() {
	std::string fname = GetPiUserDir() + "opengl.txt";
	FILE *f = fopen(fname.c_str(), "w");
	if (!f) return;

	std::ostringstream ss;
	ss << "OpenGL version " << glGetString(GL_VERSION);
	ss << ", running on " << glGetString(GL_VENDOR);
	ss << " " << glGetString(GL_RENDERER) << std::endl;

	ss << "Available extensions:" << std::endl;
	GLint numext = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
	if (glewIsSupported("GL_VERSION_3_0")) {
		for (int i = 0; i < numext; ++i) {
			ss << "  " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
		}
	}
	else {
		ss << "  ";
		std::istringstream ext(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
		std::copy(
			std::istream_iterator<std::string>(ext),
			std::istream_iterator<std::string>(),
			std::ostream_iterator<std::string>(ss, "\n  "));
	}

	fprintf(f, "%s", ss.str().c_str());
	fclose(f);
	printf("OpenGL system information saved to %s\n", fname.c_str());
}

} /* namespace Render */
