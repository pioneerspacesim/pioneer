#include "WindowSDL.h"

#include "SDL.h"
#include "SDLWrappers.h"
#include "OS.h"

namespace Graphics {

bool WindowSDL::CreateWindowAndContext(const char *name, int w, int h, bool fullscreen, int samples, int depth_bits) {
	Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if (fullscreen) winFlags |= SDL_WINDOW_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, samples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);

	m_window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, winFlags);
	if (!m_window)
		return false;

	m_glContext = SDL_GL_CreateContext(m_window);
	if (!m_glContext) {
		SDL_DestroyWindow(m_window);
		m_window = 0;
		return false;
	}

	return true;
}

WindowSDL::WindowSDL(const Graphics::Settings &vs, const std::string &name)
{
	bool ok;

	// attempt sequence is:
	// 1- requested mode
	ok = CreateWindowAndContext(name.c_str(), vs.width, vs.height, vs.fullscreen, vs.requestedSamples, 24);

	// 2- requested mode with no anti-aliasing (skipped if no AA was requested anyway)
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying without multisampling.\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs.width, vs.height, vs.fullscreen, 0, 24);
	}

	// 3- requested mode with 16 bit depth buffer
	if (!ok) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs.width, vs.height, vs.fullscreen, vs.requestedSamples, 16);
	}

	// 4- requested mode with 16-bit depth buffer and no anti-aliasing
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer and no multisampling\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs.width, vs.height, vs.fullscreen, 0, 16);
	}

	// 5- abort!
	if (!ok) {
		OS::Error("Failed to set video mode: %s", SDL_GetError());
	}

	int bpp;
	Uint32 rmask, gmask, bmask, amask;
	SDL_PixelFormatEnumToMasks(SDL_GetWindowPixelFormat(m_window), &bpp, &rmask, &gmask, &bmask, &amask);

	switch (bpp) {
		case 16:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			break;
		case 24:
		case 32:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			break;
		default:
			fprintf(stderr, "Invalid pixel depth: %d bpp\n", bpp);

		// this valuable is not reliable if antialiasing vs are overridden by
		// nvidia/ati/whatever vs
		int actualSamples = 0;
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &actualSamples);
		if (vs.requestedSamples != actualSamples)
			fprintf(stderr, "Requested AA mode: %dx, actual: %dx\n", vs.requestedSamples, actualSamples);
	}

	SDLSurfacePtr surface = LoadSurfaceFromFile(vs.iconFile);
	if (surface)
		SDL_SetWindowIcon(m_window, surface.Get());

	SDL_SetWindowTitle(m_window, vs.title);
}

WindowSDL::~WindowSDL()
{
	SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(m_window);
}

int WindowSDL::GetWidth() const
{
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	return w;
}

int WindowSDL::GetHeight() const
{
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	return h;
}

void WindowSDL::SetGrab(bool grabbed)
{
	SDL_SetWindowGrab(m_window, SDL_bool(grabbed));
	SDL_SetRelativeMouseMode(SDL_bool(grabbed));
}

void WindowSDL::SwapBuffers()
{
	SDL_GL_SwapWindow(m_window);
}

}
