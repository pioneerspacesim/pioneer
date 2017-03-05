// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "WindowSDL.h"

#include "SDL.h"
#include "SDLWrappers.h"
#include "OS.h"

namespace Graphics {

bool WindowSDL::CreateWindowAndContext(const char *name, const Graphics::Settings &vs, int samples, int depth_bits) {
	Uint32 winFlags = SDL_WINDOW_OPENGL | (vs.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
	if (!vs.hidden && vs.fullscreen) winFlags |= SDL_WINDOW_FULLSCREEN;

	switch(vs.rendererType) {
	case Graphics::RendererType::RENDERER_OPENGL_21:
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			// cannot initialise 3.x content on OSX with anything but CORE profile
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
			// OSX also forces us to use this for 3.2 onwards
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		}
		break;
	case Graphics::RendererType::RENDERER_OPENGL_3x:
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			// cannot initialise 3.x content on OSX with anything but CORE profile
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			// OSX also forces us to use this for 3.2 onwards
			if (vs.gl3ForwardCompatible) SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		}
		break;
	default:
		assert(false);
		Error("Passed DUMMY or unhandled type of renderer into WindowSDL::CreateWindowAndContext");
		break;
	}

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, samples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);

	// need full 32-bit color
	// (need an alpha channel because of the way progress bars are drawn)
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	m_window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vs.width, vs.height, winFlags);
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

WindowSDL::WindowSDL(Graphics::Settings &vs, const std::string &name) {

	// XXX horrible hack. if we don't want a renderer, we might be in an
	// environment that doesn't actually have graphics available. since we're
	// not going to draw anything anyway, there's not much point initialising a
	// window (which will fail in aforementioned headless environment)
	//
	// the "right" way would be to have a dummy window class as well, and move
	// a lot of this initialisation into the GL renderer. this is much easier
	// right now though
	if (vs.rendererType == Graphics::RENDERER_DUMMY)
		return;

	bool ok;

	// attempt sequence is:
	// 1- requested mode
	ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 24);

	// 2- requested mode with no anti-aliasing (skipped if no AA was requested anyway)
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		Output("Failed to set video mode. (%s). Re-trying without multisampling.\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, 0, 24);
	}

	// 3- requested mode with 16 bit depth buffer
	if (!ok) {
		Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 16);
	}

	// 4- requested mode with 16-bit depth buffer and no anti-aliasing
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer and no multisampling\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, 0, 16);
	}

	if (!ok && vs.rendererType != Graphics::RENDERER_OPENGL_21)
	{
		Output("Retrying all previous modes using an OpenGL 2.1 context\n", SDL_GetError());
		vs.rendererType = Graphics::RENDERER_OPENGL_21;

		// attempt sequence is:
		// 1- requested mode
		ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 24);

		// 2- requested mode with no anti-aliasing (skipped if no AA was requested anyway)
		//    (skipped if no AA was requested anyway)
		if (!ok && vs.requestedSamples) {
			Output("Failed to set video mode. (%s). Re-trying without multisampling.\n", SDL_GetError());
			ok = CreateWindowAndContext(name.c_str(), vs, 0, 24);
		}

		// 3- requested mode with 16 bit depth buffer
		if (!ok) {
			Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer\n", SDL_GetError());
			ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 16);
		}

		// 4- requested mode with 16-bit depth buffer and no anti-aliasing
		//    (skipped if no AA was requested anyway)
		if (!ok && vs.requestedSamples) {
			Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer and no multisampling\n", SDL_GetError());
			ok = CreateWindowAndContext(name.c_str(), vs, 0, 16);
		}
	}

	// 5- abort!
	if (!ok) {
		Error("Failed to set video mode: %s", SDL_GetError());
	}

	SDLSurfacePtr surface = LoadSurfaceFromFile(vs.iconFile);
	if (surface)
		SDL_SetWindowIcon(m_window, surface.Get());

	SDL_SetWindowTitle(m_window, vs.title);
	SDL_ShowCursor(0);

	SDL_GL_SetSwapInterval((vs.vsync!=0) ? 1 : 0);
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
