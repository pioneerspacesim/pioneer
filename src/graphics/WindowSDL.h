// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GRAPHICS_WINDOW_SDL_H
#define GRAPHICS_WINDOW_SDL_H

#include "Graphics.h"

namespace Graphics {

class WindowSDL {
public:
	WindowSDL(Graphics::Settings &settings, const std::string &name);
	~WindowSDL();

	int GetWidth() const;
	int GetHeight() const;

	void SetGrab(bool grabbed);

	void SwapBuffers();

    SDL_Window *GetSDLWindow() const { return m_window; }
private:
	bool CreateWindowAndContext(const char *name, const Graphics::Settings &settings, int samples, int depth_bits);

	SDL_Window *m_window;
	SDL_GLContext m_glContext;
};

}

#endif
