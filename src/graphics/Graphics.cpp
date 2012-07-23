#include "Graphics.h"
#include "Shader.h"
#include "RendererLegacy.h"
#include "RendererGL2.h"
#include "FileSystem.h"
#include "OS.h"

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

Renderer* Init(const Settings &vs)
{
	assert(!initted);
	if (initted) return 0;

	int width = vs.width;
	int height = vs.height;

	// no mode set, find an ok one
	if ((width <= 0) || (height <= 0)) {
		SDL_Rect **modes = SDL_ListModes(NULL, SDL_HWSURFACE | SDL_FULLSCREEN);

		if (modes == 0) {
			fprintf(stderr, "It seems no video modes are available...");
		}
		if (modes == reinterpret_cast<SDL_Rect **>(-1)) {
			// hm. all modes available. odd. try 800x600
			width = 800; height = 600;
		} else {
			width = modes[0]->w;
			height = modes[0]->h;
		}
	}

	const SDL_VideoInfo *info = SDL_GetVideoInfo();

	//printf("SDL_GetVideoInfo says %d bpp\n", info->vfmt->BitsPerPixel);

	switch (info->vfmt->BitsPerPixel) {
		case 16:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			break;
		case 24:
		case 32:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			break;
		default:
			fprintf(stderr, "Invalid pixel depth: %d bpp\n", info->vfmt->BitsPerPixel);
	}
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, vs.requestedSamples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, vs.requestedSamples);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vs.vsync);

	Uint32 flags = SDL_OPENGL;
	if (vs.fullscreen) flags |= SDL_FULLSCREEN;

	// attempt sequence is:
	// 1- requested mode
	SDL_Surface *scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags);

	// 2- requested mode with no anti-aliasing (skipped if no AA was requested anyway)
	if (!scrSurface && vs.requestedSamples) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying without multisampling.\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

		scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags);
	}

	// 3- requested mode with 16 bit depth buffer
	if (!scrSurface) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, vs.requestedSamples ? 1 : 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, vs.requestedSamples);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

		scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags);
	}

	// 4- requested mode with 16-bit depth buffer and no anti-aliasing
	//    (skipped if no AA was requested anyway)
	if (!scrSurface && vs.requestedSamples) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer and no multisampling\n", SDL_GetError());
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

		scrSurface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, flags);
	}

	// 5- abort!
	if (!scrSurface) {
		OS::Error("Failed to set video mode: %s", SDL_GetError());
	}

	// this valuable is not reliable if antialiasing settings are overridden by
	// nvidia/ati/whatever settings
	int actualSamples = 0;
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &actualSamples);
	if (vs.requestedSamples != actualSamples)
		fprintf(stderr, "Requested AA mode: %dx, actual: %dx\n", vs.requestedSamples, actualSamples);

	glewInit();

	if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
		OS::Error("OpenGL extension ARB_vertex_buffer_object not supported. Pioneer can not run on your graphics card.");

	Renderer *renderer = 0;

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = vs.shaders && shadersAvailable;

	if (shadersEnabled)
		renderer = new RendererGL2(vs.width, vs.height);
	else
		renderer = new RendererLegacy(vs.width, vs.height);

	printf("Initialized %s\n", renderer->GetName());

	initted = true;

	//XXX to be moved
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
