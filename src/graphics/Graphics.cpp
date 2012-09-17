#include "Graphics.h"
#include "FileSystem.h"
#include "Material.h"
#include "RendererGL2.h"
#include "RendererLegacy.h"
#include "OS.h"

static GLuint boundArrayBufferObject = 0;
static GLuint boundElementArrayBufferObject = 0;

namespace Graphics {

static bool initted = false;
bool shadersAvailable = false;
bool shadersEnabled = false;
Material *vtxColorMaterial;

float State::invLogZfarPlus1;
std::vector<Light> State::m_lights;

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

	const bool useDXTnTextures = vs.useDXTn && glewIsSupported("GL_ARB_texture_compression");

	Renderer *renderer = 0;

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = vs.shaders && shadersAvailable;

	if (shadersEnabled)
		renderer = new RendererGL2(vs.width, vs.height, useDXTnTextures);
	else
		renderer = new RendererLegacy(vs.width, vs.height, useDXTnTextures);

	printf("Initialized %s\n", renderer->GetName());

	initted = true;

	MaterialDescriptor desc;
	desc.vertexColors = true;
	vtxColorMaterial = renderer->CreateMaterial(desc);
	
	return renderer;
}

void Uninit()
{
	delete vtxColorMaterial;
}

void SwapBuffers()
{
	SDL_GL_SwapBuffers();
}

bool AreShadersEnabled()
{
	return shadersEnabled;
}

void Graphics::State::SetLights(int n, const Light *lights)
{
	m_lights.clear();
	m_lights.reserve(n);
	for (int i = 0;i < n;i++)
		m_lights.push_back(lights[i]);
}

}
