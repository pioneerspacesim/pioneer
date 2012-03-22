#include "Graphics.h"
#include "Shader.h"
#include "RendererLegacy.h"
#include "RendererGL2.h"
#include "FileSystem.h"

static GLuint boundArrayBufferObject = 0;
static GLuint boundElementArrayBufferObject = 0;

namespace Graphics {

static bool initted = false;

Shader *simpleShader;
Shader *planetRingsShader[4];

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

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = wantShaders && shadersAvailable;

	if (shadersEnabled)
		renderer = new RendererGL2(screen_width, screen_height);
	else
		renderer = new RendererLegacy(screen_width, screen_height);

	printf("Initialized %s\n", renderer->GetName());
	
	initted = true;

	if (shadersEnabled) {
		simpleShader = new Shader("simple");
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

}
